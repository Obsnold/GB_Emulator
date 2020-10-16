#include "gb_mem_map.h"
#include "gb_test_video_ram.h"
#include <string.h>
uint8_t gb_mem_map[GB_MEM_SIZE];
uint8_t gb_reg_map[GB_REG_SIZE];

//const values
uint8_t addr_38H = 0x38;
uint8_t addr_30H = 0x30;
uint8_t addr_28H = 0x28;
uint8_t addr_20H = 0x20;
uint8_t addr_18H = 0x18;
uint8_t addr_10H = 0x10;
uint8_t addr_08H = 0x08;
uint8_t addr_00H = 0x00;


uint16_t get_16_from_8(uint8_t* source){
    uint16_t temp = *source;
    return (temp << 8) + (*(source+1));
}

uint8_t get_16_low(uint16_t* source){
    return  (*source & 0x0F);
}

uint8_t get_16_high(uint16_t* source){
    return (*source >> 8);
}

void init_mem_map(){
    memset(gb_mem_map,0,GB_MEM_SIZE);

    gb_mem_map[VRAM_BLOCK_0+0]= 0xFF;
    gb_mem_map[VRAM_BLOCK_0+1]= 0xFF;
    gb_mem_map[VRAM_BLOCK_0+2]= 0x81;
    gb_mem_map[VRAM_BLOCK_0+3]= 0x81;
    gb_mem_map[VRAM_BLOCK_0+4]= 0x81;
    gb_mem_map[VRAM_BLOCK_0+5]= 0x81;
    gb_mem_map[VRAM_BLOCK_0+6]= 0x81;
    gb_mem_map[VRAM_BLOCK_0+7]= 0x81;
    gb_mem_map[VRAM_BLOCK_0+8]= 0x81;
    gb_mem_map[VRAM_BLOCK_0+9]= 0x81;
    gb_mem_map[VRAM_BLOCK_0+10]= 0x81;
    gb_mem_map[VRAM_BLOCK_0+11]= 0x81;
    gb_mem_map[VRAM_BLOCK_0+12]= 0x81;
    gb_mem_map[VRAM_BLOCK_0+13]= 0x81;
    gb_mem_map[VRAM_BLOCK_0+14]= 0xFF;
    gb_mem_map[VRAM_BLOCK_0+15]= 0xFF;
    
    gb_mem_map[VRAM_BLOCK_0+16]= 0x81;
    gb_mem_map[VRAM_BLOCK_0+17]= 0x81;
    gb_mem_map[VRAM_BLOCK_0+18]= 0x42;
    gb_mem_map[VRAM_BLOCK_0+19]= 0x42;
    gb_mem_map[VRAM_BLOCK_0+20]= 0x24;
    gb_mem_map[VRAM_BLOCK_0+21]= 0x24;
    gb_mem_map[VRAM_BLOCK_0+22]= 0x18;
    gb_mem_map[VRAM_BLOCK_0+23]= 0x18;
    gb_mem_map[VRAM_BLOCK_0+24]= 0x18;
    gb_mem_map[VRAM_BLOCK_0+25]= 0x18;
    gb_mem_map[VRAM_BLOCK_0+26]= 0x24;
    gb_mem_map[VRAM_BLOCK_0+27]= 0x24;
    gb_mem_map[VRAM_BLOCK_0+28]= 0x42;
    gb_mem_map[VRAM_BLOCK_0+29]= 0x42;
    gb_mem_map[VRAM_BLOCK_0+30]= 0x81;
    gb_mem_map[VRAM_BLOCK_0+31]= 0x81;

    /*
    for(int j=0;j<128;j++){
        for(int i = 0; i<16;i++){
            gb_mem_map[VRAM_BLOCK_0+i+(j*16)] = gb_test_tile_map[i];
            //gb_mem_map[VRAM_BLOCK_1+i+(j*32)] = gb_test_tile_map[i];
            //gb_mem_map[VRAM_BLOCK_2+i+(j*32)] = gb_test_tile_map[i];
        }
    }*/

    for(int i =0 ;i < 1024; i+=2){
        gb_mem_map[BG_MAP_1+i] = gb_test_map[i];
        gb_mem_map[BG_MAP_1+i+1] = gb_test_map[i+1];
    }
    SET_MEM_MAP(LCD_STAT,LCD_STAT_MODE_VBLNK);
    gb_mem_map[LCD_SCY] = 250;
    gb_mem_map[LCD_SCX] = 250;

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