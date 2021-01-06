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

#define clear() DEBUG_PRINT("\033[H\033[J")
uint8_t temp_LCDY = 0;

void gb_cpu(){
    // get time 
    unsigned long tick = get_ns();
    cpu_cycles += tick - cpu_prev_tick;
    cpu_prev_tick = tick;

    //dirty trick for now
    //if CPU_REG.F has been written to make sure the bottom 4 bits are 0
    CPU_REG.F &= 0xF0;

    if(power_mode == PWR_HALT){
        if(gb_mem_map[INTERRUPT_ENABLE] & gb_mem_map[INTERRUPT_FLAGS]){
            power_mode = PWR_NORMAL;
        }
    }

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
            cpu_set_power_mode(PWR_NORMAL);
            interrupts_enabled = false;
            CPU_REG.SP -= 2;
            set_mem_map_16(CPU_REG.SP, CPU_REG.PC);
            CPU_REG.PC = interrupt_address;
        }
    }

    //if no interrupts then process normally
    if(power_mode == PWR_NORMAL){
        if(cpu_cycles > (current_cycle_time * CYCLE_TIME)){
            //if(CPU_REG.PC<0x235){
                //if(CPU_REG.PC>=0x0600 && CPU_REG.PC<=0x060f){
                    //print_cpu_reg();
                    //print_indirect_reg();
                    //print_opcode();
                //}
                uint8_t opcode = gb_mem_map[CPU_REG.PC];
                uint16_t temp_pc = CPU_REG.PC;
                CPU_REG.PC += opcode_table[opcode].length;
                current_cycle_time = opcode_table[opcode].operation(temp_pc);
            //}
        }
    }
}


