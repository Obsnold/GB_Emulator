#ifndef __GB_COMMON_H
#define __GB_COMMON_H


#include <stdint.h>

//BIT_DEFINES
#define BIT_0	0x01
#define BIT_1	0x02
#define BIT_2	0x04
#define BIT_3	0x08
#define BIT_4	0x10
#define BIT_5	0x20
#define BIT_6	0x40
#define BIT_7	0x80

#define BIT_SET 0x01
#define BIT_CLEAR 0x00

uint16_t get_16_from_8(uint8_t* source);
void set_16_from_8(uint8_t* dest, uint16_t source);
uint8_t get_16_low(uint16_t* source);
uint8_t get_16_high(uint16_t* source);
unsigned long get_ticks();

#endif