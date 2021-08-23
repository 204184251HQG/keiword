#include "util.h"

#ifdef DBG_SPINLOCK
#include "log/keilog.h"
#define spin_log(fmt, arg...) KLOG_D(fmt, ##arg)
#else
#define spin_log(fmt, arg...)
#endif
int spin_lock16(uint16_t *lock) {
    int waitc = 0;
    do {
        if (*lock == 0) {
            if (atomic_cmpxchg((uint16_t volatile*)lock, 1, 0) == 0) {
                log_debug("lock16ed");
                return 0;
            }
        }
        spin_log("lock16 waitting %d", waitc);
        
        do {
            LOOP_HINT
            if (*lock == 0) {
                break;
            }
            if (waitc++ > 30) {
                waitc = 0;
                SwitchToThread();
            }
        } while (1);
        
    } while (1);
}

int spin_unlock16(uint16_t *lock) {
    if (atomic_cmpxchg((uint16_t volatile*)lock, 0, 1) == 0) {
        spin_log("unlock16 error, no in lock");
        return -1;
    }
    spin_log("unlock16ed");
    return 0;
}