#ifndef _COUNTER_H_
#define _COUNTER_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"{
#endif
typedef uint32_t counter_uint;

typedef struct counter_t_{
    counter_uint global_count;
    counter_uint global_reserve;
    uint32_t index; //to find thread local value value
} counter_t;

#ifdef __cplusplus
}
#endif

#endif //_COUNTER_H_