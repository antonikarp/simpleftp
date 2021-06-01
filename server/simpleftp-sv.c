#include "simpleftp-sv.h"


void usage(char *name) {
	fprintf(stderr, "USAGE: %s port workdir\n", name);
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
	if (argc != 2) {
		usage(argv[0]);
	}
	
	return EXIT_SUCCESS;
}
