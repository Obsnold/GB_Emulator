#include "gb_joypad_input.h"
#include "gb_mem_map.h"

// Active low, so 0xFF is "nothing pressed" - the state after reset.
uint8_t previous_keys = 0xFF;

// Latest key state from the host. Held here rather than folded into the port
// register, because the low nibble the game reads depends on which half of the
// pad is selected at the moment of the read.
static uint8_t current_keys = 0xFF;

void gb_input(uint8_t keys){
    // The joypad interrupt is a high-to-low transition, which with active-low
    // keys means a key going down. Firing on any change ran a game's input
    // handler a second time on release.
    if((previous_keys & (uint8_t)~keys) != 0){
        set_mem_map_bit(INTERRUPT_FLAGS,INTERRUPT_JOYPAD);
    }

    current_keys = keys;
    previous_keys = keys;
}

uint8_t gb_joypad_read(void){
    // Bits 4 and 5 are the game's selection and read back as written; bits 6
    // and 7 are unused and read high.
    uint8_t select = get_mem_map_8(JOYPAD_INPUT) & (JOYPAD_BUT | JOYPAD_DIR);
    uint8_t lines = 0x0F;

    // The selection lines drive the key matrix and the result appears on the
    // low nibble immediately, so it is computed per read rather than written
    // when input is polled. Writing it on a poll only worked while the poll ran
    // between every instruction: a game selects a half and reads in the next
    // instruction, and reads both halves within one frame, so anything slower
    // than that handed it the nibble for the wrong half.
    //
    // Selecting both halves pulls a line low if either half has that key down,
    // hence the AND rather than an either/or.
    if((select & JOYPAD_BUT) == 0){
        lines &= (uint8_t)((current_keys >> 4) & 0x0F);
    }
    if((select & JOYPAD_DIR) == 0){
        lines &= (uint8_t)(current_keys & 0x0F);
    }

    return (uint8_t)(0xC0 | select | lines);
}
