#ifndef __DEBUG_PRINT_H
#define __DEBUG_PRINT_H

#include "gb_common.h"

void print_cpu_reg();
void print_indirect_reg();
void print_cart_header();
void print_opcode_table();
void print_opcode();
void print_memory(uint16_t start_pos, uint16_t end_pos);
void print_lcd();
void print_display_buffer();
void print_step();
void print_interrupts();
#define PRINT(fmt, args...)    printf(fmt, ## args)

#endif //__DEBUG_PRINT_H