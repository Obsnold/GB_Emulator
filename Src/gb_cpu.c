#include "gb_mem_map.h"
#include "gb_common.h"
#include "gb_opcodes.h"
#include "gb_cpu.h"
#include "debug_print.h"

uint8_t current_cycle_time = 0;

// cpu modes and flags
enum cpu_power power_mode;
//bool interrupts_enabled = false;
unsigned long cpu_prev_tick = 0;
unsigned long cpu_cycles = 0;

void cpu_init(){
    CPU_REG.BC = 0;
    CPU_REG.DE = 0;
    CPU_REG.HL = 0;
    CPU_REG.AF = 0;
    CPU_REG.PC = 0;
    CPU_REG.SP = 0;
}

void cpu_set_power_mode(enum cpu_power mode ){
    power_mode = mode;
}

#define clear() printf("\033[H\033[J")

void gb_cpu(){
    // get time 
    unsigned long tick = get_ticks();
    cpu_cycles += tick - cpu_prev_tick;
    cpu_prev_tick = tick;

    //cpu_reg_print();

    //check interrupts
    /*if(interrupts_enabled){
        if(){

        } else if(){

        } else if(){
            
        } else if(){
            
        }

    }*/

    //if no interrupts then process normally
    if(cpu_cycles > (current_cycle_time * 250)){
        if(CPU_REG.PC == 0x100 || CPU_REG.PC == 0x00){
            print_memory(TILE_RAM,0x8200);
            print_memory(0x134,0x14E);
        }
        //clear();
        //if(CPU_REG.PC != 0x60 && CPU_REG.PC != 0x62 && CPU_REG.PC != 0x64 && CPU_REG.PC != 0x66 && CPU_REG.PC != 0x68 && CPU_REG.PC != 0x6A && CPU_REG.PC != 0x6B){
          if(CPU_REG.PC > 0xF3){          
                    print_cpu_reg();
                   // printf("LCD_SCY=%02x, LCD_SCX=%02x\n",gb_mem_map[LCD_SCY],gb_mem_map[LCD_SCX]);
                   //printf("HL=%04x, %02x\n",CPU_REG.HL,gb_mem_map[CPU_REG.HL]);
                    print_opcode();
                   // print_memory(0x104,0x133);
        }    
        uint8_t opcode = gb_mem_map[CPU_REG.PC];
        uint16_t temp_pc = CPU_REG.PC;
        CPU_REG.PC += opcode_table[opcode].length;
        current_cycle_time = opcode_table[opcode].operation(temp_pc);
    }
}


