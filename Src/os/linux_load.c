#include "gb_cart.h"
#include "gb_mem_map.h"
#include "debug_print.h"
#include <string.h>
#include <stdio.h> 
#include <stdlib.h>

#ifdef DEBUG
#define DEBUG_PRINT(fmt, args...)    PRINT(fmt, ## args)
#else
#define DEBUG_PRINT(fmt, args...)
#endif


int load_raw_file(char* filename, uint8_t** data_out){
	size_t size = 0;
	FILE *f = fopen(filename, "rb");
	if (f == NULL) 
	{ 
		return -1;
	} 
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, 0, SEEK_SET);
	*data_out = (uint8_t *)malloc(size);
	if (size != fread(*data_out, sizeof(char), size, f)) 
	{ 
		fclose(f);
		if(data_out!=NULL){
			free(*data_out);
		}
		return -2;
	}
	fclose(f);
	return size;
}

int save_raw_file(char* filename, uint8_t* data_in){
	return 0;
}

