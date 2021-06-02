#include "simpleftp-sv.h"

volatile sig_atomic_t do_work = 1;

void sigint_handler(int sig) {
	do_work = 0;
}

void usage(char *name) {
	fprintf(stderr, "USAGE: %s port workdir\n", name);
	exit(EXIT_FAILURE);
}



void runServer(int server_fd) {
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
					write(client_fd[i], hello, strlen(hello) + 1);
				} else {
					// All connections are taken.
					write(new_client_fd, reject, strlen(reject) + 1);
				}	
			} else {
				// Handle messages from the clients.
				i = 0;
				while (i < MAXCL && client_fd[i] != 0 && !FD_ISSET(client_fd[i], &rfds)) {
					// Find which client sent a message.
					i++;
				}
				int result_read = read(client_fd[i], buf, BUFSIZE);
				if (result_read < 0) {
					ERR("read");
				} else if (result_read == 0) {
					// Handle disconnection.
					if (close(client_fd[i]) < 0) {
						ERR("close");
					}
					FD_CLR(client_fd[i], &base_rfds);
					client_fd[i] = 0;
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
	if (argc != 3) {
		usage(argv[0]);
	}
	int16_t port = atoi(argv[1]);
	int server_fd = bind_tcp_socket(port);
	runServer(server_fd);
	return EXIT_SUCCESS;
}
