#include "gb_common.h"

uint16_t get_16_from_8(uint8_t* source){
    uint16_t temp = *source;
    return (temp << 8) + (*(source+1));
}

void set_16_from_8(uint8_t* dest, uint16_t source){
    *dest = source >> 8;
    *(dest+1) = source & 0x0F;
}

uint8_t get_16_low(uint16_t* source){
    return  (*source & 0x0F);
}

uint8_t get_16_high(uint16_t* source){
    return (*source >> 8);
}