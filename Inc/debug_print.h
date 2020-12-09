#ifndef __DEBUG_PRINT_H
#define __DEBUG_PRINT_H

#include "gb_common.h"

void print_cpu_reg();
void print_cart_header();
void print_opcode_table();
void print_opcode();
void print_memory(uint16_t start_pos, uint16_t end_pos);

#endif