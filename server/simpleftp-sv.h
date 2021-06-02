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

/* BACKLOG
 * Limit the number of outstading connections in the socket's
 * listen queue
 */

#define BACKLOG 3		    

/* MAXCL
 * Maximum number of simultaneous clients
 */
 
#define MAXCL 3

/* BUFSIZE
 * Size of buffer holding a TCP payload
 */
#define BUFSIZE 1000

/* helper-sv.c */
int make_socket(int domain, int type);
int bind_tcp_socket(uint16_t port);
int add_new_client(int sfd);
int calculate_max(int server_fd, int *client_fd);


/* simpleftp-sv.c */
void usage(char *name);
void runServer(int server_fd);


#endif
