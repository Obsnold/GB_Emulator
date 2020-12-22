#include "gb_opcodes.h"
#include "gb_mem_map.h"
#include "gb_common.h"
#include "gb_cpu.h"
#include "debug_print.h"

#ifdef DEBUG
#define DEBUG_PRINT(fmt, args...)    PRINT(fmt, ## args)
#else
#define DEBUG_PRINT(fmt, args...)
#endif

//******************************************************
//jump commands
//******************************************************
/*
jp   nn        C3 nn nn    16 ---- jump to nn, PC=nn
*/
uint8_t opcode_jp(uint16_t opcode_address){
    uint8_t opcode = get_mem_map_8(opcode_address);
    CPU_REG.PC = get_mem_map_16(opcode_address+1);
    DEBUG_PRINT("%s: CPU_REG.PC = %04x\n",__func__,CPU_REG.PC);
    return opcode_table[opcode].cycles;
}

/*
jp   (HL)        E9           4 ---- jump to HL, PC=(HL)
*/
uint8_t opcode_jp_hl(uint16_t opcode_address){
    uint8_t opcode = get_mem_map_8(opcode_address);
    CPU_REG.PC = CPU_REG.HL;//get_mem_map_16(CPU_REG.HL);
    DEBUG_PRINT("%s: CPU_REG.PC = %04x\n",__func__,CPU_REG.PC);
    return opcode_table[opcode].cycles;
}

/*
jp   f,nn      xx nn nn 16;12 ---- conditional jump if nz,z,nc,c
*/
uint8_t opcode_jp_cnd(uint16_t opcode_address){
    uint8_t opcode = get_mem_map_8(opcode_address);
    uint8_t cycles = CYCLE_3;

    if(get_flag_condition(GET_OPCODE_Y(opcode))){
        CPU_REG.PC = get_mem_map_16(opcode_address+1);
        cycles =CYCLE_4;
        DEBUG_PRINT("%s: CPU_REG.PC = %04x\n",__func__,CPU_REG.PC);
    }
    return cycles;
}

/*
jr   PC+dd     18 dd       12 ---- relative jump to nn (PC=PC+/-7bit)
*/
uint8_t opcode_jr(uint16_t opcode_address){
    uint8_t opcode = get_mem_map_8(opcode_address);
    int8_t temp = (int8_t)get_mem_map_8(opcode_address+1);
    CPU_REG.PC += temp;
    DEBUG_PRINT("%s: CPU_REG.PC = %04x\n",__func__,CPU_REG.PC);
    return opcode_table[opcode].cycles;
}

/*
jr   f,PC+dd   xx dd     12;8 ---- conditional relative jump if nz,z,nc,c
*/
uint8_t opcode_jr_cnd(uint16_t opcode_address){
    uint8_t opcode = get_mem_map_8(opcode_address);
    uint8_t cycles = CYCLE_2;
    if(get_flag_condition(GET_OPCODE_Y(opcode)-4) == true){
        int8_t temp = (int8_t)get_mem_map_8(opcode_address+1);
        CPU_REG.PC += temp;
        cycles = CYCLE_3;
        DEBUG_PRINT("%s: CPU_REG.PC = %04x\n",__func__,CPU_REG.PC);
    }
    return cycles;
}

/*
call nn        CD nn nn    24 ---- call to nn, SP=SP-2, (SP)=PC, PC=nn
*/
uint8_t opcode_call(uint16_t opcode_address){
    uint8_t opcode = get_mem_map_8(opcode_address);
    CPU_REG.SP -= 2;
    set_mem_map_16(CPU_REG.SP,CPU_REG.PC);
    CPU_REG.PC = get_mem_map_16(opcode_address+1);
    DEBUG_PRINT("%s: CPU_REG.PC = %04x\n",__func__,CPU_REG.PC);
    return opcode_table[opcode].cycles;
}

/*
call f,nn      xx nn nn 24;12 ---- conditional call if nz,z,nc,c
*/
uint8_t opcode_call_cnd(uint16_t opcode_address){
    uint8_t opcode = get_mem_map_8(opcode_address);
    uint8_t cycles = CYCLE_3;

    if(get_flag_condition(GET_OPCODE_Y(opcode))){
        DEBUG_PRINT("----------------");
        CPU_REG.SP -= 2;
        set_mem_map_16(CPU_REG.SP,CPU_REG.PC);
        CPU_REG.PC = get_mem_map_16(opcode_address+1);
        cycles =CYCLE_6;
        DEBUG_PRINT("%s: CPU_REG.PC = %04x\n",__func__,CPU_REG.PC);
    }
    return cycles;
}

/*
ret            C9          16 ---- return, PC=(SP), SP=SP+2
*/
uint8_t opcode_ret(uint16_t opcode_address){
    uint8_t opcode = get_mem_map_8(opcode_address);
    CPU_REG.PC = get_mem_map_16(CPU_REG.SP);
    CPU_REG.SP += 2;
    DEBUG_PRINT("%s: CPU_REG.PC = %04x\n",__func__,CPU_REG.PC);
    return opcode_table[opcode].cycles;
}

/*
ret  f         xx        20;8 ---- conditional return if nz,z,nc,c
*/
uint8_t opcode_ret_cnd(uint16_t opcode_address){
    uint8_t opcode = get_mem_map_8(opcode_address);
    uint8_t cycles = CYCLE_2;

    if(get_flag_condition(GET_OPCODE_Y(opcode))){
        CPU_REG.PC = get_mem_map_16(CPU_REG.SP);
        CPU_REG.SP += 2;
        cycles =CYCLE_5;
        DEBUG_PRINT("%s: CPU_REG.PC = %04x\n",__func__,CPU_REG.PC);
    }
    return cycles;
}

/*
reti           D9          16 ---- return and enable interrupts (IME=1)
*/
uint8_t opcode_reti(uint16_t opcode_address){
    uint8_t opcode = get_mem_map_8(opcode_address);
    opcode_ret(opcode_address);
    opcode_ei(opcode_address);
    DEBUG_PRINT("%s: CPU_REG.PC = %04x\n",__func__,CPU_REG.PC);
    return opcode_table[opcode].cycles;
}

/*
rst  n         xx          16 ---- call to 00,08,10,18,20,28,30,38
*/
uint8_t opcode_rst(uint16_t opcode_address){
    uint8_t opcode = get_mem_map_8(opcode_address);
    uint8_t opcode_y = GET_OPCODE_Y(opcode);
    CPU_REG.SP -= 2;
    set_mem_map_16(CPU_REG.SP,CPU_REG.PC);
    CPU_REG.PC = opcode_y*8;
    DEBUG_PRINT("%s: CPU_REG.PC = %04x\n",__func__,CPU_REG.PC);
    //opcode_call(opcode_y*8);
    return opcode_table[opcode].cycles;
}
