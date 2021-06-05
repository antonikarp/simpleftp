#ifndef _SIMPLEFTP_SV_H
#define _SIMPLEFTP_SV_H

#define _GNU_SOURCE
#define ERR(source) (perror(source),\
		     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
		     exit(EXIT_FAILURE))

#include "../common/common.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <pthread.h>

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


struct thread_arg {
	int id;
	int *ptr_cur_client_fd;
	int *ptr_new_request_condition;
	pthread_cond_t *ptr_new_request_cond;
	pthread_mutex_t *ptr_new_request_mutex;
};

struct global_store {
	
	pthread_t *threads;
	struct thread_arg *args;
	
	int cur_client_fd;
	int new_request_condition;
	pthread_cond_t new_request_cond;
	pthread_mutex_t new_request_mutex;
	
	
};

/* helper-sv.c */
int bind_tcp_socket(uint16_t port);
int add_new_client(int sfd);
int calculate_max(int server_fd, int *client_fd);
void close_all_connections (int server_fd, int *client_fd);

/* thread-sv.c */
void* thread_worker(void *void_arg);
void init_threads(struct global_store *store);

/* simpleftp-sv.c */
void usage(char *name);
void run_server(int server_fd, struct global_store *store);
void sigint_handler(int sig);
void initialize_global_store (struct global_store *store);
void deallocate_global_store(struct global_store *store);


#endif
