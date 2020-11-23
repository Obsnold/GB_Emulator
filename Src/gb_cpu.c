#include "gb_mem_map.h"
#include "gb_common.h"
#include "gb_opcodes.h"
#include "gb_cpu.h"

//uint8_t gb_cpu_reg[GB_REG_SIZE];

uint8_t current_cycle_time = 0;

gb_cpu_reg CPU_REG;

// cpu modes and flags
enum cpu_power power_mode;
//bool interrupts_enabled = false;
uint8_t current_opcode_time = 0;

void cpu_set_power_mode(enum cpu_power mode ){
    power_mode = mode;
}

void gb_cpu(){
    uint8_t opcode = gb_mem_map[CPU_REG.PC];

    printf("%d [ %02x ", CPU_REG.PC, opcode);
    for(int i = 1; i < opcode_table[opcode].length;i++){
        printf("%02x ",gb_mem_map[CPU_REG.PC+i]);
    }
    printf("] %02x, %02x, %s", opcode_table[opcode].length, opcode_table[opcode].cycles, opcode_table[opcode].description);
    current_cycle_time = opcode_table[opcode].operation(CPU_REG.PC);
    
    // if we have an alt time then the program counter has been altered and there is no need to increment it
    // this should only happen when a jump or call or something happens
    if((current_cycle_time & ALT_CYCLE_MASK) == 0){
        CPU_REG.PC += opcode_table[opcode].length;
         //printf("--- %04x, %02x, %02x", CPU_REG.PC, CPU_REG.PC_1, CPU_REG.PC_2);
    }

    printf("\n");
    //check interrupts
    /*if(interrupts_enabled){
        if(){

        } else if(){

        } else if(){
            
        } else if(){
            
        }

    }*/

    //if no interrupts then process normally
    //process_opcode(opcode);
}


