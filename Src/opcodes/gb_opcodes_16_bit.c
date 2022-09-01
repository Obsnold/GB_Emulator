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
ld   (a16),SP    08          
*/
uint8_t opcode_16_ld(uint16_t opcode_address){
    uint8_t opcode = op_get_mem_map_8(opcode_address);
    uint16_t* dest = NULL;
    uint16_t value = 0;
    uint16_t addr = 0;

    switch(opcode){
        case 0xF9:
            dest = &CPU_REG.SP;
            value = CPU_REG.HL;
            *dest = value;
            break;
        case 0x08:
            addr = op_get_mem_map_16(opcode_address+1);
            op_set_mem_map_16(addr,CPU_REG.SP);
            break;
        default:
            dest = get_reg_16_sp(GET_OPCODE_P(opcode));
            value = op_get_mem_map_16(opcode_address+1);
            *dest = value;
            break;
    }

    
    return opcode_table[opcode].cycles;
}

/*
ld   HL,SP+dd  F8 dd         12 00hc HL = SP +/- dd ;dd is 8bit signed number
*/
uint8_t opcode_16_ld_offset(uint16_t opcode_address){
    uint8_t opcode = op_get_mem_map_8(opcode_address);
    uint8_t test_value =op_get_mem_map_8(opcode_address+1);
    int8_t value = (int8_t)test_value;
    

    if (check_8_overflow(CPU_REG.SP_1, test_value,0)){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    if(check_4_overflow(CPU_REG.SP_1, test_value,0)){
        SET_HALF_CARRY_FLAG;
    } else {
        CLR_HALF_CARRY_FLAG;
    }
    if (check_16_overflow(CPU_REG.SP, test_value,0)){
        SET_CARRY_FLAG;
    }
    if(check_12_overflow(CPU_REG.SP, test_value,0)){
        SET_HALF_CARRY_FLAG;
    }

    CPU_REG.HL = CPU_REG.SP + value;
    CLR_ZERO_FLAG;
    CLR_ADD_SUB_FLAG;
    return opcode_table[opcode].cycles;
}

/*
push rr          x5        16 ---- SP=SP-2  (SP)=rr   (rr may be BC,DE,HL,AF)
*/
uint8_t opcode_16_push(uint16_t opcode_address){
    uint8_t opcode = op_get_mem_map_8(opcode_address);
    CPU_REG.SP -=2;
    uint16_t* reg = get_reg_16_af(GET_OPCODE_P(opcode));
    op_set_mem_map_16(CPU_REG.SP, *reg);

    return opcode_table[opcode].cycles;
}
/*
pop  rr          x1        12 (AF) rr=(SP)  SP=SP+2   (rr may be BC,DE,HL,AF)
*/
uint8_t opcode_16_pop(uint16_t opcode_address){
    uint8_t opcode = op_get_mem_map_8(opcode_address);
    uint16_t* dest = get_reg_16_af(GET_OPCODE_P(opcode));
    *dest = op_get_mem_map_16(CPU_REG.SP);
    CPU_REG.SP +=2;
    return opcode_table[opcode].cycles;
}
/*
add  HL,rr     x9           8 -0hc HL = HL+rr     ;rr may be BC,DE,HL,SP
*/
uint8_t opcode_16_add_hl(uint16_t opcode_address){
    uint8_t opcode = op_get_mem_map_8(opcode_address);
    uint16_t *reg = get_reg_16_sp(GET_OPCODE_P(opcode));

    if (check_16_overflow(CPU_REG.HL, *reg,0)){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    if(check_12_overflow(CPU_REG.HL, *reg,0)){
        SET_HALF_CARRY_FLAG;
    } else {
        CLR_HALF_CARRY_FLAG;
    }

    CPU_REG.HL += *reg;

    CLR_ADD_SUB_FLAG;
    return opcode_table[opcode].cycles;
}

/*
add  SP,dd     E8 dd         16 00hc SP = SP +/- dd ;dd is 8bit signed number
*/
uint8_t opcode_16_add_sp(uint16_t opcode_address){
    uint8_t opcode = op_get_mem_map_8(opcode_address);
    uint8_t test_value =op_get_mem_map_8(opcode_address+1);
    int8_t value = (int8_t)test_value;
    
    if (check_8_overflow(CPU_REG.SP_1, test_value,0)){
        SET_CARRY_FLAG;
    } else {
        CLR_CARRY_FLAG;
    }
    if(check_4_overflow(CPU_REG.SP_1, test_value,0)){
        SET_HALF_CARRY_FLAG;
    } else {
        CLR_HALF_CARRY_FLAG;
    }
    if (check_16_overflow(CPU_REG.SP, test_value,0)){
        SET_CARRY_FLAG;
    }
    if(check_12_overflow(CPU_REG.SP, test_value,0)){
        SET_HALF_CARRY_FLAG;
    }

    CPU_REG.SP += value;
    CLR_ZERO_FLAG;
    CLR_ADD_SUB_FLAG;
    return opcode_table[opcode].cycles;
}

/*
inc  rr        x3           8 ---- rr = rr+1      ;rr may be BC,DE,HL,SP
*/
uint8_t opcode_16_inc(uint16_t opcode_address){
    uint8_t opcode = op_get_mem_map_8(opcode_address);
    uint16_t* dest =  get_reg_16_sp(GET_OPCODE_P(opcode));
    *dest += 1;
    return opcode_table[opcode].cycles;
}

/*
dec  rr        xB           8 ---- rr = rr-1      ;rr may be BC,DE,HL,SP
*/
uint8_t opcode_16_dec(uint16_t opcode_address){
    uint8_t opcode = op_get_mem_map_8(opcode_address);
    uint16_t* dest =  get_reg_16_sp(GET_OPCODE_P(opcode));
    *dest -= 1;
    return opcode_table[opcode].cycles;
}
