#ifndef __GB_CART_H
#define __GB_CART_H
#include "gb_common.h"

int gb_cart_load(char* filename);
void gb_cart_write_rom(uint16_t addr, uint8_t data);
uint8_t gb_cart_read_rom(uint16_t addr);
void gb_cart_write_ram(uint16_t addr, uint8_t data);
uint8_t gb_cart_read_ram(uint16_t addr);


#endif