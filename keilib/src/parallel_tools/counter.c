#include "util.h"
#include "log/keilog.h"
#include "parallel_tools/spinlock.h"
#include "parallel_tools/counter.h"

#include <pthread.h>

#ifdef DBG_COUNTER
#define counter_logd(fmt, arg...) KLOG_D(fmt, ##arg)
#else
#define counter_logd(fmt, arg...)
#endif

__thread counter_uint counter = 0;
__thread counter_uint countermax = 0;

uint16_t gblcnt_mutex = 0;

static pthread_key_t counter_key = 0;

static void counter_destr(void *arr)
{
}

static void counter_init(void)
{
    if (counter_key){
        return 0;
    }
    spin_lock16(&gblcnt_mutex);
    if (counter_key)
    {
        goto done;
    }
    if (pthread_key_create(&counter_key, counter_destr))
    {
        abort();
    }
    
    pthread_setspecific(counter_key, );
    KLOG_I("counter_init done, key %d", counter_key);
done:
    spin_unlock16(&gblcnt_mutex);
}

int new_counter(counter_t *counter)
{
    counter_init();

}

int add_count(counter_t * counter, counter_uint delta)
{
    if(countermax - counter >= delta){
        WRITE_ONCE(counter, counter + delta);
        return 1;
    }
    spin_lock16(&gblcnt_mutex);



    spin_unlock16(&gblcnt_mutex);
}

