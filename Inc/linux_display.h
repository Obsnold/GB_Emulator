#ifndef __LINUX_DISPLAY_H
#define __LINUX_DISPLAY_H
void init_screen();
void free_screen();
void update_screen(uint8_t line);
uint32_t get_key_press();
#endif