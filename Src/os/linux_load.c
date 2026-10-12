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
	uint8_t *buffer = NULL;
	FILE *f = fopen(filename, "rb");
	if (f == NULL)
	{
		return -1;
	}
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, 0, SEEK_SET);

	// Checked before the read: fread into a null pointer is undefined, and a
	// failed ftell lands here as a huge size, which this catches too.
	buffer = (uint8_t *)malloc(size);
	if (buffer == NULL)
	{
		fclose(f);
		return -3;
	}

	if (size != fread(buffer, sizeof(char), size, f))
	{
		fclose(f);
		free(buffer);
		return -2;
	}

	fclose(f);
	// Only handed back on success, so a failed load cannot leave the caller
	// holding a freed pointer.
	*data_out = buffer;
	return size;
}
