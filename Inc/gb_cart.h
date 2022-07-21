#ifndef __GB_CART_H
#define __GB_CART_H
#include "gb_common.h"

int gb_cart_load(char* filename);
void gb_cart_switch_rom_bank(unsigned int bank);
void gb_cart_switch_ram_bank(unsigned int bank);
void gb_cart_set_ram_enabled(bool set);
void gb_cart_set_banking_mode(uint8_t set);

#endif