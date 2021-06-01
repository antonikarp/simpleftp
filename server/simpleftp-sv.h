#ifndef _SIMPLEFTP_SV_H
#define _SIMPLEFTP_SV_H

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
#include <netinet/in.h>


#define BACKLOG 3		    


/* helper-sv.c */
int make_socket(int domain, int type);
int bind_tcp_socket(uint16_t port);
int add_new_client(int sfd);


/* simpleftp-sv.c */


#endif
