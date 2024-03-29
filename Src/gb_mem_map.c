#include "gb_mem_map.h"
#include "debug_print.h"
#include "gb_cpu.h"
#include "gb_cart.h"
#include <string.h>

#define DEBUG

#ifdef DEBUG
#define DEBUG_//PRINT(fmt, args...)   //PRINT(fmt, ## args)
#else
#define DEBUG_//PRINT(fmt, args...)
#endif

uint8_t gb_mem_map[GB_MEM_SIZE];

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
    memset(gb_mem_map,0,GB_MEM_SIZE);
#if 0 //load boot rom
    memcpy(gb_mem_map,gb_bootrom, 0x100);
#else // set up memory as if bot rom had run
    CPU_REG.AF=0x01B0;
    CPU_REG.BC=0x0013;
    CPU_REG.DE=0x00D8;
    CPU_REG.HL=0x014D;
    CPU_REG.SP=0xFFFE;
    CPU_REG.PC=0x0100;
    gb_mem_map[0xFF05] = 0x00   ;// TIMA
    gb_mem_map[0xFF06] = 0x00   ;// TMA
    gb_mem_map[0xFF07] = 0x00   ;// TAC
    gb_mem_map[0xFF10] = 0x80   ;// NR10
    gb_mem_map[0xFF11] = 0xBF   ;// NR11
    gb_mem_map[0xFF12] = 0xF3   ;// NR12
    gb_mem_map[0xFF14] = 0xBF   ;// NR14
    gb_mem_map[0xFF16] = 0x3F   ;// NR21
    gb_mem_map[0xFF17] = 0x00   ;// NR22
    gb_mem_map[0xFF19] = 0xBF   ;// NR24
    gb_mem_map[0xFF1A] = 0x7F   ;// NR30
    gb_mem_map[0xFF1B] = 0xFF   ;// NR31
    gb_mem_map[0xFF1C] = 0x9F   ;// NR32
    gb_mem_map[0xFF1E] = 0xBF   ;// NR34
    gb_mem_map[0xFF20] = 0xFF   ;// NR41
    gb_mem_map[0xFF21] = 0x00   ;// NR42
    gb_mem_map[0xFF22] = 0x00   ;// NR43
    gb_mem_map[0xFF23] = 0xBF   ;// NR44
    gb_mem_map[0xFF24] = 0x77   ;// NR50
    gb_mem_map[0xFF25] = 0xF3   ;// NR51
    gb_mem_map[0xFF26] = 0xF1   ;// NR52
    gb_mem_map[0xFF40] = 0x91   ;// LCDC
    gb_mem_map[0xFF42] = 0x00   ;// SCY
    gb_mem_map[0xFF43] = 0x00   ;// SCX
    gb_mem_map[0xFF45] = 0x00   ;// LYC
    gb_mem_map[0xFF47] = 0xFC   ;// BGP
    gb_mem_map[0xFF48] = 0xFF   ;// OBP0
    gb_mem_map[0xFF49] = 0xFF   ;// OBP1
    gb_mem_map[0xFF4A] = 0x00   ;// WY
    gb_mem_map[0xFF4B] = 0x00   ;// WX
    gb_mem_map[0xFFFF] = 0x00   ;// IE
#endif
}

// the following functions should only be used by cpu/opcode instructions
uint8_t op_get_mem_map_8(uint16_t reg){
   //PRINT("op_get_mem_map_8\n");
    uint8_t data = 0;
    if(reg < TILE_RAM_0 ){
        // CART ROM BANKS
        data = gb_cart_read_rom(reg);
    } else if(reg >= TILE_RAM_0 && reg < CART_RAM){
        // VRAM
        //need to check if VRAM is accessible
        if((gb_mem_map[LCD_STAT] & LCD_STAT_MODE) != LCD_STAT_MODE_PIXEL_TRANS)
            data = gb_mem_map[reg];

    } else if(reg >= CART_RAM && reg < GB_RAM_1){
        // CART ram if available
        data = gb_cart_read_ram(reg);

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
        data = gb_mem_map[reg];

    } else if(reg >= ZERO_PAGE && reg < INTERRUPT_EN){
        // Zero page
        // basically just fancy ram
        data = gb_mem_map[reg];

    } if(reg == INTERRUPT_EN){
        // Interrupt enable
        data = gb_mem_map[reg];
    } 
    //DEBUG_//PRINT("set_mem_map_8, reg=%04x, data=%02x\n",reg,data);
    return data;
}


bool op_set_mem_map_8(uint16_t reg, uint8_t data){
    //PRINT("op_set_mem_map_8\n");
    uint8_t lData = get_mem_map_8(reg);
    if(reg < TILE_RAM_0 ){
        // CART ROM BANKS
        lData = data;
    } else if(reg >= TILE_RAM_0 && reg < CART_RAM){
        // VRAM
        //if((gb_mem_map[LCD_STAT] & LCD_STAT_MODE) != LCD_STAT_MODE_PIXEL_TRANS)
        lData = data;
    } else if(reg >= CART_RAM && reg < GB_RAM_1){
        // CART ram if available
        lData = data;
    } else if(reg >= GB_RAM_1 && reg < GB_RAM_2){
        // Game boy work ram 1
        lData = data;
    } else if(reg >= GB_RAM_2 && reg < ECHO_RAM){
        // Game boy work ram 2
        // switchable in GB color
        lData = data;
    } else if(reg >= ECHO_RAM && reg < OAM_TABLE){
        // echo ram 
        // mirrors work ram 1 and 2
        // nintendo says not to use 
        //lData = data;
    } else if(reg >= OAM_TABLE && reg < NA_MEM){
        // OAM TAble
        // need to check if OAM is accesible
        uint8_t ppu_mode = get_mem_map_8(LCD_STAT) & LCD_STAT_MODE;
        if(ppu_mode != LCD_STAT_MODE_PIXEL_TRANS && ppu_mode != LCD_STAT_MODE_OAM)
            lData = data;
        
    } else if(reg >= NA_MEM && reg < IO_PORTS){
        // NA MEMORY
        // unusable memory for now just write data 0
        // should cause bug if accessed when OAM in not accessible
        lData = 0;
    } else if(reg >= IO_PORTS && reg < ZERO_PAGE){
        // IO_PORTS
        // lots of stuff going on here 
        // for now just write data
        lData = data;
    } else if(reg >= ZERO_PAGE && reg < INTERRUPT_EN){
        // Zero page
        // basically just fancy ram
        lData = data;
    } if(reg == INTERRUPT_EN){
        // Interrupt enable
        lData = data;
    }
    set_mem_map_8(reg,lData);
    return true;
}

// following functions are for system/io_ports --------------------------------------------------
uint8_t get_mem_map_8(uint16_t reg){
    uint8_t data = 0;
    if(reg < TILE_RAM_0 ){
        data = gb_cart_read_rom(reg);
    } else if(reg >= CART_RAM && reg < GB_RAM_1){
        // CART ram if available
        data = gb_cart_read_ram(reg);
    } else {
        data = gb_mem_map[reg];
    }
    return data;
}

void set_mem_map_8(uint16_t reg, uint8_t data){
    //PRINT("set_mem_map_8\n");
    if(reg < TILE_RAM_0 ){

        gb_cart_write_rom(reg,data);
    } else if(reg >= CART_RAM && reg < GB_RAM_1){
        // CART ram if available
        gb_cart_write_ram(reg,data);
    } else {
        gb_mem_map[reg] = data;
    }
}

uint8_t get_mem_map_bit(uint16_t reg, uint8_t data){
    //PRINT("get_mem_map_bit\n");
    return get_mem_map_8(reg) & data;
}

void set_mem_map_bit(uint16_t reg, uint8_t data){
   //PRINT("set_mem_map_bit\n");
    gb_mem_map[reg] |= data;
}

void clear_mem_map_bit(uint16_t reg, uint8_t data){
   //PRINT("clear_mem_map_bit\n");
    gb_mem_map[reg] &= ~data;
}

uint16_t get_mem_map_16(uint16_t reg){
    return op_get_mem_map_16(reg);
}

bool set_mem_map_16(uint16_t reg, uint16_t data){
    return op_set_mem_map_16(reg,data);
}

uint16_t op_get_mem_map_16(uint16_t reg){
    return op_get_mem_map_8(reg) + (op_get_mem_map_8(reg+1)<<8);
}

bool op_set_mem_map_16(uint16_t reg, uint16_t data){
    op_set_mem_map_8(reg,(uint8_t)(data&0xFF));
    op_set_mem_map_8(reg+1,(uint8_t)(data>>8));
    return true;
}

