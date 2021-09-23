#ifndef _COUNTER_H_
#define _COUNTER_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"{
#endif
typedef uint32_t counter_uint;
#define COUNTER_MAX_THREADS 512
struct counter_thread_data_t_;
typedef struct counter_t_{
    counter_uint globalcountmax;
    counter_uint globalcount;
    counter_uint globalreserve;
    uint16_t cnt_mutex;
    uint32_t index; //index number of the counter in all counter
    struct counter_thread_data_t_ *counter_threadp[COUNTER_MAX_THREADS];
} counter_t;

int new_counter(counter_t *counter, counter_uint max);
int add_count(counter_t *counter, counter_uint delta);
int sub_count(counter_t *counter, counter_uint delta);
#ifdef __cplusplus
}
#endif

#endif //_COUNTER_H_