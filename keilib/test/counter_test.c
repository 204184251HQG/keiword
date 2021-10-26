
#include "util/apipthread.h"
#include "util/atomic.h"
#include "log/keilog.h"
#include "parallel_tools/counter.h"
#include "parallel_tools/spinlock.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

atomic_t nthreadsrunning;
int nthreadsexpected;


#define GOFLAG_INIT    0
#define GOFLAG_HOG    1
#define GOFLAG_RUN_UP    2
#define GOFLAG_RUN_DOWN    3
#define GOFLAG_RUN    4
#define GOFLAG_STOP    5

int goflag __attribute__((__aligned__(CACHE_LINE_SIZE))) = 0;

#define COUNT_READ_RUN   1000
#define COUNT_UPDATE_RUN 1000 

uint16_t result_lock = LOCK16_UNLOCK;
DEFINE_PER_THREAD(long long, n_reads_pt);
DEFINE_PER_THREAD(long long, n_updates_pt);
long long n_reads = 0LL;
long long n_updates = 0LL;
unsigned long garbage = 0; /* disable compiler optimizations. */

counter_t counter;

thread_id_t *tids = NULL;

void *count_read_perf_test(void *arg)
{
    int i;
    unsigned long j = 0;
    int cpu = (long)arg;
    long long n_reads_local = 0LL;

    KLOG_I("new thread cpu %d", cpu);
    run_on(cpu);
    //count_register_thread();
    atomic_inc(&nthreadsrunning);
    while (READ_ONCE(goflag) == GOFLAG_INIT)
        usleep(1000);
    while (READ_ONCE(goflag) == GOFLAG_RUN) {
        for (i = COUNT_READ_RUN; i > 0; i--) {
            j += read_count(&counter);
            barrier();
        }
        n_reads_local += COUNT_READ_RUN;
    }
    __get_thread_var(n_reads_pt) += n_reads_local;
    //count_unregister_thread(nthreadsexpected);
    garbage += j;

    spin_lock16(&result_lock);
    n_reads += n_reads_pt;
    spin_unlock16(&result_lock);


    return NULL;
}

void *count_update_perf_test(void *arg)
{
    int i;
    long long n_updates_local = 0LL;

    //count_register_thread();
    atomic_inc(&nthreadsrunning);
    while (READ_ONCE(goflag) == GOFLAG_INIT)
        usleep(1000);
    while (READ_ONCE(goflag) == GOFLAG_RUN) {
        for (i = COUNT_UPDATE_RUN; i > 0; i--) {
            add_count(&counter, 1);
            sub_count(&counter, 1);
            barrier();
        }
        n_updates_local += COUNT_UPDATE_RUN;
    }
    __get_thread_var(n_updates_pt) += n_updates_local;
    //count_unregister_thread(nthreadsexpected);
    spin_lock16(&result_lock);
    n_updates += n_updates_pt * 2;/* Each update includes an add and a subtract. */
    spin_unlock16(&result_lock);
    return NULL;
}

void perftestrun(int nthreads, int nreaders, int nupdaters)
{
    int exitcode = EXIT_SUCCESS;
    int t;
    int duration = 300;

    smp_mb();
    while (atomic_read(&nthreadsrunning) < nthreads)
        usleep(1000);
    goflag = GOFLAG_RUN;
    smp_mb();
    usleep(duration*1000);
    smp_mb();
    goflag = GOFLAG_STOP;
    smp_mb();
    wait_threads(tids, nthreads);
    if (read_count(&counter) != 0) {
        KLOG_E("!!! Count mismatch: 0 counted vs. %lu final value\n",
               (long unsigned int)read_count(&counter));
        exitcode = EXIT_FAILURE;
    }
    KLOG_I("n_reads: %lld  n_updates: %lld  nreaders: %d  nupdaters: %d duration: %d",
           n_reads, n_updates, nreaders, nupdaters, duration);
    KLOG_I("ns/read: %g  ns/update: %g",
           ((duration * 1000*1000.*(double)nreaders) /
            (double)n_reads),
           ((duration * 1000*1000.*(double)nupdaters) /
            (double)n_updates));
    exit(exitcode);
}

void perftest(int nreaders, int cpustride)
{
    int i;
    long arg;

    atomic_set(&nthreadsrunning, 0);
    nthreadsexpected = nreaders + 1;

    tids = calloc(nthreadsexpected, sizeof(thread_id_t));
    for (i = 0; i < nreaders; i++) {
        arg = (long)(i * cpustride);
        tids[i] = create_thread(count_read_perf_test, (void *)arg);
    }
    arg = (long)(i * cpustride);
    tids[i] = create_thread(count_update_perf_test, (void *)arg);
    perftestrun(i + 1, nreaders, 1);
}

/*
 * Mainprogram.
 */

void usage(int argc, char *argv[])
{
    fprintf(stderr,
        "Usage: %s [nreaders [ perf [ cpustride ] ] ]\n", argv[0]);
    fprintf(stderr,
        "Usage: %s [nreaders [ rperf [ cpustride ] ] ]\n", argv[0]);
    fprintf(stderr,
        "Usage: %s [nreaders [ uperf [ cpustride ] ] ]\n", argv[0]);
    fprintf(stderr,
        "Usage: %s [nreaders [ hog [ cpustride ] ] ]\n", argv[0]);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    int nreaders = 6;
    int cpustride = 1;

    new_counter(&counter, 0x1000000);
    KLOG_I("current enable cpu num %d", num_enable_cpus());
    if (argc > 1) {
        nreaders = strtoul(argv[1], NULL, 0);
        if (argc == 2)
            perftest(nreaders, cpustride);
        if (argc > 3)
            cpustride = strtoul(argv[3], NULL, 0);
        if (strcmp(argv[2], "perf") == 0)
            perftest(nreaders, cpustride);
        else if (strcmp(argv[2], "rperf") == 0){
            //rperftest(nreaders, cpustride);
        }
        else if (strcmp(argv[2], "uperf") == 0){
            //uperftest(nreaders, cpustride);
        }
        else if (strcmp(argv[2], "hog") == 0){
            //hogtest(nreaders, cpustride);
        }
        usage(argc, argv);
        
    }
    perftest(nreaders, cpustride);
    return 0;
}
