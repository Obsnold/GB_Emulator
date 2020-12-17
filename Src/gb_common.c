#include "gb_common.h"
#include <time.h>
#include <math.h>

uint16_t get_16_from_8(uint8_t* source){
    uint16_t temp = *source;
    return temp + (*(source+1)<<8);
}

void set_16_from_8(uint8_t* dest, uint16_t source){
    *(dest+1) = source >> 8;
    *dest = source & 0x0F;
}

uint8_t get_16_low(uint16_t* source){
    return  (*source & 0xFF);
}

uint8_t get_16_high(uint16_t* source){
    return (*source >> 8);
}

// clock is 4MHz so one tick is 250ns
unsigned long get_ns(){
    struct timespec _t;
    clock_gettime(CLOCK_REALTIME, &_t);
    return ((_t.tv_sec*1000000000 + lround(_t.tv_nsec)));
}