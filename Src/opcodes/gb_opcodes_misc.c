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
void opcode_ccf(){
    CLR_ADD_SUB_FLAG;
    CLR_HALF_CARRY_FLAG;
    if(GET_CARRY_FLAG){
        CLR_CARRY_FLAG;
    } else {
        SET_CARRY_FLAG;
    }
}

/*
scf            37           4 -001 cy=1
*/
void opcode_scf(){
    CLR_ADD_SUB_FLAG;
    CLR_HALF_CARRY_FLAG;
    SET_CARRY_FLAG;
}

/*
nop            00           4 ---- no operation
*/
void opcode_nop(){
    //do nothing.....
}

/*
halt           76         N*4 ---- halt until interrupt occurs (low power)
*/
void opcode_halt(){
    cpu_set_power_mode(PWR_HALT);
}

/*
stop           10 00        ? ---- low power standby mode (VERY low power)
*/
void opcode_stop(){
    cpu_set_power_mode(PWR_STOP);
}

/*
di             F3           4 ---- disable interrupts, IME=0
*/
void opcode_di(){
    gb_mem_map[INTERRUPT_EN] = 0x00;
}

/*
ei             FB           4 ---- enable interrupts, IME=1
*/
void opcode_ei(){
    gb_mem_map[INTERRUPT_EN] = 0x01;
}
