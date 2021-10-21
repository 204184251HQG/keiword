
#include <stdio.h>
#include <stdlib.h>
#include "parallel_tools/counter.h"
#include "util/keithread.h"
#include "util/atomic.h"

atomic_t nthreadsrunning;


#define GOFLAG_INIT	0
#define GOFLAG_HOG	1
#define GOFLAG_RUN_UP	2
#define GOFLAG_RUN_DOWN	3
#define GOFLAG_RUN	4
#define GOFLAG_STOP	5

int goflag __attribute__((__aligned__(CACHE_LINE_SIZE))) = 0;

#define COUNT_READ_RUN   1000
#define COUNT_UPDATE_RUN 1000 

void *count_read_perf_test(void *arg)
{
	int i;
	unsigned long j = 0;
	int me = (long)arg;
	long long n_reads_local = 0LL;

	run_on(me);
	count_register_thread();
	atomic_inc(&nthreadsrunning);
	while (READ_ONCE(goflag) == GOFLAG_INIT)
		poll(NULL, 0, 1);
	while (READ_ONCE(goflag) == GOFLAG_RUN) {
		for (i = COUNT_READ_RUN; i > 0; i--) {
			j += read_count();
			barrier();
		}
		n_reads_local += COUNT_READ_RUN;
	}
	__get_thread_var(n_reads_pt) += n_reads_local;
	count_unregister_thread(nthreadsexpected);
	garbage += j;

	return NULL;
}


void perftest(int nreaders, int cpustride)
{
	int i;
	long arg;

	perftestinit(nreaders + 1);
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

	smp_init();
	count_init();

	if (argc > 1) {
		nreaders = strtoul(argv[1], NULL, 0);
		if (argc == 2)
			perftest(nreaders, cpustride);
		if (argc > 3)
			cpustride = strtoul(argv[3], NULL, 0);
		if (strcmp(argv[2], "perf") == 0)
			perftest(nreaders, cpustride);
		else if (strcmp(argv[2], "rperf") == 0)
			rperftest(nreaders, cpustride);
		else if (strcmp(argv[2], "uperf") == 0)
			uperftest(nreaders, cpustride);
		else if (strcmp(argv[2], "hog") == 0)
			hogtest(nreaders, cpustride);
		usage(argc, argv);
	}
	perftest(nreaders, cpustride);
	return 0;
}
