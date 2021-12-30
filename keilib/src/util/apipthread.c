#include "util/apipthread.h"
#include "log/keilog.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>


void spin_lock_init(spinlock_t *sp)
{
	int ret;

retry:
	ret = pthread_mutex_init(sp, NULL);
	if (ret) {
		if (ret == EINTR)
			goto retry;
		fprintf(stderr, "spin_lock_init:pthread_mutex_init %d\n", ret);
		abort();
	}
}

void spin_lock(spinlock_t *sp)
{
	int en;

	en = pthread_mutex_lock(sp);
	if (en != 0) {
		fprintf(stderr, "pthread_mutex_lock: %s\n", strerror(en));
		perror("spin_lock:pthread_mutex_lock");
		abort();
	}
}

int spin_trylock(spinlock_t *sp)
{
	int retval;

	if ((retval = pthread_mutex_trylock(sp)) == 0)
		return 1;
	if (retval == EBUSY)
		return 0;
	fprintf(stderr, "pthread_mutex_trylock: %s\n", strerror(retval));
	abort();
}

void spin_unlock(spinlock_t *sp)
{
	int en;

	en = pthread_mutex_unlock(sp);
	if (en != 0) {
		fprintf(stderr, "pthread_mutex_unlock: %s\n", strerror(en));
		abort();
	}
}

int spin_is_locked(spinlock_t *sp)
{
	if (spin_trylock(sp)) {
		spin_unlock(sp);
		return 0;
	}
	return 1;
}

int num_enable_cpus(void){
    int num = sysconf(_SC_NPROCESSORS_ONLN);
    //KLOG_I("system enable cpu num is %d", num);
    return num;
}

int num_online_threads(void){
    FILE *status_f = fopen("/proc/self/status", "rb");
    if(!status_f){
        return 1;
    }
    char *line = NULL;
    size_t len = 0;
    while(getline(&line, &len, status_f)>0){
        //KLOG_I("getline %s\n", line);
        const char *fmt_match = "Threads:\t";
        if(strncmp(line, fmt_match, strlen(fmt_match)) == 0){
            const char *tnum_str = line + strlen(fmt_match);
            int tnum = atoi(tnum_str);
            if(tnum){
                //KLOG_I("tnum %d\n", tnum);
                return tnum;
            }
        }
    }
    return 1;
}

thread_id_t create_thread(void *(*func)(void *), void *arg)
{
    thread_id_t tid;
    if (pthread_create(&tid, NULL, func, arg) != 0) {
        KLOG_E("create_thread:pthread_create");
        exit(EXIT_FAILURE);
    }
    return tid;
}

void *wait_thread(thread_id_t tid){
    void *vp;
    if(pthread_join(tid, &vp) != 0){
        KLOG_E("wait_thread:pthread_join");
		exit(EXIT_FAILURE);
    }
    return vp;
}

void wait_threads(thread_id_t* tidp, int num){
    for(int i = 0;i<num;i++){
        wait_thread(tidp[i]);
    }
}