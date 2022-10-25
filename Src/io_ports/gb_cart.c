#include "gb_cart.h"
#include "gb_mem_map.h"
#include "gb_load.h"
#include "debug_print.h"
#include "gb_common.h"
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG
#define DEBUG_PRINT(fmt, args...)    PRINT(fmt, ## args)
#else
#define DEBUG_PRINT(fmt, args...)
#endif

// No MBC (32 KiB ROM only)
// MBC1 (max 2MByte ROM and/or 32 KiB RAM)
// MBC2 (max 256 KiB ROM and 512x4 bits RAM)
// MBC3 (max 2MByte ROM and/or 32KByte RAM and Timer)
// MBC5 It can map up to 64 Mbits (8 MiB) of ROM. similar to MBC1 support GBC double speed
// MBC6 - some weird chip used in only one game. seems to download minigames
// MBC7 - 2 axis accelerometer
// MMM01 - multi game cart
// M161 - anohter multi game cart
// HuC1 - hudson infrared cart
// HUC-3 - more advanced version of the HUC1 but with RTC and other bits



uint8_t* g_cart;
unsigned int g_cart_size = 0;

bool g_enable_ram = false;
uint8_t g_rom_bank = 0x01;
uint8_t g_ram_bank = 0x00;
uint8_t g_banking_mode = 0x00;
uint16_t g_number_of_rom_banks;
uint16_t g_number_of_ram_banks;
uint8_t g_ram_banks[16][CART_RAM_MB_SIZE];

int gb_cart_load(char* filename){
    //load cart rom data
    g_cart_size = load_raw_file(filename, &g_cart);
    if(g_cart_size <= 0){
        return -1;
    }
    PRINT("g_cart_size %u\n",g_cart_size);

    // get values from cart header
    switch(g_cart[CART_ROM_SIZE]){
        case ROM_SIZE_1_5MB:
        g_number_of_rom_banks = 96;
        break;
        case ROM_SIZE_1_2MB:
        g_number_of_rom_banks = 80;
        break;
        case ROM_SIZE_1_1MB:
        g_number_of_rom_banks = 72;
        break;
        case ROM_SIZE_8MB:
        g_number_of_rom_banks = 512;
        break;
        case ROM_SIZE_4MB:
        g_number_of_rom_banks = 256;
        break;
        case ROM_SIZE_2MB:
        g_number_of_rom_banks = 128;
        break;
        case ROM_SIZE_1MB:
        g_number_of_rom_banks = 64;
        break;
        case ROM_SIZE_512KB:
        g_number_of_rom_banks = 32;
        break;
        case ROM_SIZE_256KB:
        g_number_of_rom_banks = 16;
        break;
        case ROM_SIZE_128KB:
        g_number_of_rom_banks = 8;
        break;
        case ROM_SIZE_64KB:
        g_number_of_rom_banks = 4;
        break;
        case ROM_SIZE_32KB:
        default:
        g_number_of_rom_banks = 2;
        break;
    }

    switch(g_cart[CART_RAM_SIZE]){
        case RAM_SIZE_128KB:
        g_number_of_ram_banks = 16;
        break;
        case RAM_SIZE_64KB:
        g_number_of_ram_banks = 8;
        break;
        case RAM_SIZE_32KB:
        g_number_of_ram_banks = 4;
        break;
        case RAM_SIZE_8KB:
        g_number_of_ram_banks = 1;
        break;
        case RAM_SIZE_2KB:
        case RAM_SIZE_0:
        default:
        g_number_of_ram_banks = 0;
        break;
    }
    PRINT("g_number_of_ram_banks = %02x\n",g_number_of_ram_banks);
    PRINT("g_number_of_rom_banks = %02x\n",g_number_of_rom_banks);
    return 0;
}


void gb_cart_write_rom(uint16_t addr, uint8_t data){
    // rom banks need to check for mbc and other controler chips
    switch(g_cart[CART_TYPE]){
        case TYPE_ROM_MBC1:
        case TYPE_ROM_MBC1_RAM:
        case TYPE_ROM_MBC1_RAM_BAT:
            if(addr < MBC1_REG_RAM_EN){ // enable ram
                //PRINT("MBC1_REG_RAM_EN %02x\n",data);
                if(data == 0x00){
                    g_enable_ram = false;
                } else if (data & 0x0A){
                    g_enable_ram = true;
                }
            } else if (addr < MBC1_REG_ROM_BANK_NUM){ // switch rom bank
                //PRINT("MBC1_REG_ROM_BANK_NUM %02x\n",data);
                if(data == 0x00) {
                    g_rom_bank = 0x01;
                } else {
                    g_rom_bank = data & 0x1F;
                }
                if(g_banking_mode == 0x01){
                    g_rom_bank = g_rom_bank + (g_ram_bank << 0x05);
                }
            } else if (addr < MBC1_REG_RAM_BANK_NUM){ // switch ram bank
                //PRINT("MBC1_REG_RAM_BANK_NUM\n");
                g_ram_bank = data & 0x03;
            } else if (addr < MBC1_REG_BANK_MODE_SEL){ // select banking mode
                //PRINT("MBC1_REG_BANK_MODE_SEL\n");
                g_banking_mode = data;
            }
            break;
        case TYPE_ROM_MBC2:
        case TYPE_ROM_MBC2_BAT:
            if(addr & BIT_8){
                
            } else {
                if(addr < MBC2_REG_ROM_BANK_NUM){
                    if(data & 0x0A){
                        g_enable_ram = true;
                    } else {
                        g_enable_ram = false;
                    }
                } 
            }
            break;
        case TYPE_ROM_RAM:
        case TYPE_ROM_RAM_BAT:
        case TYPE_ROM_MMM01:
        case TYPE_ROM_MMM01_SRAM:
        case TYPE_ROM_MMM01_SRAM_BAT:
        case TYPE_ROM_MBC3_TIMER_BAT:
        case TYPE_ROM_MBC3_TIMER_RAM_BAT:
        case TYPE_ROM_MBC3:
        case TYPE_ROM_MBC3_RAM:
        case TYPE_ROM_MBC3_RAM_BAT:
        case TYPE_ROM_MBC5:
        case TYPE_ROM_MBC5_RAM:
        case TYPE_ROM_MBC5_RAM_BAT:
        case TYPE_ROM_MBC5_RUMBL:
        case TYPE_ROM_MBC5_RUMBL_SRAM:
        case TYPE_ROM_MBC5_RUMBL_SRAM_BAT:
        case TYPE_POCKET_CAM:
        case TYPE_BANDAI_TAMA5:
        case TYPE_HUDSON_HUC_3:
        case TYPE_HUDSON_HUC_1:
        case TYPE_ROM_NO_MBC:
        default:
            //do nothing cannot switch ram bank
            break;
    }
}


uint8_t gb_cart_read_rom(uint16_t addr){
    uint8_t data = 0;
    if (addr < CART_ROM_1){
        // return fixed rom
        data = g_cart[addr];
    } else {
        data = g_cart[(g_rom_bank * CART_MB_SIZE) + (addr - CART_MB_SIZE)];
    }
    return data;
}


void gb_cart_write_ram(uint16_t addr, uint8_t data){
    if (g_enable_ram = true){
        g_ram_banks[g_ram_bank][addr-CART_RAM] = data;
    }
}


uint8_t gb_cart_read_ram(uint16_t addr){
    if (g_enable_ram = true){
        return g_ram_banks[g_ram_bank][addr-CART_RAM];
    } else {
        return 0;
    }
}
