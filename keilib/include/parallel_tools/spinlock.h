#ifndef _SPIN_H_
#define _SPIN_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"{
#endif

#define LOCK16_UNLOCK (0)
#define LOCK16_LOCKED (1)

int spin_lock16(uint16_t *lock);

int spin_unlock16(uint16_t *lock);
#ifdef __cplusplus
}
#endif

#endif //_SPIN_H_