#ifndef __SYSTEM_INTERFACE_H
#define __SYSTEM_INTERFACE_H

#include "gb_common.h"

uint16_t get_key_press();
int init_screen();
void free_screen();
int update_screen(uint8_t line);
void enable_lcd(bool set);
void debug_screen_init();
void debug_screen_free();
void debug_screen();
void debug_screen_select(uint8_t key);
void debug_draw_tile_ram();

#define KEY_DOWN    BIT_3
#define KEY_UP      BIT_2
#define KEY_LEFT    BIT_1
#define KEY_RIGHT   BIT_0

#define KEY_START   BIT_7
#define KEY_SELECT  BIT_6
#define KEY_B       BIT_5
#define KEY_A       BIT_4

#define KEY_DEBUG   0xFFFF
#define KEY_EXIT    0xFFFE

#endif