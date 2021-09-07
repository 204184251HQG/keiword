#ifndef _KEIUTIL_H_
#define _KEIUTIL_H_

#include <stdint.h>
#include <stdbool.h>
#include "atomic.h"

#ifdef __cplusplus
extern "C"{
#endif

#define ACCESS_ONCE(x) (*(volatile typeof(x) *)&(x))
#define WRITE_ONCE(dst, src) (ACCESS_ONCE(dst) = (typeof(dst))(src))

#if defined(__x86_64__) || defined(__x86__)
#define LOOP_HINT asm __volatile__ ("pause" ::);
#else
#define LOOP_HINT 
#endif

#ifdef __cplusplus
}
#endif

#endif //_KEIUTIL_H_