#include "gb_joypad_input.h"
#include "gb_mem_map.h"

uint8_t previous_keys = 0;

void gb_input(uint8_t keys){
    //need to check for key interrupt
    if(keys != previous_keys){
        SET_MEM_MAP(INTERRUPT_FLAGS,INTERRUPT_JOYPAD);
    }

    if(GET_MEM_MAP(JOYPAD_INPUT, JOYPAD_BUT)==0){
        gb_mem_map[JOYPAD_INPUT] = 0xF0 | (keys >> 4);
    } else if(GET_MEM_MAP(JOYPAD_INPUT, JOYPAD_DIR)==0){
        gb_mem_map[JOYPAD_INPUT] = 0xF0 | (keys & 0x0F);
    } 
    previous_keys = keys;
}