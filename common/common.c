#define _GNU_SOURCE
#define ERR(source) (perror(source),\
		     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
		     exit(EXIT_FAILURE))

#include "common.h"

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
