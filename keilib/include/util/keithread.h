#ifndef _KEITHREAD_H_
#define _KEITHREAD_H_

#include <pthread.h>
#include <sched.h>
#include <sys/param.h>

#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE 128
#endif /* #ifndef CACHE_LINE_SIZE */

typedef pthread_t thread_id_t;

int num_online_threads(void);

static thread_id_t create_thread(void *(*func)(void *), void *arg)
{
    thread_id_t tid;
    if (pthread_create(&tid, NULL, func, arg) != 0) {
		perror("create_thread:pthread_create");
		exit(EXIT_FAILURE);
	}
    return tid;
}


#endif // _KEITHREAD_H_