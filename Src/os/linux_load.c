#include "gb_load.h"
#include "gb_mem_map.h"
#include "debug_print.h"
#include <string.h>
#include <stdio.h> 
#include <stdlib.h>

uint8_t* gb_cart;
unsigned int gb_cart_size = 0;

int load_gb_cart(char* filename){
	FILE *f = fopen(filename, "rb");
	if (f == NULL) 
	{ 
		return -1;
	} 
	fseek(f, 0, SEEK_END);
	gb_cart_size = ftell(f);
	fseek(f, 0, SEEK_SET);
	gb_cart = (uint8_t *)malloc(gb_cart_size);
	if (gb_cart_size != fread(gb_cart, sizeof(char), gb_cart_size, f)) 
	{ 
		free(gb_cart);
		return -2;
	} 
	fclose(f);
	return gb_cart_size;
}


void load_initial_membanks(){
    memcpy(gb_mem_map,gb_cart,0x8000);
}

uint8_t* get_cart(){
	return gb_cart;
}

void load_membank(uint8_t bank){
    if((bank * CART_MB_SIZE) < gb_cart_size){
        memcpy(&gb_mem_map[CART_ROM_1],
                (gb_cart + (bank * CART_MB_SIZE)),
                CART_MB_SIZE);
    } else {
        DEBUG_PRINT("Error! trying to load membank from outside cart");
    }
}