#include "simpleftp-cl.h"

void usage(char *name) {
	fprintf(stderr, "USAGE: %s domain port\n", name);
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
	if (argc != 3) {
		usage(argv[0]);
	}
	int client_fd = connect_socket(argv[1], argv[2]);
	char buf[BUFSIZE];
	memset(buf, 0, BUFSIZE);
	if (read(client_fd, buf, BUFSIZE) < 0) {
		ERR("write");
	}
	printf("%s\n", buf);
	if (close(client_fd) < 0) {
		ERR("close");
	} 
	return EXIT_SUCCESS;
}
