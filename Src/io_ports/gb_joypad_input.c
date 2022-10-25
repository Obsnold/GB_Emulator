#include "gb_joypad_input.h"
#include "gb_mem_map.h"

uint8_t previous_keys = 0;

void gb_input(uint8_t keys){
    //need to check for key interrupt
    if(keys != previous_keys){
        set_mem_map_bit(INTERRUPT_FLAGS,INTERRUPT_JOYPAD);
    }

    if(get_mem_map_bit(JOYPAD_INPUT, JOYPAD_BUT)==0){
        set_mem_map_8(JOYPAD_INPUT, 0xF0 | (keys >> 4));
    } else if(get_mem_map_bit(JOYPAD_INPUT, JOYPAD_DIR)==0){
        set_mem_map_8(JOYPAD_INPUT, 0xF0 | (keys & 0x0F));
    } 
    previous_keys = keys;
}