#define _GNU_SOURCE
#define ERR(source) (perror(source),\
		     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
		     exit(EXIT_FAILURE))

#include <sys/socket.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

int make_socket(int domain, int type);
int set_handler( void (*f)(int), int sigNo);
ssize_t persist_read(int fd, char *buf, size_t count);
ssize_t persist_write(int fd, char *buf, size_t count);
