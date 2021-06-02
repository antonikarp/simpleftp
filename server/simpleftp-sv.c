#include "simpleftp-sv.h"


void usage(char *name) {
	fprintf(stderr, "USAGE: %s port workdir\n", name);
	exit(EXIT_FAILURE);
}

void runServer(int server_fd) {
	int client_fd;
	char data[6];
	memset(data, 0, 6);
	strcpy(data, "Hello");
	client_fd = add_new_client(server_fd);
	
	if (client_fd == -1) {
		// O_NONBLOCK is set for server_fd and there are no connections
		// to accept.
		exit(EXIT_FAILURE);
	}
	if (write(client_fd, data, 6) < 0) {
		ERR("write");
	}
	if (close(client_fd) < 0) {
		ERR("close");
	}
	
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
