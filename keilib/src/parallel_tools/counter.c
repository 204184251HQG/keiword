#include "util.h"
#include "log/keilog.h"
#include "parallel_tools/counter.h"

typedef uint32_t counter_uint;

__thread counter_uint counter = 0;
__thread counter_uint countermax = 0;

int add_count(counter_uint delta)
{
    if(countermax - counter >= delta){
        WRITE_ONCE(counter, counter + delta);
        return 1;
    }
    

}

