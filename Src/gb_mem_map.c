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

 uint8_t gb_bootrom[0x100] = { 
    0xfe, 0x31, 0xaf, 0xff, 0xff, 0x21, 0x32, 0x9f, 0x7c, 0xcb, 0xfb, 0x20, 0x26, 0x21, 0x0e, 0xff,
    0x3e, 0x11, 0x32, 0x80, 0x0c, 0xe2, 0xf3, 0x3e, 0x32, 0xe2, 0x77, 0x3e, 0x3e, 0x77, 0xe0, 0xfc,
    0x11, 0x47, 0x01, 0x04, 0x10, 0x21, 0x1a, 0x80, 0x95, 0xcd, 0xcd, 0x00, 0x00, 0x96, 0x7b, 0x13,
    0x34, 0xfe, 0xf3, 0x20, 0xd8, 0x11, 0x06, 0x00, 0x1a, 0x08, 0x22, 0x13, 0x05, 0x23, 0xf9, 0x20,
    0x19, 0x3e, 0x10, 0xea, 0x21, 0x99, 0x99, 0x2f, 0x0c, 0x0e, 0x28, 0x3d, 0x32, 0x08, 0x20, 0x0d,
    0x2e, 0xf9, 0x18, 0x0f, 0x67, 0xf3, 0x64, 0x3e, 0xe0, 0x57, 0x3e, 0x42, 0xe0, 0x91, 0x04, 0x40,
    0x02, 0x1e, 0x0c, 0x0e, 0x44, 0xf0, 0x90, 0xfe, 0xfa, 0x20, 0x20, 0x0d, 0x1d, 0xf7, 0xf2, 0x20,
    0x13, 0x0e, 0x7c, 0x24, 0x83, 0x1e, 0x62, 0xfe, 0x06, 0x28, 0xc1, 0x1e, 0x64, 0xfe, 0x06, 0x20,
    0xe2, 0x7b, 0x3e, 0x0c, 0xe2, 0x87, 0x42, 0xf0, 0xe0, 0x90, 0x15, 0x42, 0xd2, 0x20, 0x20, 0x05,
    0x16, 0x4f, 0x18, 0x20, 0x4f, 0xcb, 0x04, 0x06, 0xcb, 0xc5, 0x17, 0x11, 0xcb, 0xc1, 0x17, 0x11,
    0x20, 0x05, 0x22, 0xf5, 0x22, 0x23, 0xc9, 0x23, 0xed, 0xce, 0x66, 0x66, 0x0d, 0xcc, 0x0b, 0x00,
    0x73, 0x03, 0x83, 0x00, 0x0c, 0x00, 0x0d, 0x00, 0x08, 0x00, 0x1f, 0x11, 0x89, 0x88, 0x0e, 0x00,
    0xcc, 0xdc, 0xe6, 0x6e, 0xdd, 0xdd, 0x99, 0xd9, 0xbb, 0xbb, 0x63, 0x67, 0x0e, 0x6e, 0xcc, 0xec,
    0xdc, 0xdd, 0x9f, 0x99, 0xb9, 0xbb, 0x3e, 0x33, 0x42, 0x3c, 0xa5, 0xb9, 0xa5, 0xb9, 0x3c, 0x42,
    0x04, 0x21, 0x11, 0x01, 0x00, 0xa8, 0x13, 0x1a, 0x20, 0xbe, 0x23, 0xfe, 0xfe, 0x7d, 0x20, 0x34,
    0x06, 0xf5, 0x78, 0x19, 0x23, 0x86, 0x20, 0x05, 0x86, 0xfb, 0xfe, 0x20, 0x01, 0x3e, 0x50, 0xe0};

void init_mem_map(){
    memset(gb_mem_map,0,GB_MEM_SIZE);
    memcpy(gb_mem_map,gb_bootrom, 0x100);
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