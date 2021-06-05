#ifndef SIMPLEFTP_CL_H
#define SIMPLEFTP_CL_H

#define _GNU_SOURCE
#define ERR(source) (perror(source),\
		     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
		     exit(EXIT_FAILURE))
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>

/* BUFSIZE
 * Size of buffer holding a TCP payload
 */
#define BUFSIZE 1000


/* helper-cl.c */
int make_socket(void);
struct sockaddr_in make_address(char *name, char *port);
int connect_socket(char *name, char *port);
int set_handler( void (*f)(int), int sigNo);
ssize_t persist_read(int fd, char *buf, size_t count);
ssize_t persist_write(int fd, char *buf, size_t count);


/* simpleftp-cl.c*/
void sigint_handler(int sig);
void communicate(int fd);
void usage(char *name);



#endif
