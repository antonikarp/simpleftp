#include "simpleftp-cl.h"

volatile sig_atomic_t do_work_client = 1;

void sigint_handler(int sig) {
	do_work_client = 0;
}
	

void usage(char *name) {
	fprintf(stderr, "USAGE: %s domain port\n", name);
	exit(EXIT_FAILURE);
}

void communicate(int fd) {
	char buf[BUFSIZE];
	while (do_work_client) {
		memset(buf, 0, BUFSIZE);
		if (fgets(buf, BUFSIZE, stdin) > 0) {
			if (buf[strlen(buf) - 1] == '\n') {
				buf[strlen(buf) - 1] = '\0';
			}
			persist_write(fd, (void *) buf, sizeof(buf));
			int bytes = persist_read(fd, buf, BUFSIZE);
			if (bytes == 0) {
				break;
			}
			printf("%s\n", buf);
		} else {
			 if (errno == EINTR) {
				 continue;
			} else {
				ERR("fgets");
			}
		}
	}
	if (close(fd) < 0) {
		ERR("close");
	}
	printf("The client has been terminated.\n");	
}

int main(int argc, char **argv) {
	if (set_handler(sigint_handler, SIGINT)) {
		ERR("set_handler");
	}
	if (set_handler(SIG_IGN, SIGPIPE)) {
		ERR("set_handler");
	}
	if (argc != 3) {
		usage(argv[0]);
	}
	int client_fd = connect_socket(argv[1], argv[2]);
	char buf[BUFSIZE];
	memset(buf, 0, BUFSIZE);
	persist_read(client_fd, buf, BUFSIZE);
	printf("%s\n", buf);
	communicate(client_fd);
	return EXIT_SUCCESS;
}
