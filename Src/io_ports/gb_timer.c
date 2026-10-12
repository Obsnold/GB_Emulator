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

// Tick periods in nanoseconds, from the DMG's 4.194304 MHz clock.
#define DIV_PERIOD_NS       61035UL      //  16384 Hz, fixed
#define TAC_PERIOD_1024_NS 244141UL      //   4096 Hz
#define TAC_PERIOD_16_NS     3815UL      // 262144 Hz
#define TAC_PERIOD_64_NS    15259UL      //  65536 Hz
#define TAC_PERIOD_256_NS   61035UL      //  16384 Hz

unsigned long clock_timer = 0;
unsigned long div_timer = 0;

static unsigned long tac_period_ns(void){
    switch(get_mem_map_bit(TIMER_TAC,TAC_CLOCK_SELECT)){
        case TAC_CLOCK_16:  return TAC_PERIOD_16_NS;
        case TAC_CLOCK_64:  return TAC_PERIOD_64_NS;
        case TAC_CLOCK_256: return TAC_PERIOD_256_NS;
        case TAC_CLOCK_1024:
        default:            return TAC_PERIOD_1024_NS;
    }
}

void gb_timer(unsigned long delta_time){
    unsigned long tima_period;

    // The accumulators have to take every delta, not just the ones large
    // enough to complete a tick on their own: a main loop step is a few
    // microseconds and a tick is 61 of them, so only accumulating inside the
    // "has a tick elapsed" test dropped the time entirely and the registers
    // never moved. Subtracting a whole period per tick is what keeps the
    // remainder bounded; the previous code left it to grow without limit, so
    // once it did start ticking it accelerated.
    div_timer += delta_time;
    while(div_timer >= DIV_PERIOD_NS){
        div_timer -= DIV_PERIOD_NS;
        set_mem_map_8(TIMER_DIV, (uint8_t)(get_mem_map_8(TIMER_DIV) + 1));
    }

    if(!get_mem_map_bit(TIMER_TAC,TAC_ENABLE)){
        return;
    }

    tima_period = tac_period_ns();

    clock_timer += delta_time;
    while(clock_timer >= tima_period){
        clock_timer -= tima_period;

        // Counting one tick at a time makes the overflow exact. Comparing the
        // new value against the old missed a wrap of exactly 256 ticks.
        if(get_mem_map_8(TIMER_TIMA) == 0xFF){
            set_mem_map_8(TIMER_TIMA, get_mem_map_8(TIMER_TMA));
            set_mem_map_bit(INTERRUPT_FLAGS,INTERRUPT_TIMER);
        } else {
            set_mem_map_8(TIMER_TIMA, (uint8_t)(get_mem_map_8(TIMER_TIMA) + 1));
        }
    }
}
