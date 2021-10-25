
#include "util/apipthread.h"
#include "util/atomic.h"
#include "parallel_tools/counter.h"
#include <stdio.h>
#include <stdlib.h>

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

DEFINE_PER_THREAD(long long, n_reads_pt);
DEFINE_PER_THREAD(long long, n_updates_pt);
unsigned long garbage = 0; /* disable compiler optimizations. */

counter_t counter;

void *count_read_perf_test(void *arg)
{
    int i;
    unsigned long j = 0;
    int cpu = (long)arg;
    long long n_reads_local = 0LL;

    run_on(cpu);
    //count_register_thread();
    atomic_inc(&nthreadsrunning);
    while (READ_ONCE(goflag) == GOFLAG_INIT)
        sleep(1);
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

    return NULL;
}

void *count_update_perf_test(void *arg)
{
    int i;
    long long n_updates_local = 0LL;

    //count_register_thread();
    atomic_inc(&nthreadsrunning);
    while (READ_ONCE(goflag) == GOFLAG_INIT)
        sleep(1);
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
    return NULL;
}

void perftestrun(int nthreads, int nreaders, int nupdaters)
{
    int exitcode = EXIT_SUCCESS;
    int t;
    int duration = 240;

    smp_mb();
    while (atomic_read(&nthreadsrunning) < nthreads)
        poll(NULL, 0, 1);
    goflag = GOFLAG_RUN;
    smp_mb();
    poll(NULL, 0, duration);
    smp_mb();
    goflag = GOFLAG_STOP;
    smp_mb();
    wait_all_threads();
    for_each_thread(t) {
        n_reads += per_thread(n_reads_pt, t);
        n_updates += per_thread(n_updates_pt, t);
    }
    n_updates *= 2;  /* Each update includes an add and a subtract. */
    if (read_count() != 0) {
        printf("!!! Count mismatch: 0 counted vs. %lu final value\n",
               read_count());
        exitcode = EXIT_FAILURE;
    }
    printf("n_reads: %lld  n_updates: %lld  nreaders: %d  nupdaters: %d duration: %d\n",
           n_reads, n_updates, nreaders, nupdaters, duration);
    printf("ns/read: %g  ns/update: %g\n",
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

    atomic_set(nthreadsrunning, 0);
    nthreadsexpected = nreaders + 1;
    for (i = 0; i < nreaders; i++) {
        arg = (long)(i * cpustride);
        create_thread(count_read_perf_test, (void *)arg);
    }
    arg = (long)(i * cpustride);
    create_thread(count_update_perf_test, (void *)arg);
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
    int cpustride = 6;

    usage(argc, argv);
    counter_init();
    new_counter(&counter, 0x1000000);
    
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
        
    }
    perftest(nreaders, cpustride);
    return 0;
}
