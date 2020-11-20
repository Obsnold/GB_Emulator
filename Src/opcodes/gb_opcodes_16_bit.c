#include "gb_opcodes.h"
#include "gb_mem_map.h"
#include "gb_common.h"
#include "gb_cpu.h"


//******************************************************
//16-bit commands
//******************************************************
/*
ld   rr,nn       x1 nn nn  12 ---- rr=nn (rr may be BC,DE,HL or SP)
ld   SP,HL       F9         8 ---- SP=HL
*/
uint8_t opcode_16_ld(uint16_t opcode_address){
    uint8_t opcode = gb_mem_map[opcode_address];
    uint8_t* dest = NULL;
    uint8_t* source = NULL;

    switch(opcode){
        case 0xF9:
            dest = &gb_cpu_reg[GB_REG_SP];
            source = &gb_cpu_reg[GB_REG_HL];
            break;
        default:
            dest = get_reg_16_sp(GET_OPCODE_P(opcode));
            source = &gb_mem_map[opcode_address+1];
            break;
    }

    *dest = *source;
    *(dest+1) = *(source+1);
    return opcode_table[opcode].cycles;
}

/*
ld   HL,SP+dd  F8 dd         12 00hc HL = SP +/- dd ;dd is 8bit signed number
*/
uint8_t opcode_16_ld_offset(uint16_t opcode_address){
    uint8_t opcode = gb_mem_map[opcode_address];

    uint16_t source = get_reg_16_value(GB_REG_SP);
    source += (int8_t) gb_mem_map[opcode_address+1];

    gb_cpu_reg[GB_REG_H] = get_16_high(&source);
    gb_cpu_reg[GB_REG_L] = get_16_low(&source);
    return opcode_table[opcode].cycles;
}

/*
push rr          x5        16 ---- SP=SP-2  (SP)=rr   (rr may be BC,DE,HL,AF)
*/
uint8_t opcode_16_push(uint16_t opcode_address){
    uint8_t opcode = gb_mem_map[opcode_address];
    uint16_t temp = get_16_from_8(&gb_cpu_reg[GB_REG_SP]);
    temp -=2;
    gb_cpu_reg[GB_REG_SP_1] = get_16_high(&temp);
    gb_cpu_reg[GB_REG_SP_2] = get_16_low(&temp);
    gb_mem_map[temp] = get_16_from_8(get_reg_16_af(GET_OPCODE_P(opcode)));
    return opcode_table[opcode].cycles;
}
/*
pop  rr          x1        12 (AF) rr=(SP)  SP=SP+2   (rr may be BC,DE,HL,AF)
*/
uint8_t opcode_16_pop(uint16_t opcode_address){
    uint8_t opcode = gb_mem_map[opcode_address];
    uint16_t temp = get_16_from_8(&gb_cpu_reg[GB_REG_SP]);
    uint8_t* dest = get_reg_16_af(GET_OPCODE_P(opcode));
    *dest = gb_mem_map[temp];
    *(dest+1) = gb_mem_map[temp+1];
    temp +=2;
    gb_cpu_reg[GB_REG_SP_1] = get_16_high(&temp);
    gb_cpu_reg[GB_REG_SP_2] = get_16_low(&temp);
    return opcode_table[opcode].cycles;
}
/*
add  HL,rr     x9           8 -0hc HL = HL+rr     ;rr may be BC,DE,HL,SP
*/
uint8_t opcode_16_add_hl(uint16_t opcode_address){
    uint8_t opcode = gb_mem_map[opcode_address];
    uint8_t *source = get_reg_16_sp(GET_OPCODE_P(opcode));
    //first part
    gb_cpu_reg[GB_REG_A] = gb_cpu_reg[GB_REG_L];
    if (check_8_overflow(source+1)){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    if(check_4_overflow(source+1)){
        SET_HALF_CARRY_FLAG;
    } else {
        CLR_HALF_CARRY_FLAG;
    }

    gb_cpu_reg[GB_REG_A] += *(source+1);
    gb_cpu_reg[GB_REG_L] = gb_cpu_reg[GB_REG_A];

    //second part
    gb_cpu_reg[GB_REG_A] = gb_cpu_reg[GB_REG_H];
    if (check_8_overflow(source)){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    if(check_4_overflow(source)){
        SET_HALF_CARRY_FLAG;
    } else {
        CLR_HALF_CARRY_FLAG;
    }
    gb_cpu_reg[GB_REG_A] += *source;
    gb_cpu_reg[GB_REG_A] += GET_CARRY_FLAG;
    gb_cpu_reg[GB_REG_H] = gb_cpu_reg[GB_REG_A];

    CLR_ADD_SUB_FLAG;
    return opcode_table[opcode].cycles;
}

/*
add  SP,dd     E8 dd         16 00hc SP = SP +/- dd ;dd is 8bit signed number
*/
uint8_t opcode_16_add_sp(uint16_t opcode_address){
    uint8_t opcode = gb_mem_map[opcode_address];
    uint16_t temp = get_16_from_8(&gb_cpu_reg[GB_REG_SP]);
    temp += (int8_t)gb_mem_map[opcode_address+1];
    gb_cpu_reg[GB_REG_SP_1] = get_16_high(&temp);
    gb_cpu_reg[GB_REG_SP_2] = get_16_low(&temp);
    CLR_ZERO_FLAG;
    CLR_ADD_SUB_FLAG;
    return opcode_table[opcode].cycles;
}

/*
inc  rr        x3           8 ---- rr = rr+1      ;rr may be BC,DE,HL,SP
*/
uint8_t opcode_16_inc(uint16_t opcode_address){
    uint8_t opcode = gb_mem_map[opcode_address];
    uint8_t* dest =  get_reg_16_af(GET_OPCODE_P(opcode));
    uint16_t temp = get_16_from_8(dest);
    temp++;
    *dest = get_16_high(&temp);
    *(dest+1) = get_16_low(&temp);
    return opcode_table[opcode].cycles;
}

/*
dec  rr        xB           8 ---- rr = rr-1      ;rr may be BC,DE,HL,SP
*/
uint8_t opcode_16_dec(uint16_t opcode_address){
    uint8_t opcode = gb_mem_map[opcode_address];
    uint8_t* dest =  get_reg_16_af(GET_OPCODE_P(opcode));
    uint16_t temp = get_16_from_8(dest);
    temp--;
    *dest = get_16_high(&temp);
    *(dest+1) = get_16_low(&temp);
    return opcode_table[opcode].cycles;
}
