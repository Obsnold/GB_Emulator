#ifndef __GB_CPU_H
#define __GB_CPU_H

enum cpu_power{
    PWR_NORMAL,
    PWR_HALT,
    PWR_STOP
};

void cpu_set_power_mode(enum cpu_power mode );
void gb_cpu();

//CPU Registers
#define GB_REG_B	0x00
#define GB_REG_C	0x01
#define GB_REG_BC	0x00
#define GB_REG_D	0x02
#define GB_REG_E	0x03
#define GB_REG_DE	0x02
#define GB_REG_H	0x04
#define GB_REG_L	0x05
#define GB_REG_HL	0x04
#define GB_REG_A	0x06
#define GB_REG_F	0x07
#define GB_REG_AF	0x06

#define GB_REG_SP   0x08
#define GB_REG_SP_1 0x08
#define GB_REG_SP_2 0x09
#define GB_REG_PC   0x0A
#define GB_REG_PC_1 0x0A
#define GB_REG_PC_2 0x0B

#define GB_REG_SIZE 0x0C

//flags
#define ZERO_FLAG       BIT_7
#define ADD_SUB_FLAG    BIT_6
#define HALF_CARRY_FLAG BIT_5
#define CARRY_FLAG      BIT_4


extern uint8_t gb_cpu_reg[GB_REG_SIZE];

extern uint8_t gb_cpu_reg_sp;
extern uint8_t gb_cpu_reg_pc;

#define GET_ZERO_FLAG		((gb_cpu_reg[GB_REG_F] & ZERO_FLAG) >> 0x07)
#define GET_ADD_SUB_FLAG	((gb_cpu_reg[GB_REG_F] & ADD_SUB_FLAG) >> 0x06)
#define GET_HALF_CARRY_FLAG	((gb_cpu_reg[GB_REG_F] & HALF_CARRY_FLAG) >> 0x05)
#define GET_CARRY_FLAG		((gb_cpu_reg[GB_REG_F] & CARRY_FLAG) >> 0x04)

#define SET_ZERO_FLAG		gb_cpu_reg[GB_REG_F] |= ZERO_FLAG
#define CLR_ZERO_FLAG		gb_cpu_reg[GB_REG_F] &= ~ZERO_FLAG
#define SET_ADD_SUB_FLAG	gb_cpu_reg[GB_REG_F] |= ADD_SUB_FLAG
#define CLR_ADD_SUB_FLAG	gb_cpu_reg[GB_REG_F] &= ~ADD_SUB_FLAG
#define SET_HALF_CARRY_FLAG	gb_cpu_reg[GB_REG_F] |= HALF_CARRY_FLAG
#define CLR_HALF_CARRY_FLAG	gb_cpu_reg[GB_REG_F] &= ~HALF_CARRY_FLAG
#define SET_CARRY_FLAG		gb_cpu_reg[GB_REG_F] |= CARRY_FLAG
#define CLR_CARRY_FLAG		gb_cpu_reg[GB_REG_F] &= ~CARRY_FLAG

#define GET_REG_F(BIT) (gb_cpu_reg[GB_REG_F] & BIT)
#define SET_REG_F(BIT) (gb_cpu_reg[GB_REG_F] |= BIT)
#define CLR_REG_F(BIT) (gb_cpu_reg[GB_REG_F] &= ~BIT)

#endif