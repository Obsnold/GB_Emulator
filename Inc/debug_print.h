#ifndef __DEBUG_PRINT_H
#define __DEBUG_PRINT_H

#include "gb_common.h"

void print_cpu_reg();
void print_cart_header();
void print_opcode_table();
void print_opcode();
void print_memory(uint16_t start_pos, uint16_t end_pos);
void print_lcd();
void print_display_buffer();

#ifdef DEBUG
#define DEBUG_PRINT(fmt, args...)    printf(fmt, ## args)
#else
#define DEBUG_PRINT(fmt, args...)    /* Don't do anything in release builds */
#endif

#define PRINT(fmt, args...)    printf(fmt, ## args)

#endif //__DEBUG_PRINT_H