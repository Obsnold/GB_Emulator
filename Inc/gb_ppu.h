#ifndef __GB_PPU_H
#define __GB_PPU_H

#include <stdint.h>
#include "gb_common.h"

#define GB_SCREEN_WIDTH 160
#define GB_SCREEN_HEIGHT 144
#define GB_SCREEN_HEIGHT_V_BLANK 9

#define PAL_TRANS   0x000000
#define PAL_WHITE   0x9BBC0F
#define PAL_L_GRAY  0x8BAC0F
#define PAL_D_GRAY  0x306230
#define PAL_BLACK   0x0F380F

// greens in rgb565
/*#define green_1 0x09C1
#define green_2 0x3306
#define green_3 0x8D61
#define green_4 0x9DE1*/

extern uint32_t gb_display[GB_SCREEN_WIDTH][GB_SCREEN_HEIGHT];

uint8_t ppu(unsigned long delta_time);

#endif