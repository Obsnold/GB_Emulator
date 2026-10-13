#include "gb_mem_map.h"
#include "gb_opcodes.h"
#include "gb_cpu.h"
#include "debug_print.h"

uint8_t current_cycle_time = 0;
struct cpu_reg CPU_REG;

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

// Most emulated time the CPU will try to make up in one call. A host stall
// longer than this is dropped rather than replayed as a burst of fast
// emulation, and it bounds the catch-up loop below. Two frames absorbs
// ordinary jitter without being long enough to notice.
#define CPU_CATCHUP_LIMIT_NS (2UL * 16742706UL)

// Leave a low power mode if the condition for leaving it has arrived.
static void cpu_check_wake(void){
    if(power_mode == PWR_HALT){
        if(get_mem_map_8(INTERRUPT_ENABLE) & get_mem_map_8(INTERRUPT_FLAGS)){
            power_mode = PWR_NORMAL;
        }
    } else if(power_mode == PWR_STOP){
        // STOP is left by a joypad input, and that does not depend on IME or
        // on the interrupt being enabled. Without this the only way out was
        // interrupt dispatch below, so a STOP with interrupts disabled wedged
        // the CPU for good.
        if(get_mem_map_bit(INTERRUPT_FLAGS,INTERRUPT_JOYPAD)){
            power_mode = PWR_NORMAL;
        }
    }
}

// Dispatch the highest priority pending interrupt, if one is pending and the
// master enable allows it.
static void cpu_service_interrupts(void){
    uint16_t interrupt_address = 0;

    if(!interrupts_enabled){
        return;
    }

    if(get_mem_map_bit(INTERRUPT_ENABLE,INTERRUPT_V_BLANK) && get_mem_map_bit(INTERRUPT_FLAGS,INTERRUPT_V_BLANK)){
        interrupt_address = VBLANK_INTR_ADDR;
        clear_mem_map_bit(INTERRUPT_FLAGS,INTERRUPT_V_BLANK);
    } else if(get_mem_map_bit(INTERRUPT_ENABLE,INTERRUPT_LCD_STAT) && get_mem_map_bit(INTERRUPT_FLAGS,INTERRUPT_LCD_STAT)){
        interrupt_address = LCDC_STAT_INTR_ADDR;
        clear_mem_map_bit(INTERRUPT_FLAGS,INTERRUPT_LCD_STAT);
    } else if(get_mem_map_bit(INTERRUPT_ENABLE,INTERRUPT_TIMER) && get_mem_map_bit(INTERRUPT_FLAGS,INTERRUPT_TIMER)){
        interrupt_address = TIMER_OVERFLOW_INTR_ADDR;
        clear_mem_map_bit(INTERRUPT_FLAGS,INTERRUPT_TIMER);
    } else if(get_mem_map_bit(INTERRUPT_ENABLE,INTERRUPT_SERIAL) && get_mem_map_bit(INTERRUPT_FLAGS,INTERRUPT_SERIAL)){
        interrupt_address = SERIAL_COMPLETE_INTR_ADDR;
        clear_mem_map_bit(INTERRUPT_FLAGS,INTERRUPT_SERIAL);
    } else if(get_mem_map_bit(INTERRUPT_ENABLE,INTERRUPT_JOYPAD) && get_mem_map_bit(INTERRUPT_FLAGS,INTERRUPT_JOYPAD)){
        interrupt_address = HTL_P10_P13_INTR_ADDR;
        clear_mem_map_bit(INTERRUPT_FLAGS,INTERRUPT_JOYPAD);
    }

    if(interrupt_address > 0){
        cpu_set_power_mode(PWR_NORMAL);
        interrupts_enabled = false;
        CPU_REG.SP -= 2;
        set_mem_map_16(CPU_REG.SP, CPU_REG.PC);
        CPU_REG.PC = interrupt_address;
    }
}

// Execute one instruction, if enough emulated time has accumulated to pay for
// the one before it. Returns false when there is not, which ends the catch-up
// loop.
static bool cpu_step_instruction(void){
    unsigned long cost = (unsigned long)current_cycle_time * CYCLE_TIME;
    uint8_t opcode;
    uint16_t temp_pc;

    if(cpu_cycles <= cost){
        return false;
    }
    cpu_cycles -= cost;

    opcode = get_mem_map_8(CPU_REG.PC);
    temp_pc = CPU_REG.PC;
    CPU_REG.PC += opcode_table[opcode].length;
    current_cycle_time = opcode_table[opcode].operation(temp_pc);
    return true;
}

void gb_cpu(unsigned long delta_time){

    cpu_cycles += delta_time;

    if(cpu_cycles > CPU_CATCHUP_LIMIT_NS){
        cpu_cycles = CPU_CATCHUP_LIMIT_NS;
    }

    // Retire as many instructions as the accumulated time pays for, rather
    // than one per call. One per call made the emulated speed a function of
    // how fast the host happened to loop, which only worked because the native
    // loop spun flat out; anything that calls this on a schedule - a frame
    // callback, say - would run the CPU at the callback rate.
    //
    // The interrupt and wake checks belong inside the loop, not before it: run
    // once per call instead, an interrupt would wait for the whole batch of
    // instructions to retire before being serviced.
    for(;;){
        // F's low four bits do not exist on hardware and always read as 0.
        CPU_REG.F &= 0xF0;

        cpu_check_wake();
        cpu_service_interrupts();

        if(power_mode != PWR_NORMAL){
            break;      // halted or stopped: nothing retires until it wakes
        }
        if(!cpu_step_instruction()){
            break;      // not enough accumulated time for the next instruction
        }
    }
}
