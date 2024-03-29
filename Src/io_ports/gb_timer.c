#include "gb_timer.h"
#include "gb_mem_map.h"
#include "debug_print.h"
#include <time.h>

/*
           00: CPU Clock / 1024 (DMG, SGB2, CGB Single Speed Mode:   4096 Hz, SGB1:   ~4194 Hz, CGB Double Speed Mode:   8192 Hz)
           01: CPU Clock / 16   (DMG, SGB2, CGB Single Speed Mode: 262144 Hz, SGB1: ~268400 Hz, CGB Double Speed Mode: 524288 Hz)
           10: CPU Clock / 64   (DMG, SGB2, CGB Single Speed Mode:  65536 Hz, SGB1:  ~67110 Hz, CGB Double Speed Mode: 131072 Hz)
           11: CPU Clock / 256  (DMG, SGB2, CGB Single Speed Mode:  16384 Hz, SGB1:  ~16780 Hz, CGB Double Speed Mode:  32768 Hz)
*/

unsigned long clock_timer = 0;
unsigned long div_timer = 0;

void gb_timer(unsigned long delta_time){
    // divider register is always updated at a rate of 16384Hz
    // or 1 tick every 61035ns
    if((div_timer + delta_time) > 61035){
        uint8_t temp = get_mem_map_8(TIMER_DIV)+(div_timer / 61035);
        set_mem_map_8(TIMER_DIV, temp);
        div_timer += (delta_time % 61035);
    }

    if(get_mem_map_bit(TIMER_TAC,TAC_ENABLE)){
        unsigned long  clock_timer_limit = 244141;
        switch(get_mem_map_bit(TIMER_TAC,TAC_CLOCK_SELECT)){
            case TAC_CLOCK_1024:
                clock_timer_limit = 244141;
                break;
            case TAC_CLOCK_16:
                clock_timer_limit = 3815;
                break;
            case TAC_CLOCK_64:
                clock_timer_limit = 15259;
                break;
            case TAC_CLOCK_256:
                clock_timer_limit = 61035;
                break;
        }

        if((clock_timer + delta_time) > clock_timer_limit){
            uint8_t temp_time = get_mem_map_8(TIMER_TIMA);
            uint8_t temp = get_mem_map_8(TIMER_TIMA) + (clock_timer / clock_timer_limit);
            set_mem_map_8(TIMER_TIMA,temp);
            if(temp < temp_time){
                set_mem_map_8(TIMER_TIMA, get_mem_map_8(TIMER_TMA));
                set_mem_map_bit(INTERRUPT_FLAGS,INTERRUPT_TIMER);
            }
            clock_timer += (delta_time % clock_timer_limit);
        }
    }
}