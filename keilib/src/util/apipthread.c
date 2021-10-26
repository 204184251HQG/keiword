#include "util/apipthread.h"
#include "log/keilog.h"
#include <stdio.h>
#include <stdlib.h>

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
        perror("create_thread:pthread_create");
        exit(EXIT_FAILURE);
    }
    return tid;
}

void *wait_thread(thread_id_t tid){
    void *vp;
    if(pthread_join(tid, &vp) != 0){
        perror("wait_thread:pthread_join");
		exit(EXIT_FAILURE);
    }
    return vp;
}

void wait_threads(thread_id_t* tidp, int num){
    for(int i = 0;i<num;i++){
        wait_thread(tidp[i]);
    }
}