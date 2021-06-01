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


/* helper-cl.c */
int make_socket(void);
struct sockaddr_in make_address(char *name, char *port);
int connect_socket(char *name, char *port);


/* simpleftp-cl.c*/




#endif
