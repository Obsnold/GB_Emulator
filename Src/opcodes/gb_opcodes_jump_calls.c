#include "gb_opcodes.h"
#include "gb_mem_map.h"
#include "gb_common.h"
#include "gb_cpu.h"


//******************************************************
//jump commands
//******************************************************
/*
jp   nn        C3 nn nn    16 ---- jump to nn, PC=nn
*/
uint8_t opcode_jp(uint16_t opcode_address){
    uint8_t opcode = gb_mem_map[opcode_address];
    CPU_REG.PC_1 = gb_mem_map[opcode_address+1];
    CPU_REG.PC_2 = gb_mem_map[opcode_address+2];
    return opcode_table[opcode].cycles;
}

/*
jp   (HL)        E9           4 ---- jump to HL, PC=(HL)
*/
uint8_t opcode_jp_hl(uint16_t opcode_address){
    uint8_t opcode = gb_mem_map[opcode_address];
    CPU_REG.PC_1 = gb_mem_map[CPU_REG.HL];
    CPU_REG.PC_2 = gb_mem_map[CPU_REG.HL+1];
    return opcode_table[opcode].cycles;
}

/*
jp   f,nn      xx nn nn 16;12 ---- conditional jump if nz,z,nc,c
*/
uint8_t opcode_jp_cnd(uint16_t opcode_address){
    uint8_t opcode = gb_mem_map[opcode_address];
    uint8_t cycles = CYCLE_3;

    if(get_flag_condition(GET_OPCODE_Y(opcode))){
        CPU_REG.PC_1 = gb_mem_map[opcode_address+1];
        CPU_REG.PC_2 = gb_mem_map[opcode_address+2];
        cycles = ALT_CYCLE_4;
    }
    return cycles;
}

/*
jr   PC+dd     18 dd       12 ---- relative jump to nn (PC=PC+/-7bit)
*/
uint8_t opcode_jr(uint16_t opcode_address){
    uint8_t opcode = gb_mem_map[opcode_address];
    uint16_t temp = get_16_from_8(&CPU_REG.PC_1);
    temp += (int8_t) gb_mem_map[opcode_address+1];
    CPU_REG.PC_1 = get_16_high(&temp);
    CPU_REG.PC_2 = get_16_low(&temp);
    return opcode_table[opcode].cycles;
}

/*
jr   f,PC+dd   xx dd     12;8 ---- conditional relative jump if nz,z,nc,c
*/
uint8_t opcode_jr_cnd(uint16_t opcode_address){
    uint8_t opcode = gb_mem_map[opcode_address];
    uint8_t cycles = CYCLE_2;

    if(get_flag_condition(GET_OPCODE_Y(opcode-4))){
        uint16_t temp = get_16_from_8(&CPU_REG.PC_1);
        temp += (int8_t) gb_mem_map[opcode_address+1];
        CPU_REG.PC_1 = get_16_high(&temp);
        CPU_REG.PC_2 = get_16_low(&temp);
        cycles = ALT_CYCLE_3;
    }
    return cycles;
}

/*
call nn        CD nn nn    24 ---- call to nn, SP=SP-2, (SP)=PC, PC=nn
*/
uint8_t opcode_call(uint16_t opcode_address){
    uint8_t opcode = gb_mem_map[opcode_address];
    uint16_t temp = get_16_from_8(&CPU_REG.SP_1);
    temp -= 2;
    CPU_REG.SP_1 = get_16_high(&temp);
    CPU_REG.SP_2 = get_16_low(&temp);
    gb_mem_map[temp] = CPU_REG.PC_1;
    gb_mem_map[temp+1] = CPU_REG.PC_2;
    CPU_REG.PC_1 = gb_mem_map[opcode_address+1];
    CPU_REG.PC_2 = gb_mem_map[opcode_address+2];
    return opcode_table[opcode].cycles;
}

/*
call f,nn      xx nn nn 24;12 ---- conditional call if nz,z,nc,c
*/
uint8_t opcode_call_cnd(uint16_t opcode_address){
    uint8_t opcode = gb_mem_map[opcode_address];
    uint8_t cycles = CYCLE_3;

    if(get_flag_condition(GET_OPCODE_Y(opcode))){
        uint16_t temp = get_16_from_8(&CPU_REG.SP_1);
        temp -= 2;
        CPU_REG.SP_1 = get_16_high(&temp);
        CPU_REG.SP_2 = get_16_low(&temp);
        gb_mem_map[temp] = CPU_REG.PC_1;
        gb_mem_map[temp+1] = CPU_REG.PC_2;
        CPU_REG.PC_1 = gb_mem_map[opcode_address+1];
        CPU_REG.PC_2 = gb_mem_map[opcode_address+2];
        cycles = ALT_CYCLE_6;
    }
    return cycles;
}

/*
ret            C9          16 ---- return, PC=(SP), SP=SP+2
*/
uint8_t opcode_ret(uint16_t opcode_address){
    uint8_t opcode = gb_mem_map[opcode_address];
    uint16_t temp = get_16_from_8(&CPU_REG.SP_1);
    CPU_REG.PC_1 = gb_mem_map[temp];
    CPU_REG.PC_2 = gb_mem_map[temp+1];
    temp += 2;
    CPU_REG.SP_1 = get_16_high(&temp);
    CPU_REG.SP_2 = get_16_low(&temp);
    return opcode_table[opcode].cycles;
}

/*
ret  f         xx        20;8 ---- conditional return if nz,z,nc,c
*/
uint8_t opcode_ret_cnd(uint16_t opcode_address){
    uint8_t opcode = gb_mem_map[opcode_address];
    uint8_t cycles = CYCLE_2;

    if(get_flag_condition(GET_OPCODE_Y(opcode-4))){
        uint16_t temp = get_16_from_8(&CPU_REG.SP_1);
        CPU_REG.PC_1 = gb_mem_map[temp];
        CPU_REG.PC_2 = gb_mem_map[temp+1];
        temp += 2;
        CPU_REG.SP_1 = get_16_high(&temp);
        CPU_REG.SP_2 = get_16_low(&temp);
        cycles = ALT_CYCLE_5;
    }
    return cycles;
}

/*
reti           D9          16 ---- return and enable interrupts (IME=1)
*/
uint8_t opcode_reti(uint16_t opcode_address){
    uint8_t opcode = gb_mem_map[opcode_address];
    opcode_ret(opcode_address);
    opcode_ei(opcode_address);
    return opcode_table[opcode].cycles;
}

/*
rst  n         xx          16 ---- call to 00,08,10,18,20,28,30,38
*/
uint8_t opcode_rst(uint16_t opcode_address){
    uint8_t opcode = gb_mem_map[opcode_address];
    uint8_t opcode_y = GET_OPCODE_Y(opcode);
    opcode_call(opcode_y*8);
    return opcode_table[opcode].cycles;
}
