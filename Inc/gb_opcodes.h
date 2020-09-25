#ifndef __GB_OPCODES_H
#define __GB_OPCODES_H
#include <stdint.h>
#include <stdio.h> 

void opcode_8_ld(uint8_t* dest, uint8_t* source);
void opcode_8_ldi(uint8_t* dest, uint8_t* source);
void opcode_8_ldd(uint8_t* dest, uint8_t* source);
void opcode_8_add(uint8_t* source);
void opcode_8_adc(uint8_t* source);
void opcode_8_sub(uint8_t* source);
void opcode_8_sbc(uint8_t* source);
void opcode_8_and(uint8_t* source);
void opcode_8_xor(uint8_t* source);
void opcode_8_or(uint8_t* source);
void opcode_8_cp(uint8_t* source);
void opcode_8_inc(uint8_t* dest);
void opcode_8_dec(uint8_t* dest);
void opcode_8_daa();
void opcode_8_cpl();

void opcode_16_ld(uint8_t* dest, uint8_t* source);
void opcode_16_ld_offset(uint8_t* dest, uint8_t* source, int8_t offset);
void opcode_16_push(uint8_t* source);
void opcode_16_pop(uint8_t* dest);
void opcode_16_add_hl(uint8_t* source);
void opcode_16_add_sp(int8_t offset);
void opcode_16_inc(uint8_t* dest);
void opcode_16_dec(uint8_t* dest);

void opcode_rlca();
void opcode_rla();
void opcode_rrca();
void opcode_rra();
void opcode_rlc(uint8_t* dest);
void opcode_lc(uint8_t* dest);
void opcode_rl(uint8_t* dest);
void opcode_rrc(uint8_t* dest);
void opcode_rr(uint8_t* dest);
void opcode_sla(uint8_t* dest);
void opcode_swap(uint8_t* dest);
void opcode_sra(uint8_t* dest);
void opcode_srl(uint8_t* dest);

void opcode_bit(uint8_t* dest, uint8_t bit);
void opcode_set(uint8_t* dest, uint8_t bit);
void opcode_res(uint8_t* dest, uint8_t bit);
void opcode_ccf();
void opcode_scf();
void opcode_nop();
void opcode_halt();
void opcode_stop();
void opcode_di();
void opcode_ei();

void opcode_jp(uint16_t address);
void opcode_jp_hl();
void opcode_jp_cnd(uint16_t address, uint8_t condition);
void opcode_jr(int8_t offset);
void opcode_jr_cnd(int8_t offset,uint8_t condition);
void opcode_call(uint16_t address);
void opcode_call_cnd(uint16_t address, uint8_t condition);
void opcode_ret();
void opcode_ret_cnd(uint8_t condition);
void opcode_reti();
void opcode_rst(uint16_t address);
void opcode_CB_prefix();
void opcode_NA();

#endif