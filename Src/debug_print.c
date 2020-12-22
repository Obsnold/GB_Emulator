#include "debug_print.h"
#include "gb_mem_map.h"
#include "gb_cpu.h"
#include "gb_opcodes.h"
#include "gb_ppu.h"

#define DEBUG

void print_cpu_reg(){
#ifdef DEBUG
    //PRINT("BB-CC-DD-EE-HH-LL-AA-FF-#PC#-#SP#\n");
    PRINT("%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%04x-%04x  -  ",
    CPU_REG.B,CPU_REG.C,CPU_REG.D,CPU_REG.E,CPU_REG.H,CPU_REG.L,CPU_REG.A,CPU_REG.F,CPU_REG.PC,CPU_REG.SP);
#endif
}

void print_indirect_reg(){
#ifdef DEBUG
    //PRINT("BB-CC-DD-EE-HH-LL-AA-FF-#PC#-#SP#\n");
    PRINT("%02x-%02x-%02x-%02x-%02x-%02x  -  ",
    gb_mem_map[CPU_REG.BC],gb_mem_map[CPU_REG.DE],gb_mem_map[CPU_REG.HL],gb_mem_map[CPU_REG.AF],gb_mem_map[CPU_REG.PC],gb_mem_map[CPU_REG.SP]);
#endif
}

void print_cart_header(){
#ifdef DEBUG
    PRINT("0x0100 - 0x0103: Entry Point\n");
    PRINT("    %02X %02X %02X %02X\n", gb_mem_map[CART_START],
                                        gb_mem_map[CART_START+1],
                                        gb_mem_map[CART_START+2],
                                        gb_mem_map[CART_START+3]);
    
    PRINT("0x0104 - 0x0133: Nintendo Logo\n");
    PRINT("    ");
    for(int i = CART_LOGO; i < CART_TITLE ; i++){
        PRINT("%02X ", gb_mem_map[i]);
    }
    PRINT("\n");

    PRINT("0x0134 - 0x0143: Title\n");
    PRINT("    ");
    for(int i = CART_TITLE; i < CART_LIC_NEW_HIGH ; i++){
        PRINT("%02X ", gb_mem_map[i]);
    }
    PRINT("\n");
    PRINT("    ");
    for(int i = CART_TITLE; i < CART_LIC_NEW_HIGH ; i++){
        PRINT("%c" , gb_mem_map[i]);
    }
    PRINT("\n");

    PRINT("0x0144 - 0x0145: New Licensee Code\n");
    PRINT("    %02X %02X\n", gb_mem_map[CART_LIC_NEW_HIGH],
                                gb_mem_map[CART_LIC_NEW_LOW]);
    PRINT("0x0146: SGB Flag\n");
    PRINT("    %02X\n", gb_mem_map[CART_SGB_TYPE]);

    PRINT("0x0147: Cartridge Type\n");
    PRINT("    %02X\n", gb_mem_map[CART_TYPE]);
    
    PRINT("0x0148: Rom Size\n");
    PRINT("    %02X\n", gb_mem_map[CART_ROM_SIZE]);
    
    PRINT("0x0149: Ram Size\n");
    PRINT("    %02X\n", gb_mem_map[CART_RAM_SIZE]);
    
    PRINT("0x014A: Destination Code\n");
    PRINT("    %02X\n", gb_mem_map[CART_REGION]);
    
    PRINT("0x014B: Old Licensee Code\n");
    PRINT("    %02X\n", gb_mem_map[CART_LIC_OLD]);
    
    PRINT("0x014C: Mask ROM Version Number\n");
    PRINT("    %02X\n", gb_mem_map[CART_ROM_VER]);
    
    PRINT("0x014D: Header Checksum\n");
    PRINT("    %02X\n", gb_mem_map[CART_COMP_CHECK]);
    
    PRINT("0x014E - 0x014F: Global Checksum\n");
    PRINT("    %02X %02X\n", gb_mem_map[CART_CHECK_SUM_1],
                                gb_mem_map[CART_CHECK_SUM_2]);
#endif
}

void print_opcode_table(){
#ifdef DEBUG
    PRINT("OP - LN - CY : DESC\n");
    for(int i = 0; i < OPCODE_TABLE_SIZE; i++){
        PRINT("%02X - %02X - %02X : %s\n",i,opcode_table[i].length,opcode_table[i].cycles, opcode_table[i].description);
    }
#endif
}

void print_opcode(){
#ifdef DEBUG
    uint8_t opcode = gb_mem_map[CPU_REG.PC];
    PRINT("%04x, %02x, %s", CPU_REG.PC, opcode_table[opcode].length, opcode_table[opcode].description);
    PRINT(" [ %02x ", opcode);
    for(int i = 1; i < opcode_table[opcode].length;i++){
        PRINT("%02x ",gb_mem_map[CPU_REG.PC+i]);
    }
    PRINT("]\n");
#endif
}

void print_lcd(){
#ifdef DEBUG
    PRINT("=================LCD=================\n");

    PRINT("LCD_CTRL_ENABLE=%02x \n",gb_mem_map[LCD_CTRL]&LCD_CTRL_ENABLE);
    PRINT("LCD_CTRL_WINDOW_MAP_SELECT=%02x \n",gb_mem_map[LCD_CTRL]&LCD_CTRL_WINDOW_MAP_SELECT);
    PRINT("LCD_CTRL_WINDOW_ENABLE=%02x \n",gb_mem_map[LCD_CTRL]&LCD_CTRL_WINDOW_ENABLE);
    PRINT("LCD_CTRL_BG_W_TILE_SELECT=%02x \n",gb_mem_map[LCD_CTRL]&LCD_CTRL_BG_W_TILE_SELECT);
    PRINT("LCD_CTRL_BG_MAP_SELECT=%02x \n",gb_mem_map[LCD_CTRL]&LCD_CTRL_BG_MAP_SELECT);
    PRINT("LCD_CTRL_OBJ_SIZE=%02x \n",gb_mem_map[LCD_CTRL]&LCD_CTRL_OBJ_SIZE);
    PRINT("LCD_CTRL_OBJ_ENABLE=%02x \n",gb_mem_map[LCD_CTRL]&LCD_CTRL_OBJ_ENABLE);
    PRINT("LCD_CTRL_BG_PRIORITY=%02x \n",gb_mem_map[LCD_CTRL]&LCD_CTRL_BG_PRIORITY);

    PRINT("LCD_STAT_LCY_INTR_EN=%02x \n",gb_mem_map[LCD_STAT]&LCD_STAT_LCY_INTR_EN);
    PRINT("LCD_STAT_OAM_INTR_EN=%02x \n",gb_mem_map[LCD_STAT]&LCD_STAT_OAM_INTR_EN);
    PRINT("LCD_STAT_VBLNK_INTR_EN=%02x \n",gb_mem_map[LCD_STAT]&LCD_STAT_VBLNK_INTR_EN);
    PRINT("LCD_STAT_HBLNK_INTR_EN=%02x \n",gb_mem_map[LCD_STAT]&LCD_STAT_COINC_FLAG);
    PRINT("LCD_STAT_COINC_FLAG=%02x \n",gb_mem_map[LCD_STAT]&LCD_STAT_COINC_FLAG);
    PRINT("LCD_STAT_MODE=%02x \n",gb_mem_map[LCD_STAT]&LCD_STAT_MODE);

    PRINT("LCD_SCY=%02x \n",gb_mem_map[LCD_SCY]);
    PRINT("LCD_SCX=%02x \n",gb_mem_map[LCD_SCX]);
    PRINT("LCD_LY=%02x \n",gb_mem_map[LCD_LY]);
    PRINT("LCD_LYC=%02x \n",gb_mem_map[LCD_LYC]);
    PRINT("LCD_DMA=%02x \n",gb_mem_map[LCD_DMA]);
    PRINT("LCD_BGP=%02x \n",gb_mem_map[LCD_BGP]);
    PRINT("LCD_OBP0=%02x \n",gb_mem_map[LCD_OBP0]);
    PRINT("LCD_OBP1=%02x \n",gb_mem_map[LCD_OBP1]);
    PRINT("LCD_WY=%02x \n",gb_mem_map[LCD_WY]);
    PRINT("LCD_WX=%02x \n",gb_mem_map[LCD_WX]);
#endif
}

void print_memory(uint16_t start_pos, uint16_t end_pos){
#ifdef DEBUG
    for (int i = start_pos; i < end_pos; i++){
        if((i - start_pos)%16 ==0){
            PRINT("    ");
        }
        if((i - start_pos)%32 ==0){
            PRINT("\n%04x: ", i);
        }
        PRINT("%02x ", gb_mem_map[i]);
    }
    PRINT("\n");
#endif
}

void print_display_buffer(){
#ifdef DEBUG
    for(int i =0; i <GB_SCREEN_HEIGHT;i++){
        for(int j =0; j < GB_SCREEN_WIDTH; j++){
            int val = 0;
            switch(gb_display[j][i]){
                case PAL_WHITE:
                    val = 0;
                break;
                case PAL_L_GRAY:
                    val = 1;
                break;
                case PAL_D_GRAY:
                    val = 2;
                break;
                case PAL_BLACK:
                    val = 3;
                break;
            }
            PRINT("%d", val);
        }
        PRINT("\n");
    }
#endif
}