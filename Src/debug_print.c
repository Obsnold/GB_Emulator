#include "debug_print.h"
#include "gb_mem_map.h"
#include "gb_cpu.h"
#include "gb_opcodes.h"


void print_cpu_reg(){
    //printf("BB-CC-DD-EE-HH-LL-AA-FF-#PC#-#SP#\n");
    printf("%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%04x-%04x\n",
    CPU_REG.B,CPU_REG.C,CPU_REG.D,CPU_REG.E,CPU_REG.H,CPU_REG.L,CPU_REG.A,CPU_REG.F,CPU_REG.PC,CPU_REG.SP);
}

void print_cart_header(){
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

void print_opcode_table(){
    printf("OP - LN - CY : DESC\n");
    for(int i = 0; i < OPCODE_TABLE_SIZE; i++){
        printf("%02X - %02X - %02X : %s\n",i,opcode_table[i].length,opcode_table[i].cycles, opcode_table[i].description);
    }
}

void print_opcode(){
    uint8_t opcode = gb_mem_map[CPU_REG.PC];
    printf("%04x, %02x, %s", CPU_REG.PC, opcode_table[opcode].length, opcode_table[opcode].description);
    printf(" [ %02x ", opcode);
    for(int i = 1; i < opcode_table[opcode].length;i++){
        printf("%02x ",gb_mem_map[CPU_REG.PC+i]);
    }
    printf("]\n");
}

void print_memory(uint16_t start_pos, uint16_t end_pos){
    for (int i = start_pos; i < end_pos; i++){
        if((i - start_pos)%16 ==0){
            printf("    ");
        }
        if((i - start_pos)%32 ==0){
            printf("\n%04x: ", i);
        }
        printf("%02x ", gb_mem_map[i]);
    }
    printf("\n");
}