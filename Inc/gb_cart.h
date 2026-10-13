#ifndef __GB_CART_H
#define __GB_CART_H
#include "gb_common.h"

int gb_cart_load(char* filename);

/* Map a cart from memory the caller owns and keeps alive. gb_cart_load is this
 * plus reading the file; a host that already has the image - a browser handing
 * over a file the user picked - calls this directly. */
int gb_cart_load_buffer(uint8_t* data, unsigned int size);
void gb_cart_write_rom(uint16_t addr, uint8_t data);
uint8_t gb_cart_read_rom(uint16_t addr);
void gb_cart_write_ram(uint16_t addr, uint8_t data);
uint8_t gb_cart_read_ram(uint16_t addr);


#endif