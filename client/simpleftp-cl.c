#include "simpleftp-cl.h"

volatile sig_atomic_t do_work = 1;

void sigint_handler(int sig) {
	do_work = 0;
}
	

void usage(char *name) {
	fprintf(stderr, "USAGE: %s domain port\n", name);
	exit(EXIT_FAILURE);
}

void communicate(int fd) {
	char buf[BUFSIZE];
	while (do_work) {
		memset(buf, 0, BUFSIZE);
		if (fgets(buf, BUFSIZE, stdin) > 0) {
			if (buf[strlen(buf) - 1] == '\n') {
				buf[strlen(buf) - 1] = '\0';
			}
			if (write(fd, (void *) buf, BUFSIZE) < 0) {
				ERR("write");
			}
			read(fd, buf, BUFSIZE);
			printf("%s\n", buf);
		} else {
			 if (errno == EINTR) {
				 continue;
			} else {
				ERR("fgets");
			}
		}
	}
	printf("The client has been terminated.\n");
	
}

int main(int argc, char **argv) {
	if (set_handler(sigint_handler, SIGINT)) {
		ERR("set_handler");
	}
	
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
	communicate(client_fd);
	if (close(client_fd) < 0) {
		ERR("close");
	} 
	return EXIT_SUCCESS;
}
