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

#define ID_MASK 0x80000000
typedef struct counter_thread_datapacked_t_
{
    uint32_t id; // need to add with ID_MASK
    uint32_t len;
    counter_thread_data_t **data_array;
} counter_thread_datapacked_t;

typedef struct counter_slot_t_
{
    counter_t *counter_g;
    counter_lock cnt_mutex;
} counter_slot_t;

static int thread_map[COUNTER_MAX_THREADS] = {0};
static counter_lock thread_map_mutex = 0;
static counter_slot_t *slot_table = 0;
static uint32_t counter_slot_number = 0;

static counter_lock gblcnt_mutex = 0;

static pthread_key_t counter_key = 0;


static inline void counter_thread_data_reset(counter_thread_data_t *tdata){
    if(!tdata){
        return;
    }
    tdata->counter = 0;
    tdata->countermax = 0;
}

static int is_counter_valid(counter_t *counter)// delete ?
{
    if(counter->index == (typeof(counter->index))-1){
        return 0;
    }
    return 1;
}

static int register_to_thread_map(counter_thread_datapacked_t *ctable){
    int i = 0;
    pthread_t tid = pthread_self();
    
    spin_lock16(&thread_map_mutex);
    for (i = 0; i < COUNTER_MAX_THREADS; i++) {
        if(thread_map[i]){
            continue;
        }
        thread_map[i] = tid;
        break;
    }
    spin_unlock16(&thread_map_mutex);

    if(i == COUNTER_MAX_THREADS){
        return -1;
    }

    ctable->id = i + ID_MASK;
    return 0;
}

static int unregister_to_thread_map(){
    pthread_t tid = pthread_self();
    
    spin_lock16(&thread_map_mutex);
    for (int t = 0; t < COUNTER_MAX_THREADS; t++) {
        if(thread_map[t] && (thread_map[t] == tid)){
            thread_map[t] = 0;
        }
    }
    spin_unlock16(&thread_map_mutex);
}


int delete_counter(counter_t *counter)
{
    if(!is_counter_valid(counter)){
        return 0;
    }
    int idx = counter->index;
    counter_lock *tlockp = &slot_table[counter->index].cnt_mutex;
    
    spin_lock16(tlockp);
    if (slot_table[idx].counter_g){
        slot_table[idx].counter_g = 0;
    }
    counter->index = (typeof(counter->index))-1;
    for(int i = 0;i<COUNTER_MAX_THREADS;i++){
        counter_thread_data_reset(counter->counter_threadp[i]);
    }
    spin_unlock16(tlockp);
    return 0;
}

static counter_thread_data_t *get_counter_thread_data(counter_t *counter)
{
    counter_thread_datapacked_t *ctable = pthread_getspecific(counter_key);
    if (!ctable)
    {
        int new_counter_slot_number = counter_slot_number;

        ctable = (counter_thread_datapacked_t *)calloc(1, sizeof(counter_thread_datapacked_t) +
                                                       new_counter_slot_number * sizeof(counter_thread_data_t *));
        if (!ctable)
        {
            goto failed;
        }
        ctable->len = new_counter_slot_number;
        pthread_setspecific(counter_key, ctable);
        if(register_to_thread_map(ctable)){
            goto failed;
        }
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
               (new_counter_slot_number - ctable->len) * sizeof(counter_thread_data_t *));
        ctable->len = new_counter_slot_number;
        pthread_setspecific(counter_key, ctable);
    }
    if (!ctable->data_array[i])
    {
        ctable->data_array[i] = (counter_thread_data_t *)calloc(1, sizeof(counter_thread_data_t));
        if (!ctable->data_array[i])
        {
            goto failed;
        }
    }

    if(counter->counter_threadp[ctable->id-ID_MASK] == 0){
        counter->counter_threadp[ctable->id-ID_MASK] = ctable->data_array[i];
    }

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
    if(!is_counter_valid(counter)){
        return 1;
    }
    counter_thread_data_t *t = get_counter_thread_data(counter);
    counter_lock *tlockp = &slot_table[counter->index].cnt_mutex;
    if (t->countermax - t->counter >= delta)
    {
        WRITE_ONCE(t->counter, t->counter + delta);
        return 1;
    }
    spin_lock16(tlockp);
    globalize_count(counter, t);
    if(counter->globalcountmax - counter->globalcount - counter->globalreserve < delta){
        spin_unlock16(tlockp);
        return 0;
    }
    counter->globalcount +=  delta;
    blance_count(counter, t);
    spin_unlock16(tlockp);
    return 1;
}

int sub_count(counter_t *counter, counter_uint delta){
    if(!is_counter_valid(counter)){
        return 1;
    }
    counter_thread_data_t *t = get_counter_thread_data(counter);
    counter_lock *tlockp = &slot_table[counter->index].cnt_mutex;
    if(t->counter >= delta){
        WRITE_ONCE(t->counter, t->counter - delta);
        return 1;
    }
    spin_lock16(tlockp);
    globalize_count(counter, t);
    if (counter->globalcount < delta){
        spin_unlock16(tlockp);
        return 0;
    }
    counter->globalcount -= delta;
    blance_count(counter, t);
    spin_unlock16(tlockp);
    return 1;
}

counter_uint read_count(counter_t *counter){
    if(!is_counter_valid(counter)){
        return 1;
    }
    counter_thread_data_t *t = get_counter_thread_data(counter);
    counter_lock *tlockp = &slot_table[counter->index].cnt_mutex;
	counter_uint sum;
    spin_lock16(tlockp);
    sum = counter->globalcount;
    for(int t = 0; t < COUNTER_MAX_THREADS; t++){
        if(counter->counter_threadp[t])
            sum += (counter->counter_threadp[t]->counter);
    }
    spin_unlock16(tlockp);
}

static void counter_destr(void *arr)
{
    counter_thread_datapacked_t *ctable = (counter_thread_datapacked_t *)arr;
    if(!ctable){
        return;
    }
    int i = 0;
    for(i;i<ctable->len;i++){
        if(ctable->data_array[i]){
            if (slot_table[i].counter_g)
            {// globalize it 
                counter_lock *tlockp = &slot_table[i].cnt_mutex;
                spin_lock16(tlockp);
                counter_t *counter = slot_table[i].counter_g;
                if(counter){
                    globalize_count(counter, ctable->data_array[i]);
                }
                spin_unlock16(tlockp);
            }
            free(ctable->data_array[i]);
            ctable->data_array[i] = 0;
        }
    }
    free(ctable);
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
        if(counter_slot_number){//should be zero
            abort();
        }
        counter_slot_number = 10;
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

    memset(counter, 0, sizeof(counter_t));
retry:
    for (i; i < counter_slot_number; i++)
    {
        if (slot_table[i].counter_g == 0)
        {
            counter_lock *tlockp = &slot_table[i].cnt_mutex;
            spin_lock16(tlockp);
            if (slot_table[i].counter_g == 0)
            {
                counter->index = i;
                slot_table[i].counter_g = counter;
            }
            spin_unlock16(tlockp);

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