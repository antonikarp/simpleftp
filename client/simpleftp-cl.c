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
			if (strlen(buf) == 0) {
				continue;
			}
			char *part1, *part2;
			part1 = strtok(buf, " ");
			part2 = buf + strlen(part1) + 1;
			if (!strcmp(part1, "get")) {
				handle_get_cl(fd, part2);
				continue;
			}
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

void handle_get_cl(int fd, char* filename) {
	if (strlen(filename) == 0) {
		return;
	}
	FILE* new_file = fopen(filename, "w+");
	if (!new_file) {
		ERR("fopen");
	}
	int end_condition = 0;
	char buf[BUFSIZE];
	uint16_t size;
	char size_char[2];
	while (!end_condition) {
		persist_read(fd, buf, BUFSIZE);
		if (buf[0] == '1') {
			end_condition = 1;
		}
		size_char[0] = buf[1];
		size_char[1] = buf[2];
		size = *(uint16_t *) size_char;
		for(int i = 3; i < 3 + size; ++i) {
			fwrite(&buf[i], 1, 1, new_file);
		}
	}
	if (fclose(new_file)) {
		ERR("fclose");
	}
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
