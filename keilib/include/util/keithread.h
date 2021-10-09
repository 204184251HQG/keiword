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



#endif // _KEITHREAD_H_