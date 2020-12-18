#ifndef __GB_PPU_H
#define __GB_PPU_H

#include <stdint.h>
#include "gb_common.h"

#define GB_SCREEN_WIDTH 160
#define GB_SCREEN_HEIGHT 144
#define GB_SCREEN_HEIGHT_V_BLANK 9

#define green_1 0x9BBC0F
#define green_2 0x8BAC0F
#define green_3 0x306230
#define green_4 0x0F380F

// greens in rgb565
#define trans   0x0000
/*#define green_1 0x09C1
#define green_2 0x3306
#define green_3 0x8D61
#define green_4 0x9DE1*/

extern uint32_t gb_display[GB_SCREEN_WIDTH][GB_SCREEN_HEIGHT];

uint8_t ppu();

/*
uint16_t display_1[160*144];
uint16_t display_2[160*144];

#define green_1 0x09C1
#define green_2 0x3306
#define green_3 0x8D61
#define green_4 0x9DE1
*/


#endif