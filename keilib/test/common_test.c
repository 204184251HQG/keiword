
#include "util/apipthread.h"
#include "log/keilog.h"
#include <stdint.h>

int main()
{
    int32_t index = 0;
    int32_t total = 200;
    int32_t min = 0;
    while (index <= total)
    {
        min =  kmin(index, total);
        KLOG_I("min is = %d  max is = %d\n", min, kmax(index, total));
        index++;
    }
    int tnum = num_online_threads();
    KLOG_I("t %d", tnum);
    return 0;
}