#include "simpleftp-sv.h"

volatile sig_atomic_t do_work = 1;

void sigint_handler(int sig) {
	do_work = 0;
}

void usage(char *name) {
	fprintf(stderr, "USAGE: %s port workdir\n", name);
	exit(EXIT_FAILURE);
}

void run_server(int server_fd, struct global_store *store) {
	char hello[] = "Hello";
	char reject[] = "Rejected. Too many clients.";
	
	
	int client_fd[MAXCL];
	memset(client_fd, 0, MAXCL*sizeof(int));
	// All invalid descriptors are stored as zeros.
	
	int new_client_fd;
	int i;
	char buf[BUFSIZE];
	memset(buf, 0, BUFSIZE);
	int fdmax;
	
	fd_set base_rfds, rfds;
	FD_ZERO(&base_rfds);
	FD_SET(server_fd, &base_rfds);
	
	sigset_t mask, oldmask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigprocmask(SIG_BLOCK, &mask, &oldmask);
	
	while(do_work) {
		rfds = base_rfds;
		fdmax = calculate_max(server_fd, client_fd);
		if (pselect(fdmax + 1, &rfds, NULL, NULL, NULL, &oldmask) > 0) {
			if (FD_ISSET(server_fd, &rfds)) {
				// Handle incoming connections.
				new_client_fd = add_new_client(server_fd);
				i = 0;
				while (i < MAXCL && client_fd[i] != 0) {
					i++;
				}

				if (i < MAXCL) {
					// New client can be added.
					client_fd[i] = new_client_fd;
					FD_SET(client_fd[i], &base_rfds);
					persist_write(client_fd[i], hello, strlen(hello) + 1);
				} else {
					// All connections are taken.
					persist_write(new_client_fd, reject, strlen(reject) + 1);
				}	
			} else {
				// Handle messages from the clients.
				i = 0;
				while (i < MAXCL && client_fd[i] != 0 && !FD_ISSET(client_fd[i], &rfds)) {
					// Find which client sent a message.
					i++;
				}
				
				
				if (pthread_mutex_lock(&(store->new_request_mutex)) != 0) {
					ERR("pthread_mutex_lock");
				}
				store->cur_client_fd = client_fd[i];
				store->new_request_condition = 1;
				if (pthread_mutex_unlock(&(store->new_request_mutex)) != 0) {
					ERR("pthread_cond_unlock");
				}
				if (pthread_cond_signal(&(store->new_request_cond)) != 0) {
					ERR("pthread_cond_signal");
				}
			}
		} else {
			if (errno == EINTR) {
				continue;
			} else {
				ERR("pselect");
			}
		}	
	}
	close_all_connections(server_fd, client_fd);

}

void initialize_global_store (struct global_store *store) {
	memset(store, 0, sizeof(struct global_store));
	pthread_cond_init(&(store->new_request_cond), NULL);
	pthread_mutex_init(&(store->new_request_mutex), NULL);
	// store->new_request_condition is initialized to 0
	struct thread_arg *args = (struct thread_arg*) malloc(MAXCL * sizeof(struct thread_arg));
	store->args = args;
	pthread_t *threads = (pthread_t *) malloc(MAXCL * sizeof(pthread_t));
	store->threads = threads;
	
	
}

void deallocate_global_store(struct global_store *store) {
	free(store->args);
	free(store->threads);
}


int main(int argc, char **argv) {
	if (set_handler(sigint_handler, SIGINT)) {
		ERR("set_handler");
	}
	
	if (argc != 3) {
		usage(argv[0]);
	}
	int16_t port = atoi(argv[1]);
	int server_fd = bind_tcp_socket(port);
	
	struct global_store store;
	
	initialize_global_store(&store);

	init_threads(&store);
	run_server(server_fd, &store);
	
	deallocate_global_store(&store);
	
	return EXIT_SUCCESS;
}
