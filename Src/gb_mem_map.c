#include "gb_mem_map.h"

uint8_t gb_mem_map[GB_MEM_SIZE];
uint8_t gb_reg_map[GB_REG_SIZE];
/*uint8_t bc_pointer;
uint8_t de_pointer;
uint8_t hl_pointer;
uint8_t u8_pointer;
uint8_t u16_pointer_1;
uint8_t u16_pointer_2;
uint8_t s8_pointer;*/

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
/*
void update_pointers(){
    bc_pointer = &(gb_mem_map[get_16_from_8(GB_REG_BC)]);
    de_pointer = &(gb_mem_map[get_16_from_8(GB_REG_DE)]);
    hl_pointer = &(gb_mem_map[get_16_from_8(GB_REG_HL)]);
}*/

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