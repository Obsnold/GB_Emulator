#ifndef __GB_OPCODES_H
#define __GB_OPCODES_H
#include <stdint.h>
#include <stdio.h> 
#include "gb_cpu.h"
#include "gb_common.h"
#include "gb_mem_map.h"
#define OPCODE_TABLE_SIZE 0x100

typedef uint8_t (*opcodeFunction)(uint16_t opcode_address);

struct gb_opcode{
    char* description;
    uint8_t length;
    uint8_t cycles;
    opcodeFunction operation;
    /*uint8_t* operand_1;
    uint8_t* operand_2;
    uint8_t* operand_3;*/
};

extern struct gb_opcode opcode_table[OPCODE_TABLE_SIZE];

uint8_t opcode_8_ld(uint16_t opcode_address);
uint8_t opcode_8_ldi(uint16_t opcode_address);
uint8_t opcode_8_ldd(uint16_t opcode_address);
uint8_t opcode_8_add(uint16_t opcode_address);
uint8_t opcode_8_adc(uint16_t opcode_address);
uint8_t opcode_8_sub(uint16_t opcode_address);
uint8_t opcode_8_sbc(uint16_t opcode_address);
uint8_t opcode_8_and(uint16_t opcode_address);
uint8_t opcode_8_xor(uint16_t opcode_address);
uint8_t opcode_8_or(uint16_t opcode_address);
uint8_t opcode_8_cp(uint16_t opcode_address);
uint8_t opcode_8_inc(uint16_t opcode_address);
uint8_t opcode_8_dec(uint16_t opcode_address);
uint8_t opcode_8_daa(uint16_t opcode_address);
uint8_t opcode_8_cpl(uint16_t opcode_address);

uint8_t opcode_16_ld(uint16_t opcode_address);
uint8_t opcode_16_ld_offset(uint16_t opcode_address);
uint8_t opcode_16_push(uint16_t opcode_address);
uint8_t opcode_16_pop(uint16_t opcode_address);
uint8_t opcode_16_add_hl(uint16_t opcode_address);
uint8_t opcode_16_add_sp(uint16_t opcode_address);
uint8_t opcode_16_inc(uint16_t opcode_address);
uint8_t opcode_16_dec(uint16_t opcode_address);

uint8_t opcode_rrca(uint16_t opcode_address);
uint8_t opcode_rra(uint16_t opcode_address);
uint8_t opcode_rlca(uint16_t opcode_address);
uint8_t opcode_rla(uint16_t opcode_address);
uint8_t opcode_cb_prefix(uint16_t opcode_address);

uint8_t opcode_ccf(uint16_t opcode_address);
uint8_t opcode_scf(uint16_t opcode_address);
uint8_t opcode_nop(uint16_t opcode_address);
uint8_t opcode_halt(uint16_t opcode_address);
uint8_t opcode_stop(uint16_t opcode_address);
uint8_t opcode_di(uint16_t opcode_address);
uint8_t opcode_ei(uint16_t opcode_address);


// jump and call opcodes
uint8_t opcode_jp(uint16_t opcode_address);
uint8_t opcode_jp_hl(uint16_t opcode_address);
uint8_t opcode_jp_cnd(uint16_t opcode_address);
uint8_t opcode_jr(uint16_t opcode_address);
uint8_t opcode_jr_cnd(uint16_t opcode_address);
uint8_t opcode_call(uint16_t opcode_address);
uint8_t opcode_call_cnd(uint16_t opcode_address);
uint8_t opcode_ret(uint16_t opcode_address);
uint8_t opcode_ret_cnd(uint16_t opcode_address);
uint8_t opcode_reti(uint16_t opcode_address);
uint8_t opcode_rst(uint16_t opcode_address);
uint8_t opcode_NA(uint16_t opcode_address);


// common functions for the opcodes
int check_16_overflow(uint16_t a, uint16_t b);
int check_16_underflow(uint16_t a, uint16_t b);
int check_8_overflow(uint16_t b);
int check_8_underflow(uint16_t b);
int check_4_overflow(uint8_t b,uint8_t carry);
int check_4_underflow(uint8_t b,uint8_t carry);
void check_zero_flag(uint8_t val);

uint8_t* get_reg_8(uint8_t reg);
uint16_t* get_reg_16_sp(uint8_t reg);
uint16_t* get_reg_16_af(uint8_t reg);
bool get_flag_condition(uint8_t flag);


// opcode stuff
#define OP_X_MASK 0xC0
#define OP_Y_MASK 0x38
#define OP_Z_MASK 0x07
#define OP_P_MASK 0x30
#define OP_Q_MASK 0x08

#define OP_X_1  0x01
#define OP_X_2  0x10
#define OP_X_3  0x18
#define OP_X_4  0x20

#define OP_Y_1  0x08
#define OP_Y_2  0x10
#define OP_Y_3  0x18
#define OP_Y_4  0x20
#define OP_Y_5  0x28
#define OP_Y_6  0x30
#define OP_Y_7  0x38

#define OP_Z_0  0x00
#define OP_Z_1  0x01
#define OP_Z_2  0x02
#define OP_Z_3  0x03
#define OP_Z_4  0x04
#define OP_Z_5  0x05
#define OP_Z_6  0x06
#define OP_Z_7  0x07

#define OP_P_0  0x00
#define OP_P_1  0x10
#define OP_P_2  0x20
#define OP_P_3  0x30

#define OP_Q_0  0x00
#define OP_Q_1  0x08


#define GET_OPCODE_X(x) (((x) & OP_X_MASK) >> 6)
#define GET_OPCODE_Y(x) (((x) & OP_Y_MASK) >> 3)
#define GET_OPCODE_Z(x) ((x) & OP_Z_MASK)
#define GET_OPCODE_P(x) (((x) & OP_P_MASK) >> 4)
#define GET_OPCODE_Q(x) (((x) & OP_Q_MASK) >> 3)

//cycles
//some opcodes have vairable cycle lengths 
//so if a cycle has 2 possible cyle times it 
//will be encoded in the higher nibble of the number
#define CYCLE_MASK_1 0x0F
#define CYCLE_MASK_2 0xF0

#define NA_CYCLES 0x00
#define CYCLE_MASK 0x0F
#define CYCLE_1 0x01 //4 cycles
#define CYCLE_2 0x02 //8 cycles
#define CYCLE_3 0x03 //12 cycles
#define CYCLE_4 0x04 //16 cycles
#define CYCLE_5 0x05 //20 cycles
#define CYCLE_6 0x06 //24 cycles

#define ALT_CYCLE_MASK 0xF0
#define ALT_CYCLE_1 0x10 //4 cycles
#define ALT_CYCLE_2 0x20 //8 cycles
#define ALT_CYCLE_3 0x30 //12 cycles
#define ALT_CYCLE_4 0x40 //16 cycles
#define ALT_CYCLE_5 0x50 //20 cycles
#define ALT_CYCLE_6 0x60 //24 cycles


#endif