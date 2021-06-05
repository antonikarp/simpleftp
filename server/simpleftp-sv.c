#include "simpleftp-sv.h"

volatile sig_atomic_t do_work = 1;

void sigint_handler(int sig) {
	do_work = 0;
}

void usage(char *name) {
	fprintf(stderr, "USAGE: %s port workdir\n", name);
	exit(EXIT_FAILURE);
}

void run_server(int server_fd, struct thread_arg *arg) {
	char hello[] = "Hello";
	char reject[] = "Rejected. Too many clients.";
	
	// All invalid descriptors are stored as zeros.
	int client_fd[MAXCL];
	memset(client_fd, 0, MAXCL * sizeof(int));
	
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
				
				
				if (pthread_mutex_lock(arg->new_request_mutex) != 0) {
					ERR("pthread_mutex_lock");
				}
				*(arg->cur_client_fd) = client_fd[i];
				*(arg->new_request_condition) = 1;
				if (pthread_mutex_unlock(arg->new_request_mutex) != 0) {
					ERR("pthread_cond_unlock");
				}
				if (pthread_cond_signal(arg->new_request_cond) != 0) {
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





int main(int argc, char **argv) {
	if (set_handler(sigint_handler, SIGINT)) {
		ERR("set_handler");
	}
	
	if (argc != 3) {
		usage(argv[0]);
	}
	int16_t port = atoi(argv[1]);
	int server_fd = bind_tcp_socket(port);
	
	struct thread_arg arg;
	
	int cur_client_fd;
	arg.cur_client_fd = &cur_client_fd;
	
	pthread_cond_t new_request_cond = PTHREAD_COND_INITIALIZER;
	arg.new_request_cond = &new_request_cond;
	
	pthread_mutex_t new_request_mutex = PTHREAD_MUTEX_INITIALIZER;
	arg.new_request_mutex = &new_request_mutex;
	
	int new_request_condition = 0;
	arg.new_request_condition = &new_request_condition;
	
	struct thread_arg *args = (struct thread_arg*) malloc(MAXCL * sizeof(struct thread_arg));
	pthread_t *threads = (pthread_t *) malloc(MAXCL * sizeof(pthread_t)); 

	init_threads(threads, &arg, args);
	run_server(server_fd, &arg);
	
	free(args);
	free(threads);
	
	return EXIT_SUCCESS;
}
