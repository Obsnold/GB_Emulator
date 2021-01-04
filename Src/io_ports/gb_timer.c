#include "gb_timer.h"
#include "gb_mem_map.h"

unsigned long clock_timer = 0;
unsigned long div_timer = 0;

/*
           00: CPU Clock / 1024 (DMG, SGB2, CGB Single Speed Mode:   4096 Hz, SGB1:   ~4194 Hz, CGB Double Speed Mode:   8192 Hz)
           01: CPU Clock / 16   (DMG, SGB2, CGB Single Speed Mode: 262144 Hz, SGB1: ~268400 Hz, CGB Double Speed Mode: 524288 Hz)
           10: CPU Clock / 64   (DMG, SGB2, CGB Single Speed Mode:  65536 Hz, SGB1:  ~67110 Hz, CGB Double Speed Mode: 131072 Hz)
           11: CPU Clock / 256  (DMG, SGB2, CGB Single Speed Mode:  16384 Hz, SGB1:  ~16780 Hz, CGB Double Speed Mode:  32768 Hz)
*/

void gb_timer(){
    // get time 
    unsigned long timer = get_ns();

    // divider register is always updated at a rate of 16384Hz
    // or 1 tick every 61035ns
    if((timer - div_timer) > 61035){
        div_timer = timer;
        gb_mem_map[TIMER_DIV]++;
    }

    if(GET_MEM_MAP(TIMER_TAC,TAC_ENABLE)){
        unsigned long  clock_timer_limit = 244141;
        switch(GET_MEM_MAP(TIMER_TAC,TAC_CLOCK_SELECT)){
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

        if((timer - clock_timer) > clock_timer_limit){
            clock_timer = timer;
            uint8_t temp = gb_mem_map[TIMER_TIMA];
            gb_mem_map[TIMER_TIMA]++;
            if(temp == 0xFF && gb_mem_map[TIMER_TIMA] == 00){
                gb_mem_map[TIMER_TIMA] = gb_mem_map[TIMER_TMA];
                SET_MEM_MAP(INTERRUPT_FLAGS,INTERRUPT_TIMER);
            }
        }

    }
}