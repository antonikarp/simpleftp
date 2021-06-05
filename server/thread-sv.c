#include "simpleftp-sv.h"

extern volatile sig_atomic_t do_work;

/* thread_worker
 * Each incoming request from a client is serviced by this function/
 */
void* thread_worker(void *void_arg) {
	struct thread_arg *targ = (struct thread_arg *) void_arg;
	char buf[BUFSIZE];
	int cfd;
	for (;;) {
		while (!(*(targ->ptr_new_request_condition)) && do_work) {
			if (pthread_cond_wait(targ->ptr_new_request_cond, targ->ptr_new_request_mutex) != 0) {
				ERR("pthread_cond_wait");
			}
		}
		
		*(targ->ptr_new_request_condition) = 0;
		if (!do_work) {
			pthread_exit(NULL);
		}
		
		cfd = *(targ->ptr_cur_client_fd);
		if (pthread_mutex_unlock(targ->ptr_new_request_mutex) != 0) {
			ERR("pthread_mutex_unlock");
		}
		persist_read(cfd, buf, BUFSIZE);
		char *hello = "Hello from a thread\n";
		persist_write(cfd, hello, strlen(hello) + 1);
	}
}

/* init_threads
 * Intialize idle, detached threads.
 */
void init_threads(struct global_store *store) {
	for (int i = 0; i < MAXCL; i++) {
		pthread_attr_t thread_attr;
		if (pthread_attr_init(&thread_attr)) {
			ERR("pthread_attr_init");
		}
		if (pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED)) {
			ERR("pthread_attr_setdetachstate");
		}
		store->args[i].id = i;
		store->args[i].ptr_cur_client_fd = &(store->cur_client_fd);
		store->args[i].ptr_new_request_condition = &(store->new_request_condition);
		store->args[i].ptr_new_request_cond = &(store->new_request_cond);
		store->args[i].ptr_new_request_mutex = &(store->new_request_mutex);
		if (pthread_create(&(store->threads[i]), &thread_attr, thread_worker, (void *) &(store->args[i])) != 0) {
			ERR("pthread_create");
		}
		pthread_attr_destroy(&thread_attr);
	}
	
}
