#ifndef __GB_LOAD_H
#define __GB_LOAD_H

#include <stdint.h>

int load_raw_file(char* filename, uint8_t** data_out);
int save_raw_file(char* filename, uint8_t* data_in);

#endif