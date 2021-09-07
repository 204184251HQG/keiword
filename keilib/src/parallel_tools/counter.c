#include <stdlib.h>
#include <pthread.h>
#include "util/util.h"
#include "util/keithread.h"
#include "log/keilog.h"
#include "parallel_tools/spinlock.h"
#include "parallel_tools/counter.h"

#ifdef DBG_COUNTER
#define counter_logd(fmt, arg...) KLOG_D(fmt, ##arg)
#else
#define counter_logd(fmt, arg...)
#endif
#define MAX_COUNTERMAX 100
typedef struct counter_thread_data_t_
{
    counter_uint counter;
    counter_uint countermax;
} counter_thread_data_t;

typedef struct counter_thread_datapacked_t_
{
    uint32_t len;
    counter_thread_data_t **data_array;
} counter_thread_datapacked_t;

typedef struct counter_slot_t_
{
    uint32_t in_use;
} counter_slot_t;
static counter_slot_t *slot_table = 0;
static uint32_t counter_slot_number = 0;

static uint16_t gblcnt_mutex = 0;

static pthread_key_t counter_key = 0;

static void counter_destr(void *arr)
{
}

static void counter_init(void)
{
    if (counter_key)
    {
        return;
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

    if (!slot_table)
    {
        counter_slot_number += 10;
        slot_table = (counter_slot_t *)calloc(counter_slot_number, sizeof(counter_slot_t));
    }
done:
    spin_unlock16(&gblcnt_mutex);
}

int new_counter(counter_t *counter, counter_uint max)
{
    counter_init();
    spin_lock16(&gblcnt_mutex);
    int i = 0;

retry:
    for (i; i < counter_slot_number; i++)
    {
        if (slot_table[i].in_use == 0)
        {
            counter->index = i;
            slot_table[i].in_use = 1;
            goto done;
        }
    }

    /* no free slot */
    int append_size = 10;
    slot_table = realloc(slot_table, (append_size + counter_slot_number) * sizeof(counter_slot_t));
    if (!slot_table)
    {
        goto failed;
    }
    memset(slot_table + counter_slot_number, 0, append_size * sizeof(counter_slot_t));
    counter_slot_number += append_size;
    goto retry;

done:
    spin_unlock16(&gblcnt_mutex);

    counter->globalcountmax = max;
    return 0;
failed:
    abort();
}

int delete_counter(counter_t *counter){
    int i = counter->index;
    if(slot_table[i].in_use){
        slot_table[i].in_use = 0;
    }
}

static counter_thread_data_t *get_counter_thread_data(counter_t *counter)
{
    counter_thread_datapacked_t *ctable = pthread_getspecific(counter_key);
    if (!ctable)
    {
        ctable = (counter_thread_datapacked_t *)calloc(1, sizeof(counter_thread_datapacked_t) +
                                                       counter_slot_number * sizeof(counter_thread_data_t *));
        if (!ctable)
        {
            goto failed;
        }
        ctable->len = counter_slot_number;
        pthread_setspecific(counter_key, ctable);
    }
    int i = counter->index;
    if (i >= ctable->len)
    {
        int new_counter_slot_number = counter_slot_number;
        ctable = (counter_thread_datapacked_t *)realloc(ctable, sizeof(counter_thread_datapacked_t) +
                                                        new_counter_slot_number * sizeof(counter_thread_data_t *));
        if (!ctable)
        {
            goto failed;
        }
        memset(ctable->data_array + ctable->len, 0,
               (counter_slot_number - ctable->len) * sizeof(counter_thread_data_t *));
        ctable->len = counter_slot_number;
        pthread_setspecific(counter_key, ctable);
    }
    if (ctable->data_array[i])
    {
        ctable->data_array[i] = (counter_thread_data_t *)calloc(1, sizeof(counter_thread_data_t));
    }
    if (!ctable->data_array[i])
    {
        goto failed;
    }

    ctable = pthread_getspecific(counter_key);
    return (ctable->data_array[i]);

failed:
    abort();
}

static inline void globalize_count(counter_t *counter, counter_thread_data_t *t){
    counter->globalcount += t->counter;
    t->counter = 0;
    counter->globalreserve -= t->countermax;
    t->countermax = 0;
}
static inline void blance_count(counter_t *counter, counter_thread_data_t *t){
    t->countermax = counter->globalcountmax - counter->globalcount - counter->globalreserve;
    t->countermax /= num_online_threads();
    if(t->countermax > MAX_COUNTERMAX){
        t->countermax = MAX_COUNTERMAX;
    }
    counter->globalreserve += t->countermax;
    t->counter = t->countermax / 2;
    if(t->counter > counter->globalcount){
        t->counter = counter->globalcount;
    }
    counter->globalcount -= t->counter;
}

int add_count(counter_t *counter, counter_uint delta)
{
    counter_thread_data_t *t = get_counter_thread_data(counter);

    if (t->countermax - t->counter >= delta)
    {
        WRITE_ONCE(t->counter, t->counter + delta);
        return 1;
    }
    spin_lock16(&counter->cnt_mutex);
    globalize_count(counter, t);
    if(counter->globalcountmax - counter->globalcount - counter->globalreserve < delta){
        spin_unlock16(&counter->cnt_mutex);
        return 0;
    }
    counter->globalcount +=  delta;
    blance_count(counter, t);
    spin_unlock16(&counter->cnt_mutex);
    return 1;
}
