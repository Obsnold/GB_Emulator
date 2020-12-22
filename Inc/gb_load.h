#ifndef __GB_LOAD_H
#define __GB_LOAD_H

#include <stdint.h>


int load_gb_cart(char* filename);
void load_initial_membanks();
uint8_t* get_cart();
void load_membank(unsigned int bank);

#endif