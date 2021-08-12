#include "util.h"



int section_lock16(uint16_t *lock) {
    int waitc = 0;
    do {
        if (*lock == 0) {
            if (InterlockedCompareExchange16((SHORT volatile*)lock, 1, 0) == 0) {
                log_debug("lock16ed");
                return 0;
            }
        }
        log_info("lock16 waitting %d", waitc);
        
        do {
            __asm {pause};
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

int section_unlock16(uint16_t *lock) {
    if (InterlockedCompareExchange16((SHORT volatile*)lock, 0, 1) == 0) {
        log_info("unlock16 error, no in lock");
        return -1;
    }
    log_debug("unlock16ed");
    return 0;
}