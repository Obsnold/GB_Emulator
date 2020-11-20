#include "gb_mem_map.h"

uint8_t previous_keys = 0;

void gb_input(uint8_t keys){
    //need to check for key interrupt
    //if(keys != previous_keys)
    
    if(gb_mem_map[JOYPAD_INPUT] & JOYPAD_BUT){
        gb_mem_map[JOYPAD_INPUT] |= (keys & 0x0F);
    }
    if(gb_mem_map[JOYPAD_INPUT] & JOYPAD_DIR){
        gb_mem_map[JOYPAD_INPUT] |= (keys >> 4);
    }
    previous_keys = keys;
}