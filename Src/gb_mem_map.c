#include "gb_mem_map.h"
#include "gb_test_video_ram.h"
#include "debug_print.h"
#include <string.h>

#ifdef DEBUG
#define DEBUG_PRINT(fmt, args...)    PRINT(fmt, ## args)
#else
#define DEBUG_PRINT(fmt, args...)
#endif

uint8_t gb_mem_map[GB_MEM_SIZE];

//const values
uint8_t addr_38H = 0x38;
uint8_t addr_30H = 0x30;
uint8_t addr_28H = 0x28;
uint8_t addr_20H = 0x20;
uint8_t addr_18H = 0x18;
uint8_t addr_10H = 0x10;
uint8_t addr_08H = 0x08;
uint8_t addr_00H = 0x00;

uint8_t gb_bootrom[0x100] = { 
    0x31, 0xfe, 0xff, 0xaf, 0x21, 0xff, 0x9f, 0x32, 0xcb, 0x7c, 0x20, 0xfb, 0x21, 0x26, 0xff, 0x0e,
    0x11, 0x3e, 0x80, 0x32, 0xe2, 0x0c, 0x3e, 0xf3, 0xe2, 0x32, 0x3e, 0x77, 0x77, 0x3e, 0xfc, 0xe0,
    0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1a, 0xcd, 0x95, 0x00, 0xcd, 0x96, 0x00, 0x13, 0x7b,
    0xfe, 0x34, 0x20, 0xf3, 0x11, 0xd8, 0x00, 0x06, 0x08, 0x1a, 0x13, 0x22, 0x23, 0x05, 0x20, 0xf9,
    0x3e, 0x19, 0xea, 0x10, 0x99, 0x21, 0x2f, 0x99, 0x0e, 0x0c, 0x3d, 0x28, 0x08, 0x32, 0x0d, 0x20,
    0xf9, 0x2e, 0x0f, 0x18, 0xf3, 0x67, 0x3e, 0x64, 0x57, 0xe0, 0x42, 0x3e, 0x91, 0xe0, 0x40, 0x04,
    0x1e, 0x02, 0x0e, 0x0c, 0xf0, 0x44, 0xfe, 0x90, 0x20, 0xfa, 0x0d, 0x20, 0xf7, 0x1d, 0x20, 0xf2,
    0x0e, 0x13, 0x24, 0x7c, 0x1e, 0x83, 0xfe, 0x62, 0x28, 0x06, 0x1e, 0xc1, 0xfe, 0x64, 0x20, 0x06,
    0x7b, 0xe2, 0x0c, 0x3e, 0x87, 0xe2, 0xf0, 0x42, 0x90, 0xe0, 0x42, 0x15, 0x20, 0xd2, 0x05, 0x20,
    0x4f, 0x16, 0x20, 0x18, 0xcb, 0x4f, 0x06, 0x04, 0xc5, 0xcb, 0x11, 0x17, 0xc1, 0xcb, 0x11, 0x17,
    0x05, 0x20, 0xf5, 0x22, 0x23, 0x22, 0x23, 0xc9, 0xce, 0xed, 0x66, 0x66, 0xcc, 0x0d, 0x00, 0x0b,
    0x03, 0x73, 0x00, 0x83, 0x00, 0x0c, 0x00, 0x0d, 0x00, 0x08, 0x11, 0x1f, 0x88, 0x89, 0x00, 0x0e,
    0xdc, 0xcc, 0x6e, 0xe6, 0xdd, 0xdd, 0xd9, 0x99, 0xbb, 0xbb, 0x67, 0x63, 0x6e, 0x0e, 0xec, 0xcc,
    0xdd, 0xdc, 0x99, 0x9f, 0xbb, 0xb9, 0x33, 0x3e, 0x3c, 0x42, 0xb9, 0xa5, 0xb9, 0xa5, 0x42, 0x3c,
    0x21, 0x04, 0x01, 0x11, 0xa8, 0x00, 0x1a, 0x13, 0xbe, 0x20, 0xfe, 0x23, 0x7d, 0xfe, 0x34, 0x20,
    0xf5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xfb, 0x86, 0x20, 0xfe, 0x3e, 0x01, 0xe0, 0x50};



void init_mem_map(){
    //memset(gb_mem_map,0,GB_MEM_SIZE);
    memcpy(gb_mem_map,gb_bootrom, 0x100);
}

// the following functions should only be used by cpu instructions
// the system itself should directly access memory
uint8_t get_mem_map_8(uint16_t reg){
    uint8_t data = 0;
    if(reg < CART_ROM_1 ){
        // fixed rom bank
        data = gb_mem_map[reg];

    } else if(reg >= CART_ROM_1 && reg < VRAM ){
        // switchable rom bank
        data = gb_mem_map[reg];

    } else if(reg >= VRAM && reg < CART_RAM){
        // VRAM
        //need to check if VRAM is accessible
        if((gb_mem_map[LCD_STAT] & LCD_STAT_MODE) != LCD_STAT_MODE_PIXEL_TRANS)
            data = gb_mem_map[reg];

    } else if(reg >= CART_RAM && reg < GB_RAM_1){
        // CART ram if available
        data = gb_mem_map[reg];

    } else if(reg >= GB_RAM_1 && reg < GB_RAM_2){
        // Game boy work ram 1
        data = gb_mem_map[reg];

    } else if(reg >= GB_RAM_2 && reg < ECHO_RAM){
        // Game boy work ram 2
        // switchable in GB color
        data = gb_mem_map[reg];

    } else if(reg >= ECHO_RAM && reg < OAM_TABLE){
        // echo ram 
        // mirrors work ram 1 and 2
        // nintendo says not to use 
        // lets just return whatever is in the work ram
        data = gb_mem_map[reg - 0x2000];

    } else if(reg >= OAM_TABLE && reg < NA_MEM){
        // OAM TAble
        // need to check if OAM is accesible
        uint8_t ppu_mode = gb_mem_map[LCD_STAT] & LCD_STAT_MODE;
        if(ppu_mode != LCD_STAT_MODE_PIXEL_TRANS && ppu_mode != LCD_STAT_MODE_OAM)
            data = gb_mem_map[reg];
    } else if(reg >= NA_MEM && reg < IO_PORTS){
        // NA MEMORY
        // unusable memory for now return 0
        // should cause bug if accessed when OAM in not accessible
        data = 0;
    } else if(reg >= IO_PORTS && reg < ZERO_PAGE){
        // IO_PORTS
        // lots of stuff going on here 
        // for now just return data
        DEBUG_PRINT("IO_PORTS\n");
        data = gb_mem_map[reg];

    } else if(reg >= ZERO_PAGE && reg < INTERRUPT_EN){
        // Zero page
        // basically just fancy ram
        data = gb_mem_map[reg];

    } if(reg == INTERRUPT_EN){
        // Interrupt enable
        DEBUG_PRINT("INTERRUPT_EN\n");
        data = gb_mem_map[reg];

    } 
    return data;
}

uint8_t* get_mem_map_pointer(uint16_t reg){
    return &gb_mem_map[reg];
}

bool set_mem_map_8(uint16_t reg, uint8_t data){
    gb_mem_map[reg] = data;
    return true;
}

uint16_t get_mem_map_16(uint16_t reg){
    uint16_t temp = get_mem_map_8(reg);
    return temp + (get_mem_map_8(reg+1)<<8);
}

bool set_mem_map_16(uint16_t reg, uint16_t data){
    set_mem_map_8(reg+1,(uint8_t)(data>>8));
    set_mem_map_8(reg,(uint8_t)(data&0xFF));
    return true;
}