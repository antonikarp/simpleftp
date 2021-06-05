#include "simpleftp-sv.h"

/* make_socket
 * Create a socket given domain and type
 * returns: socket descriptor
 */

int make_socket(int domain, int type) {
	int sock;
	sock = socket(domain, type, 0);
	if (sock < 0) {
		ERR("socket");
	}
	return sock;
}

/* bind_tcp_socket
 * Bind TCP socket using given port on localhost
 * returns: socket descriptor
 */

int bind_tcp_socket(uint16_t port) {
	struct sockaddr_in addr;
	int socketfd;
	int t = 1;
	socketfd = make_socket(PF_INET, SOCK_STREAM);
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &t, sizeof(t))) {
		ERR("setsockopt");
	}
	if (bind(socketfd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		ERR("bind");
	}
	if (listen(socketfd, BACKLOG) < 0) {
		ERR("listen");
	}
	return socketfd;
}
/* add_new_client
 * Handle incoming connection to the bound socket (sfd)
 * returns: new file descriptor
 */

int add_new_client(int sfd) {
	int nfd;
	if ((nfd = TEMP_FAILURE_RETRY(accept(sfd, NULL, NULL))) < 0) {
		if (EAGAIN == errno || EWOULDBLOCK == errno) {
			return -1;
		}
		ERR("accept");
	}
	return nfd;
}

/* calculate_max
 * Calculate maximum nonzero file descriptor, for pselect call
 * returns: max
 */
int calculate_max(int server_fd, int *client_fd) {
	int max = server_fd;
	for (int i = 0; i < MAXCL; i++) {
		if (client_fd[i] != 0) {
			max = client_fd[i] > max ? client_fd[i] : max;
		}
	}
	return max;
	
}

/* set_handler
 * Set a function as a handler for signal
 * void (*f)(int) - pointer to function
 * int sigNo - symbolic constant of a signal
 * return: status
 */
int set_handler( void (*f)(int), int sigNo) {
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_handler = f;
	if (-1 == sigaction(sigNo, &act, NULL)) {
		return -1;
	}
	return 0;
}

/* close_all_connections
 * After receiving SIGINT, close all connections
 */
void close_all_connections (int server_fd, int *client_fd) {
	for (int i = 0; i < MAXCL; i++) {
		if (client_fd[i] != 0) {
			if (close(client_fd[i]) < 0) {
				ERR("close");
			}
		}
	}
	if (close(server_fd) < 0) {
		ERR("close");
	}
}

/* persist_read
 * Read up to |count| bytes from |fd| into |buf|.
 * Persist if interrupted by a signal.
 */
ssize_t persist_read(int fd, char *buf, size_t count) {
	ssize_t status;
	if ( (status = TEMP_FAILURE_RETRY(read(fd, buf, count))) < 0) {
		ERR("read");
	}
	return status;
}


/* persist_write
 * Write |count| bytes from |buf| to |fd|.
 * Persist if interrupted by a signal.
 */
ssize_t persist_write(int fd, char *buf, size_t count) {
	int c;
	size_t len = 0;
	do {
		c = TEMP_FAILURE_RETRY(write(fd, buf, count));
		if(c < 0) {
			ERR("write");
		}
		buf += c;
		len += c;
		count -= c;
	}
	while (count > 0);
	return len;
}


