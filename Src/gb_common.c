#include "gb_common.h"
#include <time.h>
#include <math.h>

// clock is 4MHz so one tick is 250ns
unsigned long get_ns(){
    struct timespec _t;
    clock_gettime(CLOCK_REALTIME, &_t);
    return ((_t.tv_sec*1000000000 + lround(_t.tv_nsec)));
}
