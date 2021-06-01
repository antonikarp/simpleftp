#include "simpleftp-cl.h"

int make_socket(void) {
	int sock;
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0) ERR("socket");
	return sock;
}

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

int connect_socket(char *name, char *port) {
	struct sockaddr_in addr;
	int socketfd;
	socketfd = make_socket();
	addr = make_address(name, port);
	if (connect(socketfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_in)) < 0) {
		if (errno != EINTR) {
			ERR("connect");
		} else {
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
