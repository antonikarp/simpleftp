#include "simpleftp-cl.h"


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
	socketfd = make_socket(PF_INET, SOCK_STREAM);
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
