#include "simpleftp-sv.h"

extern volatile sig_atomic_t do_work_server;

/* thread_worker
 * Each incoming request from a client is serviced by this function/
 */
void* thread_worker(void *void_arg) {
	struct thread_arg *targ = (struct thread_arg *) void_arg;
	char buf[BUFSIZE];
	int cfd;
	for (;;) {
		while (!(*(targ->ptr_new_request_condition)) && do_work_server) {
			if (pthread_cond_wait(targ->ptr_new_request_cond, targ->ptr_new_request_mutex) != 0) {
				ERR("pthread_cond_wait");
			}
		}
		if (!do_work_server) {
			pthread_exit(NULL);
		}
		cfd = targ->ptr_client_fd[*(targ->ptr_cur_client_i)];
		memset(buf, 0, BUFSIZE);
		int read_bytes = persist_read(cfd, buf, BUFSIZE);
		
		// Finished reading. Unlock the main thread.
		if (sem_post(targ->ptr_sem) != 0) {
			ERR("sem_post");
		}
		
		*(targ->ptr_new_request_condition) = 0;
		
		// Handle disconnection.
		if (read_bytes == 0) {
			targ->ptr_client_fd[*(targ->ptr_cur_client_i)] = 0;
			if (close(cfd) < 0) {
				ERR("close");
			}
			FD_CLR(cfd, targ->ptr_base_rfds);
			
		}
		// At this point we no longer modify shared data.
		if (pthread_mutex_unlock(targ->ptr_new_request_mutex) != 0) {
			ERR("pthread_mutex_unlock");
		}
		if (read_bytes == 0) {
			continue;
		}
		// Handle ls command
		if (!strcmp(buf, "ls")) {
			handle_ls(buf, cfd);
		}
		
		char *hello = "Hello from a thread\n";
		persist_write(cfd, hello, strlen(hello) + 1);
	}
}

void handle_ls(char *buf, int cfd) {
	FILE* file = popen("ls", "r");
	if (!file) {
		ERR("popen");
	}
	memset(buf, 0, BUFSIZE);
	buf[0] = '\n';
	int offset = 1;
	char intermediate[BUFSIZE];
	memset(intermediate, 0, BUFSIZE);
	while ( fscanf(file, "%s", intermediate) != EOF) {
		intermediate[strlen(intermediate)] = '\n';
		strncpy(buf + offset, intermediate, strlen(intermediate));
		offset += strlen(intermediate);
		memset(intermediate, 0, BUFSIZE);
	} 
	persist_write(cfd, buf, strlen(buf) + 1);
	if (pclose(file) == -1){
		ERR("pclose");
	}	
}


