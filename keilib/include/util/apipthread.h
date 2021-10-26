#ifndef _KEITHREAD_H_
#define _KEITHREAD_H_

#define _GNU_SOURCE 
#include <sched.h>
#include <pthread.h>
#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"
#include "log/keilog.h"

#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE 128
#endif /* #ifndef CACHE_LINE_SIZE */

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1


typedef pthread_t thread_id_t;

#ifdef __cplusplus
extern "C"{
#endif
int num_enable_cpus(void);
int num_online_threads(void);
thread_id_t create_thread(void *(*func)(void *), void *arg);
void *wait_thread(thread_id_t tid);
void wait_threads(thread_id_t* tidp, int num);
#ifdef __cplusplus
}
#endif


#define ACCESS_ONCE(x) (*(volatile typeof(x) *)&(x))
#define READ_ONCE(x) \
                ({ typeof(x) ___x = ACCESS_ONCE(x); ___x; })
#define WRITE_ONCE(dst, src) (ACCESS_ONCE(dst) = (typeof(dst))(src))




static __inline__ void run_on(int cpu)
{
    cpu_set_t mask;
    int ret;
    int cpu_max = num_enable_cpus();
    CPU_ZERO(&mask);
    CPU_SET(cpu%cpu_max, &mask);
    ret = sched_setaffinity(0, sizeof(mask), &mask);
    if (ret) {
        KLOG_E("sched_setaffinity");
        abort();
    }
}
/*
 * Per-thread variables.
 */

#define DEFINE_PER_THREAD(type, name) \
    __thread type name;


#define __get_thread_var(name) (name)


#endif // _KEITHREAD_H_