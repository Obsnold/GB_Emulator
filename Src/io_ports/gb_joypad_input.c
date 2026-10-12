#include "gb_joypad_input.h"
#include "gb_mem_map.h"

// Active low, so 0xFF is "nothing pressed" - the state after reset.
uint8_t previous_keys = 0xFF;

void gb_input(uint8_t keys){
    // The joypad interrupt is a high-to-low transition, which with active-low
    // keys means a key going down. Firing on any change ran a game's input
    // handler a second time on release.
    if((previous_keys & (uint8_t)~keys) != 0){
        set_mem_map_bit(INTERRUPT_FLAGS,INTERRUPT_JOYPAD);
    }

    // Bits 4 and 5 are the game's choice of which half of the pad to read, so
    // they have to survive the read. Writing 0xF0 set both of them, throwing
    // the selection away and forcing the game to re-select before every read.
    uint8_t select = get_mem_map_8(JOYPAD_INPUT) & (JOYPAD_BUT | JOYPAD_DIR);
    uint8_t lines = 0x0F;       // active low: nothing pressed

    if((select & JOYPAD_BUT) == 0){
        lines = (uint8_t)((keys >> 4) & 0x0F);
    } else if((select & JOYPAD_DIR) == 0){
        lines = (uint8_t)(keys & 0x0F);
    }

    // bits 6 and 7 are unused and always read high
    set_mem_map_8(JOYPAD_INPUT, (uint8_t)(0xC0 | select | lines));
    previous_keys = keys;
}