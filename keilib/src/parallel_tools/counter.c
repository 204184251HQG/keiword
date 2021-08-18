#include "util.h"
#include "log/keilog.h"
#include "parallel_tools/counter.h"

#include <pthread.h>

__thread counter_uint counter = 0;
__thread counter_uint countermax = 0;



static pthread_key_t counter_key = 0;

static void counter_destr(void *arr)
{
    
}

static
void counter_init(void)
{
    if (pthread_key_create(&counter_key, counter_destr))
    {
        abort();
    }
    MINITLS_DBG("counter_init done");
}

int add_count(counter_uint delta)
{
    if(countermax - counter >= delta){
        WRITE_ONCE(counter, counter + delta);
        return 1;
    }
    spin_lock();

}

