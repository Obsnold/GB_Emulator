#ifndef __GB_COMMON_H
#define __GB_COMMON_H


#include <stdint.h>
#include <stdbool.h>
//BIT_DEFINES
#define BIT_0	0x01
#define BIT_1	0x02
#define BIT_2	0x04
#define BIT_3	0x08
#define BIT_4	0x10
#define BIT_5	0x20
#define BIT_6	0x40
#define BIT_7	0x80
#define BIT_8	0x0100
#define BIT_9	0x0200
#define BIT_10	0x0400
#define BIT_11	0x0800
#define BIT_12	0x1000
#define BIT_13	0x2000
#define BIT_14	0x4000
#define BIT_15	0x8000

#define BIT_SET 0x01
#define BIT_CLEAR 0x00

uint16_t get_16_from_8(uint8_t* source);
void set_16_from_8(uint8_t* dest, uint16_t source);
uint8_t get_16_low(uint16_t* source);
uint8_t get_16_high(uint16_t* source);
unsigned long get_ns();
void sleep_ns(long ns);

#endif