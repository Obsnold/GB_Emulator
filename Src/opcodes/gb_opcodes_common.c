#include "gb_opcodes.h"


int check_16_overflow(uint16_t* a, uint16_t* b){
    return (*a > 0xFFFF - *b);
}

int check_16_underflow(uint16_t* a, uint16_t* b){
    return (*a < *b);
}

int check_8_overflow(uint8_t* b){
    return (CPU_REG.A > 0xFF - *b);
}

int check_8_underflow(uint8_t* b){
    return (CPU_REG.A < *b);
}

int check_4_overflow(uint8_t* b){
    return (((CPU_REG.A & 0x0F) + (*b & 0x0F)) & 0x10) == 0x10;
}

int check_4_underflow(uint8_t* b){
    return ((CPU_REG.A & 0xF0) < (*b & 0xF0));
}

void check_zero_flag(){
    if (CPU_REG.A == 0x00){
        SET_ZERO_FLAG;
    } else {
        CLR_ZERO_FLAG;
    }
}


// gb_cpu_reg functions
uint8_t* get_reg_8(uint8_t reg){
    uint8_t* ret = NULL;
    switch(reg){
        case 0:
            ret = &CPU_REG.B;
            break;
        case 1:
            ret = &CPU_REG.C;
            break;
        case 2:
            ret = &CPU_REG.D;
            break;
        case 3:
            ret = &CPU_REG.E;
            break;
        case 4:
            ret = &CPU_REG.H;
            break;
        case 5:
            ret = &CPU_REG.L;
            break;
        case 6:
            ret = &gb_mem_map[CPU_REG.HL];
            break;
        case 7:
            ret = &CPU_REG.A;
            break;
    }
    return ret;
}

uint16_t* get_reg_16_sp(uint8_t reg){
    uint16_t* ret = 0;
    switch(reg){
        case 0:
            ret = &CPU_REG.BC;
            break;
        case 1:
            ret = &CPU_REG.DE;
            break;
        case 2:
            ret = &CPU_REG.HL;
            break;
        case 3:
            ret = &CPU_REG.SP;
            break;
    }
    return ret;
}

uint16_t* get_reg_16_af(uint8_t reg){
    uint16_t* ret = 0;
    switch(reg){
        case 0:
            ret = &CPU_REG.BC;
            break;
        case 1:
            ret = &CPU_REG.DE;
            break;
        case 2:
            ret = &CPU_REG.HL;
            break;
        case 3:
            ret = &CPU_REG.AF;
            break;
    }
    return ret;
}

bool get_flag_condition(uint8_t flag){
    bool ret = false;
    switch(flag){
        case 0: // NZ
            if(GET_ZERO_FLAG == 0){
                ret = true;
            }
            break;
        case 1: // Z
            if(GET_ZERO_FLAG == 1){
                ret = true;
            }
            break;
        case 2: // NC
            if(GET_CARRY_FLAG == 0){
                ret = true;
            }
            break;
        case 3: // C
            if(GET_CARRY_FLAG == 1){
                ret = true;
            }
            break;
    }
    return ret;
}
