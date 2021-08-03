#include "log/keilog.h"
#include "parallel_tools/counter.h"

typedef uint32_t counter_uint;

__thread counter_uint counter = 0;

int add_count(counter_uint delta)
{}

