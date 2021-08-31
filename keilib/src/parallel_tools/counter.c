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

typedef struct counter_thread_data_t_{
    counter_uint counter;
    counter_uint countermax;
}counter_thread_data_t;


typedef struct counter_slot_t_{
    uint32_t in_use;
}counter_slot_t;
static counter_slot_t *slot_table = 0;
static uint32_t counter_slot_number = 0;

static uint16_t gblcnt_mutex = 0;

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

    KLOG_I("counter_init done, key %d", counter_key);

    if(!slot_table){
        counter_slot_number += 10;
        slot_table = (counter_slot_t *)calloc(counter_slot_number * sizeof(counter_slot_t));
    }
done:
    spin_unlock16(&gblcnt_mutex);
}

int new_counter(counter_t *counter)
{
    counter_init();
    spin_lock16(&gblcnt_mutex);


    for (int i = 0; i < counter_slot_number; i++)
    {
        if (slot_table[i].in_use == 0)
        {
            counter->index = i;
            slot_table[i].in_use = 1;
            return 0;
        }
    }

    spin_unlock16(&gblcnt_mutex);


    return 0;
failed:
    abort();
}

counter_thread_data_t *get_counter_thread_data(counter_t *counter)
{
    counter_thread_data_t **ctable = pthread_getspecific(counter_key);
    if (!ctable)
    {
        ctable = (counter_thread_data_t **)calloc(counter_slot_number * sizeof(counter_thread_data_t *));
        pthread_setspecific(counter_key, ctable);
    }
    int i = counter->index;
    if (ctable[i])
    {
        goto failed;
    }
    ctable[i] = (counter_thread_data_t *)calloc(1 * sizeof(counter_thread_data_t));
    if (!ctable[i])
    {
        goto failed;
    }

    counter_slot_t* data_base = pthread_getspecific(counter_key);
    return &(data_base[counter->index].counter_thread_data);
}

int add_count(counter_t *counter, counter_uint delta)
{
    if (countermax - counter >= delta)
    {
        WRITE_ONCE(counter, counter + delta);
        return 1;
    }
    spin_lock16(&gblcnt_mutex);

    spin_unlock16(&gblcnt_mutex);
}
