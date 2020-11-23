#ifndef __GB_CPU_H
#define __GB_CPU_H

enum cpu_power{
    PWR_NORMAL,
    PWR_HALT,
    PWR_STOP
};

void cpu_set_power_mode(enum cpu_power mode );
void gb_cpu();



typedef struct cpu_reg {
    union {
        struct {
            uint8_t C;
            uint8_t B;
        };
        uint16_t BC;
    };
    union {
        struct {
            uint8_t E;
            uint8_t D;
        };
        uint16_t DE;
    };
    union {
        struct {
            uint8_t L;
            uint8_t H;
        };
        uint16_t HL;
    };
    union {
        struct {
            uint8_t F;
            uint8_t A;
        };
        uint16_t AF;
    };
    union {
        struct {
            uint8_t PC_1;
            uint8_t PC_2;
        };
        uint16_t PC;
    };
    union {
        struct {
            uint8_t SP_1;
            uint8_t SP_2;
        };
        uint16_t SP;
    };
}gb_cpu_reg;

extern gb_cpu_reg CPU_REG;

//flags
#define ZERO_FLAG       BIT_7
#define ADD_SUB_FLAG    BIT_6
#define HALF_CARRY_FLAG BIT_5
#define CARRY_FLAG      BIT_4

#define GET_ZERO_FLAG		((CPU_REG.F & ZERO_FLAG) >> 0x07)
#define GET_ADD_SUB_FLAG	((CPU_REG.F & ADD_SUB_FLAG) >> 0x06)
#define GET_HALF_CARRY_FLAG	((CPU_REG.F & HALF_CARRY_FLAG) >> 0x05)
#define GET_CARRY_FLAG		((CPU_REG.F & CARRY_FLAG) >> 0x04)

#define SET_ZERO_FLAG		CPU_REG.F |= ZERO_FLAG
#define CLR_ZERO_FLAG		CPU_REG.F &= ~ZERO_FLAG
#define SET_ADD_SUB_FLAG	CPU_REG.F |= ADD_SUB_FLAG
#define CLR_ADD_SUB_FLAG	CPU_REG.F &= ~ADD_SUB_FLAG
#define SET_HALF_CARRY_FLAG	CPU_REG.F |= HALF_CARRY_FLAG
#define CLR_HALF_CARRY_FLAG	CPU_REG.F &= ~HALF_CARRY_FLAG
#define SET_CARRY_FLAG		CPU_REG.F |= CARRY_FLAG
#define CLR_CARRY_FLAG		CPU_REG.F &= ~CARRY_FLAG

#define GET_REG_F(BIT) (CPU_REG.F & BIT)
#define SET_REG_F(BIT) (CPU_REG.F |= BIT)
#define CLR_REG_F(BIT) (CPU_REG.F &= ~BIT)

#endif