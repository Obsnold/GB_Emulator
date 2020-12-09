#include "gb_opcodes.h"
#include "gb_mem_map.h"
#include "gb_common.h"
#include "gb_cpu.h"


//******************************************************
// rotate commands
//******************************************************
/*
rlca           07           4 000c rotate akku left
*/
uint8_t opcode_rlca(uint16_t opcode_address){
    uint8_t opcode = gb_mem_map[opcode_address];
    CPU_REG.A = (CPU_REG.A << 1) | (CPU_REG.A >> 7);
    if (CPU_REG.A & 0x01){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    CLR_ZERO_FLAG;
    CLR_ADD_SUB_FLAG;
    CLR_HALF_CARRY_FLAG;

    return opcode_table[opcode].cycles;
}

/*
rla            17           4 000c rotate akku left through carry
*/
uint8_t opcode_rla(uint16_t opcode_address){
    uint8_t opcode = gb_mem_map[opcode_address];
    uint8_t temp = GET_CARRY_FLAG;
    if(CPU_REG.A & 0x80){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    CPU_REG.A = (CPU_REG.A << 1) | temp;
    CLR_ZERO_FLAG;
    CLR_ADD_SUB_FLAG;
    CLR_HALF_CARRY_FLAG;
    return opcode_table[opcode].cycles;
}

/*
rrca           0F           4 000c rotate akku right
*/
uint8_t opcode_rrca(uint16_t opcode_address){
    uint8_t opcode = gb_mem_map[opcode_address];
    CPU_REG.A = (CPU_REG.A >> 0x01) | (CPU_REG.A << 0x07);
    if (CPU_REG.A & 0x80){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    CLR_ZERO_FLAG;
    CLR_ADD_SUB_FLAG;
    CLR_HALF_CARRY_FLAG;
    return opcode_table[opcode].cycles;
}

/*
rra            1F           4 000c rotate akku right through carry
*/
uint8_t opcode_rra(uint16_t opcode_address){
    uint8_t opcode = gb_mem_map[opcode_address];
    uint8_t temp = GET_CARRY_FLAG;
    if(CPU_REG.A & 0x01){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    CPU_REG.A = (CPU_REG.A >> 0x01) | (temp << 0x07);
    CLR_ZERO_FLAG;
    CLR_ADD_SUB_FLAG;
    CLR_HALF_CARRY_FLAG;
    return opcode_table[opcode].cycles;
}

/*
rlc  r         CB 0x        8 z00c rotate left
rlc  (HL)      CB 06       16 z00c rotate left
*/
void opcode_rlc(uint8_t* dest){
    *dest = (*dest << 1) | (*dest >> 7);
    if (*dest & 0x01){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    check_zero_flag(*dest);
    CLR_ADD_SUB_FLAG;
    CLR_HALF_CARRY_FLAG;
}

/*
rl   r         CB 1x        8 z00c rotate left through carry
rl   (HL)      CB 16       16 z00c rotate left through carry
*/
void opcode_rl(uint8_t* dest){
    uint8_t temp = GET_CARRY_FLAG;
    if(*dest & 0x80){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    *dest = (*dest << 1) | temp;
    check_zero_flag(*dest);
    CLR_ADD_SUB_FLAG;
    CLR_HALF_CARRY_FLAG;
}

/*
rrc  r         CB 0x        8 z00c rotate right
rrc  (HL)      CB 0E       16 z00c rotate right
*/
void opcode_rrc(uint8_t* dest){
    *dest = (*dest >> 0x01) | (*dest << 0x07);
    if (*dest & 0x80){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    check_zero_flag(*dest);
    CLR_ADD_SUB_FLAG;
    CLR_HALF_CARRY_FLAG;
}

/*
rr   r         CB 1x        8 z00c rotate right through carry
rr   (HL)      CB 1E       16 z00c rotate right through carry
*/
void opcode_rr(uint8_t* dest){
    uint8_t temp = GET_CARRY_FLAG;
    if(*dest & 0x01){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    *dest = (*dest >> 0x01) | (temp << 0x07);
    check_zero_flag(*dest);
    CLR_ADD_SUB_FLAG;
    CLR_HALF_CARRY_FLAG;
}

/*
sla  r         CB 2x        8 z00c shift left arithmetic (b0=0)
sla  (HL)      CB 26       16 z00c shift left arithmetic (b0=0)
*/
void opcode_sla(uint8_t* dest){
    if(*dest & 0x80){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    *dest = (*dest << 1);
    check_zero_flag(*dest);
    CLR_ADD_SUB_FLAG;
    CLR_HALF_CARRY_FLAG;

}

/*
swap r         CB 3x        8 z000 exchange low/hi-nibble
swap (HL)      CB 36       16 z000 exchange low/hi-nibble
*/
void opcode_swap(uint8_t* dest){
    *dest = (*dest << 0x04) & (*dest >> 0x04);
    check_zero_flag(*dest);
    CLR_ADD_SUB_FLAG;
    CLR_HALF_CARRY_FLAG;
    CLR_CARRY_FLAG;
}

/*
sra  r         CB 2x        8 z00c shift right arithmetic (b7=b7)
sra  (HL)      CB 2E       16 z00c shift right arithmetic (b7=b7)
*/
void opcode_sra(uint8_t* dest){
    uint8_t temp = *dest & 0x08;
    if(*dest & 0x01){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    *dest = (*dest >> 1) & temp;
    check_zero_flag(*dest);
    CLR_ADD_SUB_FLAG;
    CLR_HALF_CARRY_FLAG;
}

/*
srl  r         CB 3x        8 z00c shift right logical (b7=0)
srl  (HL)      CB 3E       16 z00c shift right logical (b7=0)
*/
void opcode_srl(uint8_t* dest){
    if(*dest & 0x01){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    *dest = (*dest >> 1);
    check_zero_flag(*dest);
    CLR_ADD_SUB_FLAG;
    CLR_HALF_CARRY_FLAG;
}



//******************************************************
// Single bit opperations
//******************************************************
/*
bit  n,r       CB xx       8 z01- test bit n
bit  n,(HL)    CB xx       12 z01- test bit n
*/
void opcode_bit(uint8_t* dest, uint8_t bit){
    CLR_ADD_SUB_FLAG;
    SET_HALF_CARRY_FLAG;
    if(*dest & (0x01 << bit)){
        SET_ZERO_FLAG;
    } else {
        CLR_ZERO_FLAG;
    }
}

/*
set  n,r       CB xx        8 ---- set bit n
set  n,(HL)    CB xx       16 ---- set bit n
*/
void opcode_set(uint8_t* dest, uint8_t bit){
    *dest = *dest | (0x01 << bit);
}

/*
res  n,r       CB xx        8 ---- reset bit n
res  n,(HL)    CB xx       16 ---- reset bit n
*/
void opcode_res(uint8_t* dest, uint8_t bit){
    *dest = *dest & ~(0x01 << bit);
}


uint8_t opcode_cb_prefix(uint16_t opcode_address){
    uint8_t* addr = NULL;
    uint8_t time = CYCLE_2;
    uint8_t opcode = gb_mem_map[opcode_address+1];
    uint8_t opcode_x = GET_OPCODE_X(opcode);
    uint8_t opcode_y = GET_OPCODE_Y(opcode);
    uint8_t opcode_z = GET_OPCODE_Z(opcode);
    
    addr = get_reg_8(opcode_z);

    if(opcode_z == 0x06){
        time = CYCLE_1;
    }

    switch(opcode_x){
        case 0x00: // rotate commands
            switch(opcode_y){
                case 0x00: // RLC
                    opcode_rlc(addr);
                    break;
                case 0x01: // RRC
                    opcode_rrc(addr);
                    break;
                case 0x02: // RL
                    opcode_rl(addr);
                    break;
                case 0x03: // RR
                    opcode_rr(addr);
                    break;
                case 0x04: // SLA
                    opcode_sla(addr);
                    break;
                case 0x05: // SRA
                    opcode_sra(addr);
                    break;
                case 0x06: // SWAP
                    opcode_swap(addr);
                    break;
                case 0x07: // SRL
                    opcode_srl(addr);
                    break;
            }
            break;
        case 0x01: // bit commands
            opcode_bit(addr,opcode_y);
            break;
        case 0x02: // reset commands
            opcode_res(addr,opcode_y);
            break;
        case 0x03: // set commands
            opcode_set(addr,opcode_y);
            break;
    }

    return time;
}