#include "simpleftp-cl.h"

/* make_socket
 * Create a socket
 * returns: socket descrriptor
 */
int make_socket(void) {
	int sock;
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0) ERR("socket");
	return sock;
}

/* make_address
 * Resolve the address given the domain name(name) and the port.
 * returns: address
 */
struct sockaddr_in make_address(char *name, char *port) {
	int ret;
	struct sockaddr_in addr;
	struct addrinfo *result;
	struct addrinfo hints = {};
	hints.ai_family = AF_INET;
	if ((ret = getaddrinfo(name, port, &hints, &result))) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
		exit(EXIT_FAILURE);
	}
	addr = *(struct sockaddr_in *)(result->ai_addr);
	freeaddrinfo(result);
	return addr;
}

/* connect_socket
 * Connect to TCP socket using the resolved address (from make_address)
 * returns: socket descriptor
 */

int connect_socket(char *name, char *port) {
	struct sockaddr_in addr;
	int socketfd;
	socketfd = make_socket();
	addr = make_address(name, port);
	if (connect(socketfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_in)) < 0) {
		if (errno != EINTR) {
			ERR("connect");
		} else {
			/* Interrupted by signal - wait until the connection becomes operational */
			fd_set wfds;
			int status;
			socklen_t size = sizeof(int);
			FD_ZERO(&wfds);
			FD_SET(socketfd, &wfds);
			if (TEMP_FAILURE_RETRY(select(socketfd + 1, NULL, &wfds, NULL, NULL)) < 0) {
				ERR("select");
			}
			if (getsockopt(socketfd, SOL_SOCKET, SO_ERROR, &status, &size) < 0) {
				ERR("getsockopt");
			}
			if (status != 0) {
				ERR("connect");
			}
		}
	}
	return socketfd;
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
