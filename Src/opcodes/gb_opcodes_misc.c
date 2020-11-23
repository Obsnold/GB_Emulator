#include "gb_opcodes.h"
#include "gb_mem_map.h"
#include "gb_common.h"
#include "gb_cpu.h"


//******************************************************
//cpu commands
//******************************************************
/*
ccf            3F           4 -00c cy=cy xor 1
*/
uint8_t opcode_ccf(uint16_t opcode_address){
    uint8_t opcode = gb_mem_map[opcode_address];
    CLR_ADD_SUB_FLAG;
    CLR_HALF_CARRY_FLAG;
    if(GET_CARRY_FLAG){
        CLR_CARRY_FLAG;
    } else {
        SET_CARRY_FLAG;
    }
    return opcode_table[opcode].cycles;
}

/*
scf            37           4 -001 cy=1
*/
uint8_t opcode_scf(uint16_t opcode_address){
    uint8_t opcode = gb_mem_map[opcode_address];
    CLR_ADD_SUB_FLAG;
    CLR_HALF_CARRY_FLAG;
    SET_CARRY_FLAG;
    return opcode_table[opcode].cycles;
}

/*
nop            00           4 ---- no operation
*/
uint8_t opcode_nop(uint16_t opcode_address){
    uint8_t opcode = gb_mem_map[opcode_address];
    //do nothing.....
    return opcode_table[opcode].cycles;
}

/*
halt           76         N*4 ---- halt until interrupt occurs (low power)
*/
uint8_t opcode_halt(uint16_t opcode_address){
    uint8_t opcode = gb_mem_map[opcode_address];
    cpu_set_power_mode(PWR_HALT);
    return opcode_table[opcode].cycles;
}

/*
stop           10 00        ? ---- low power standby mode (VERY low power)
*/
uint8_t opcode_stop(uint16_t opcode_address){
    uint8_t opcode = gb_mem_map[opcode_address];
    cpu_set_power_mode(PWR_STOP);
    return opcode_table[opcode].cycles;
}

/*
di             F3           4 ---- disable interrupts, IME=0
*/
uint8_t opcode_di(uint16_t opcode_address){
    uint8_t opcode = gb_mem_map[opcode_address];
    gb_mem_map[INTERRUPT_EN] = 0x00;
    return opcode_table[opcode].cycles;
}

/*
ei             FB           4 ---- enable interrupts, IME=1
*/
uint8_t opcode_ei(uint16_t opcode_address){
    uint8_t opcode = gb_mem_map[opcode_address];
    gb_mem_map[INTERRUPT_EN] = 0x01;
    return opcode_table[opcode].cycles;
}

uint8_t opcode_NA(uint16_t opcode_address){
    uint8_t opcode = gb_mem_map[opcode_address];
    //throw error
    return opcode_table[opcode].cycles;
}