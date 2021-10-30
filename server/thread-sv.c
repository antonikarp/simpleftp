#include "simpleftp-sv.h"

extern volatile sig_atomic_t do_work_server;

/* thread_worker
 * Each incoming request from a client is serviced by this function
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
		
		// Tokenize the command into two parts separated by a space
		char *part1, *part2;
		part1 = strtok(buf, " ");
		if (!part1) {
			continue;
		}
		part2 = buf + strlen(part1) + 1;
		
		// Handle get command
		if (!strcmp(part1, "get")) {
			handle_get_sv(part2, cfd);
		}
		// Send a whitespace to be consumed by the client.
		persist_write(cfd, " ", 2);
	}
}
/* handle_ls
 * List all file of the working directory and send the result to the client.
 */
void handle_ls(char *buf, int cfd) {
	// Run ls command. The currecnt working directory has already been changed.
	FILE* file = popen("ls", "r");
	
	if (!file) {
		ERR("popen");
	}
	memset(buf, 0, BUFSIZE);
	buf[0] = '\n';
	int offset = 1;
	char intermediate[BUFSIZE];
	memset(intermediate, 0, BUFSIZE);
	
	// Append the results of the ls command line by line to the buffer.
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

/* handle_get_sv
 * Read the selected file and send its contents to the client.
 */
void handle_get_sv(char *filename, int cfd) {
	if (strlen(filename) == 0) {
		return;
	}
	FILE* read_file = fopen(filename, "r+");
	if (!read_file) {
		ERR("fopen");
	}
	int end_condition = 0;
	char buf[BUFSIZE];
	char *size_ptr;
	int buf_i = 3;
	int16_t total_bytes = 0;
	while (!end_condition) {
		memset(buf, 0, BUFSIZE);
		total_bytes = 0;
		buf_i = 3;
		while (total_bytes < BUFSIZE - 3 && !end_condition){
			int read_bytes = fread(&buf[buf_i], 1, 1, read_file);
			if (read_bytes != 1) {
				end_condition = 1;
			} else {
				++buf_i;
				++total_bytes;
			}
		}
		size_ptr = (char *) &total_bytes;
		buf[1] = size_ptr[0];
		buf[2] = size_ptr[1];
		if (!end_condition) {
			// This indicates that more packets are incoming.
			buf[0] = '0';
		} else {
			// This indicates that this is the last packet.
			buf[0] = '1';
		}

		persist_write(cfd, buf, total_bytes + 3);
	}
	if (fclose(read_file)) {
		ERR("fclose");
	}
}


