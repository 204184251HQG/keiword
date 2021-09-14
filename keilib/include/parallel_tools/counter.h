#ifndef _COUNTER_H_
#define _COUNTER_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"{
#endif
typedef uint32_t counter_uint;

typedef struct counter_t_{
    counter_uint globalcountmax;
    counter_uint globalcount;
    counter_uint globalreserve;
    uint16_t cnt_mutex;
    uint32_t index; //to find thread local value value
} counter_t;


int add_count(counter_t *counter, counter_uint delta);
int sub_count(counter_t *counter, counter_uint delta);
#ifdef __cplusplus
}
#endif

#endif //_COUNTER_H_