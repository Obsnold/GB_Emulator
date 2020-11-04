#ifndef __SYSTEM_INTERFACE_H
#define __SYSTEM_INTERFACE_H

#include "gb_common.h"

#define KEY_UP      BIT_0
#define KEY_DOWN    BIT_1
#define KEY_LEFT    BIT_2
#define KEY_RIGHT   BIT_3
#define KEY_START   BIT_4
#define KEY_SELECT  BIT_5
#define KEY_A       BIT_6
#define KEY_B       BIT_7


void init_screen();
void free_screen();
void update_screen(uint8_t line);
int get_key_press();

#endif