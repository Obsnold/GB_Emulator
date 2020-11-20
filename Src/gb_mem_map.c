#include "gb_mem_map.h"
#include "gb_test_video_ram.h"
#include <string.h>
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


void init_mem_map(){
    memset(gb_mem_map,0,GB_MEM_SIZE);
}

void parse_header(){
    printf("0x0100 - 0x0103: Entry Point\n");
    printf("    %02X %02X %02X %02X\n", gb_mem_map[CART_START],
                                        gb_mem_map[CART_START+1],
                                        gb_mem_map[CART_START+2],
                                        gb_mem_map[CART_START+3]);
    
    printf("0x0104 - 0x0133: Nintendo Logo\n");
    printf("    ");
    for(int i = CART_LOGO; i < CART_TITLE ; i++){
        printf("%02X ", gb_mem_map[i]);
    }
    printf("\n");

    printf("0x0134 - 0x0143: Title\n");
    printf("    ");
    for(int i = CART_TITLE; i < CART_LIC_NEW_HIGH ; i++){
        printf("%02X ", gb_mem_map[i]);
    }
    printf("\n");
    printf("    ");
    for(int i = CART_TITLE; i < CART_LIC_NEW_HIGH ; i++){
        printf("%c" , gb_mem_map[i]);
    }
    printf("\n");

    printf("0x0144 - 0x0145: New Licensee Code\n");
    printf("    %02X %02X\n", gb_mem_map[CART_LIC_NEW_HIGH],
                                gb_mem_map[CART_LIC_NEW_LOW]);
    printf("0x0146: SGB Flag\n");
    printf("    %02X\n", gb_mem_map[CART_SGB_TYPE]);

    printf("0x0147: Cartridge Type\n");
    printf("    %02X\n", gb_mem_map[CART_TYPE]);
    
    printf("0x0148: Rom Size\n");
    printf("    %02X\n", gb_mem_map[CART_ROM_SIZE]);
    
    printf("0x0149: Ram Size\n");
    printf("    %02X\n", gb_mem_map[CART_RAM_SIZE]);
    
    printf("0x014A: Destination Code\n");
    printf("    %02X\n", gb_mem_map[CART_REGION]);
    
    printf("0x014B: Old Licensee Code\n");
    printf("    %02X\n", gb_mem_map[CART_LIC_OLD]);
    
    printf("0x014C: Mask ROM Version Number\n");
    printf("    %02X\n", gb_mem_map[CART_ROM_VER]);
    
    printf("0x014D: Header Checksum\n");
    printf("    %02X\n", gb_mem_map[CART_COMP_CHECK]);
    
    printf("0x014E - 0x014F: Global Checksum\n");
    printf("    %02X %02X\n", gb_mem_map[CART_CHECK_SUM_1],
                                gb_mem_map[CART_CHECK_SUM_2]);
}

/*
uint8_t get_mem_map_8(uint16_t reg){
    uint8_t data = 0;
    if(reg >= CART_ROM_0 && reg < CART_ROM_1 ){
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
        data = 0;
    } else if(reg >= IO_PORTS && reg < ZERO_PAGE){
        // IO_PORTS
        // lots of stuff going on here 
        // for now just return data
        data = gb_mem_map[reg];

    } else if(reg >= ZERO_PAGE && reg < INTERRUPT_EN){
        // Zero page
        // basically just fancy ram
        data = gb_mem_map[reg];

    } if(reg == INTERRUPT_EN){
        // Interrupt enable
        data = gb_mem_map[reg];

    } 


    return data;
}

bool set_mem_map_8(uint16_t reg, uint8_t data){

}*/