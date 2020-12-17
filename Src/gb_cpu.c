#include "gb_mem_map.h"
#include "gb_opcodes.h"
#include "gb_cpu.h"
#include "debug_print.h"

uint8_t current_cycle_time = 0;

// cpu modes and flags
enum cpu_power power_mode;
bool interrupts_enabled = false;
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

void set_interrupts(bool set){
    interrupts_enabled = set;
}

#define clear() printf("\033[H\033[J")
uint8_t temp_LCDY = 0;

void gb_cpu(){
    // get time 
    unsigned long tick = get_ns();
    cpu_cycles += tick - cpu_prev_tick;
    cpu_prev_tick = tick;

    //cpu_reg_print();

    //check interrupts
    if(interrupts_enabled){
        uint16_t interrupt_address = 0;
        if(GET_MEM_MAP(INTERRUPT_ENABLE,INTERRUPT_V_BLANK) && GET_MEM_MAP(INTERRUPT_FLAGS,INTERRUPT_V_BLANK)){
            interrupt_address = 0x40;
            CLR_MEM_MAP(INTERRUPT_FLAGS,INTERRUPT_V_BLANK);
        } else if(GET_MEM_MAP(INTERRUPT_ENABLE,INTERRUPT_LCD_STAT) && GET_MEM_MAP(INTERRUPT_FLAGS,INTERRUPT_LCD_STAT)){
            interrupt_address = 0x48;
            CLR_MEM_MAP(INTERRUPT_FLAGS,INTERRUPT_LCD_STAT);
        } else if(GET_MEM_MAP(INTERRUPT_ENABLE,INTERRUPT_TIMER) && GET_MEM_MAP(INTERRUPT_FLAGS,INTERRUPT_TIMER)){
            interrupt_address = 0x50;
            CLR_MEM_MAP(INTERRUPT_FLAGS,INTERRUPT_TIMER);
        } else if(GET_MEM_MAP(INTERRUPT_ENABLE,INTERRUPT_SERIAL) && GET_MEM_MAP(INTERRUPT_FLAGS,INTERRUPT_SERIAL)){
            interrupt_address = 0x58;
            CLR_MEM_MAP(INTERRUPT_FLAGS,INTERRUPT_SERIAL);
        } else if(GET_MEM_MAP(INTERRUPT_ENABLE,INTERRUPT_JOYPAD) && GET_MEM_MAP(INTERRUPT_FLAGS,INTERRUPT_JOYPAD)){
            interrupt_address = 0x60;
            CLR_MEM_MAP(INTERRUPT_FLAGS,INTERRUPT_JOYPAD);
        }


        if(interrupt_address > 0){
            interrupts_enabled = false;
            CPU_REG.SP -= 2;
            gb_mem_map[CPU_REG.SP] = CPU_REG.PC_1;
            gb_mem_map[CPU_REG.SP+1] = CPU_REG.PC_2;
            CPU_REG.PC = interrupt_address;
        }
    }

    //if no interrupts then process normally
    if(cpu_cycles > (current_cycle_time * 250)){
        /*if(CPU_REG.PC<0x235){
            if(CPU_REG.PC>0x230){
                print_opcode();
                print_cpu_reg();
            }*/
            uint8_t opcode = gb_mem_map[CPU_REG.PC];
            uint16_t temp_pc = CPU_REG.PC;
            CPU_REG.PC += opcode_table[opcode].length;
            current_cycle_time = opcode_table[opcode].operation(temp_pc);
        //}
    }
}


