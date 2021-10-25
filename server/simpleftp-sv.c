#include "simpleftp-sv.h"

volatile sig_atomic_t do_work_server = 1;

void sigint_handler(int sig) {
	do_work_server = 0;
}

void usage(char *name) {
	fprintf(stderr, "USAGE: %s port workdir\n", name);
	exit(EXIT_FAILURE);
}

void run_server(int server_fd, struct global_store *store) {
	char hello[] = "Hello";
	char reject[] = "Rejected. Too many clients.";
	
	int new_client_fd;
	int i;
	char buf[BUFSIZE];
	memset(buf, 0, BUFSIZE);
	int fdmax;
	
	fd_set rfds;
	FD_ZERO(&(store->base_rfds));
	FD_SET(server_fd, &(store->base_rfds));
	
	sigset_t mask, oldmask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigprocmask(SIG_BLOCK, &mask, &oldmask);
	
	while(do_work_server) {
		rfds = store->base_rfds;
		fdmax = calculate_max(server_fd, store->client_fd);
		if (pselect(fdmax + 1, &rfds, NULL, NULL, NULL, &oldmask) > 0) {
			if (FD_ISSET(server_fd, &rfds)) {
				// Handle incoming connections.
				new_client_fd = add_new_client(server_fd);
				i = 0;
				while (i < MAXCL && store->client_fd[i] != 0) {
					i++;
				}
				if (i < MAXCL) {
					// New client can be added.
					store->client_fd[i] = new_client_fd;
					FD_SET(store->client_fd[i], &(store->base_rfds));
					persist_write(store->client_fd[i], hello, strlen(hello) + 1);
				} else {
					// All connections are taken.
					persist_write(new_client_fd, reject, strlen(reject) + 1);
					if (shutdown(new_client_fd, SHUT_RDWR) < 0) {
						ERR("shutdown");
					}
					if (close(new_client_fd) < 0) {
						ERR("close");
					}
				}	
			} else {
				// Handle messages from the clients.
				i = 0;
				while (i < MAXCL && store->client_fd[i] != 0 && !FD_ISSET(store->client_fd[i], &rfds)) {
					// Find which client sent a message.
					i++;
				}
				if (pthread_mutex_lock(&(store->new_request_mutex)) != 0) {
					ERR("pthread_mutex_lock");
				}
				store->cur_client_i = i;
				store->new_request_condition = 1;
				if (pthread_mutex_unlock(&(store->new_request_mutex)) != 0) {
					ERR("pthread_cond_unlock");
				}
				if (pthread_cond_signal(&(store->new_request_cond)) != 0) {
					ERR("pthread_cond_signal");
				}
				/* Wait for a thread to finish reading to prevent executing
				 * this section of the code more than once */
				if (sem_wait(&(store->sem)) != 0) {
					ERR("sem_wait");
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
	close_all_connections(server_fd, store->client_fd);

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
	int *client_fd = (int *) malloc(MAXCL * sizeof(int));
	// All invalid descriptors are stored as zeros.
	memset(client_fd, 0, MAXCL * sizeof(int));
	store->client_fd = client_fd;
	if (sem_init(&(store->sem), 0, 0) != 0) {
		ERR("sem_init");
	}
	
}

void deallocate_global_store(struct global_store *store) {
	free(store->args);
	free(store->threads);
	free(store->client_fd);
}

/* init_threads
 * Intialize idle, detached threads.
 */
void init_threads(struct global_store *store) {
	for (int i = 0; i < MAXCL; i++) {
		pthread_attr_t thread_attr;
		if (pthread_attr_init(&thread_attr)) {
			ERR("pthread_attr_init");
		}
		if (pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED)) {
			ERR("pthread_attr_setdetachstate");
		}
		store->args[i].id = i;
		store->args[i].ptr_new_request_condition = &(store->new_request_condition);
		store->args[i].ptr_new_request_cond = &(store->new_request_cond);
		store->args[i].ptr_new_request_mutex = &(store->new_request_mutex);
		store->args[i].ptr_cur_client_i = &(store->cur_client_i);
		store->args[i].ptr_client_fd = store->client_fd;
		store->args[i].ptr_base_rfds = &(store->base_rfds);
		store->args[i].ptr_sem = &(store->sem);
		if (pthread_create(&(store->threads[i]), &thread_attr, thread_worker, (void *) &(store->args[i])) != 0) {
			ERR("pthread_create");
		}
		pthread_attr_destroy(&thread_attr);
	}	
}

int main(int argc, char **argv) {
	if (set_handler(sigint_handler, SIGINT)) {
		ERR("set_handler");
	}
	
	if (argc != 3) {
		usage(argv[0]);
	}
	if (chdir(argv[2]) != 0) {
		ERR("chdir");
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
