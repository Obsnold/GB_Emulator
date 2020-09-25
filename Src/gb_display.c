#include "gb_display.h"
#include <stdint.h>


/* Pictures position */
uint32_t Xpos1 = 0;
uint32_t Xpos2 = 0;
uint32_t Ypos1 = 0;
uint32_t Ypos2 = 0;

uint16_t display_1[160*144];
uint16_t display_2[160*144];

#define green_1 0x09C1
#define green_2 0x3306
#define green_3 0x8D61
#define green_4 0x9DE1


void update_screen(){
    
}