#include "gb_common.h"
#include <time.h>
#include <math.h>
#include <errno.h>

// clock is 4MHz so one tick is 250ns
unsigned long get_ns(){
    struct timespec _t;
    clock_gettime(CLOCK_REALTIME, &_t);
    return ((_t.tv_sec*1000000000 + lround(_t.tv_nsec)));
}

void sleep_ns(unsigned long ns){
    struct timespec request;

    request.tv_sec  = (time_t)(ns / 1000000000UL);
    request.tv_nsec = (long)(ns % 1000000000UL);

    // Resume the wait if a signal cuts it short, so a paced frame is not
    // silently shortened. nanosleep writes the remaining time back into
    // request, so this picks up where it left off.
    while(nanosleep(&request, &request) == -1 && errno == EINTR){
        ;
    }
}
