#include "gb_cart.h"
#include "gb_mem_map.h"
#include "gb_load.h"
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG
#define DEBUG_PRINT(fmt, args...)    PRINT(fmt, ## args)
#else
#define DEBUG_PRINT(fmt, args...)
#endif

uint8_t* g_cart;
unsigned int g_cart_size = 0;


bool g_enable_ram = false;
uint8_t g_rom_bank = 0x01;
uint8_t g_ram_bank = 0x00;
uint8_t g_banking_mode = 0x00;
uint16_t g_number_of_rom_banks;
uint16_t g_number_of_ram_banks;
uint8_t* g_ram_banks[16];


int gb_cart_load(char* filename){
    //load cart rom data
    g_cart_size = load_raw_file(filename, &g_cart);
    if(g_cart_size <= 0){
        return -1;
    }

    //load initial rom into memory
    for(uint16_t i = 0; i < CART_MB_SIZE * 2; i++){
        set_mem_map_8(CART_ROM_0+i,g_cart[i]);
    }

    // get values from cart header
    switch(get_mem_map_8(CART_ROM_SIZE)){
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

    switch(get_mem_map_8(CART_RAM_SIZE)){
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

    //init ram memory
    for(int i = 0; i < g_number_of_ram_banks; i ++){
        g_ram_banks[i] = (uint8_t *)malloc(CART_RAM_MB_SIZE);
    }

    return 0;
}


void gb_cart_switch_rom_bank(unsigned int bank){
    if(bank == 0x00) {
        g_rom_bank = 0x01;
    } else {
        g_rom_bank = bank & 0x1F;
    }
    if(g_banking_mode == 0x01){
        g_rom_bank = g_rom_bank + (g_ram_bank << 0x05);
    }

    if(g_rom_bank < g_number_of_rom_banks){
        for(uint16_t i = 0; i < CART_MB_SIZE; i++){
            set_mem_map_8(CART_ROM_1+i,g_cart[(g_rom_bank * CART_MB_SIZE) + i]);
        }
    } else {
        DEBUG_PRINT("Error! trying to switch rom membanks");
    }
}

void gb_cart_switch_ram_bank(unsigned int bank){
    //printf("gb_cart_switch_ram_bank %d\n", bank);
    if(g_number_of_ram_banks == 0x01){
        g_ram_bank = bank & 0x03;
    } else if(bank <= g_number_of_ram_banks){
        //first save existing ram
        for(uint16_t i = 0; i < CART_MB_SIZE; i++){
            g_ram_banks[g_ram_bank-1][i] = get_mem_map_8(CART_RAM+i);
        }
        
        //now load new ram
        g_ram_bank = bank & 0x03;
        for(uint16_t i = 0; i < CART_MB_SIZE; i++){
            set_mem_map_8(CART_RAM+i,g_ram_banks[g_ram_bank-1][i]);
        }
        
    } else {
        DEBUG_PRINT("Error! trying to switch ram membanks");
    }
}

void gb_cart_set_ram_enabled(bool set){
    printf("gb_cart_set_ram_enabled %d\n", set);
    g_enable_ram = set;
}

void gb_cart_set_banking_mode(uint8_t set){
    printf("gb_cart_set_banking_mode %d\n", set);
    g_banking_mode = set;
}
