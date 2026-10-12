#include "test_support.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "gb_cart.h"
#include "gb_cpu.h"
#include "gb_mem_map.h"

static uint8_t *g_fixture_cart = NULL;

uint8_t *test_build_rom(unsigned size, uint8_t cart_type,
                        uint8_t rom_size_code, uint8_t ram_size_code)
{
    uint8_t *rom = calloc(1, size);
    unsigned bank;

    if (rom == NULL) {
        fprintf(stderr, "test_build_rom: out of memory\n");
        exit(2);
    }

    /* Mark each bank so a read can be traced back to the bank it came from. */
    for (bank = 0; bank * TEST_ROM_BANK_BYTES < size; bank++) {
        unsigned off = bank * TEST_ROM_BANK_BYTES;
        unsigned n   = size - off;
        if (n > TEST_ROM_BANK_BYTES) {
            n = TEST_ROM_BANK_BYTES;
        }
        memset(rom + off, (int)(0xA0 | (bank & 0x0F)), n);
    }

    if (size > CART_RAM_SIZE) {
        rom[CART_TYPE]     = cart_type;
        rom[CART_ROM_SIZE] = rom_size_code;
        rom[CART_RAM_SIZE] = ram_size_code;
    }
    return rom;
}

void emu_reset_with_cart(unsigned banks, uint8_t cart_type,
                         uint8_t rom_size_code, uint8_t ram_size_code)
{
    if (banks < 2) {
        banks = 2;
    }

    free(g_fixture_cart);
    g_fixture_cart = test_build_rom(banks * TEST_ROM_BANK_BYTES, cart_type,
                                    rom_size_code, ram_size_code);

    /* Install directly rather than via gb_cart_load() so that the loader
     * itself stays under test rather than becoming part of the fixture. */
    g_cart      = g_fixture_cart;
    g_cart_size = banks * TEST_ROM_BANK_BYTES;

    g_enable_ram          = false;
    g_rom_bank            = 0x01;
    g_ram_bank            = 0x00;
    g_banking_mode        = 0x00;
    g_number_of_rom_banks = (uint16_t)banks;
    g_number_of_ram_banks = 0;

    cpu_init();
    init_mem_map();

    current_cycle_time = 0;
    cpu_cycles         = 0;
    interrupts_enabled = false;
    power_mode         = PWR_NORMAL;

    clock_timer = 0;
    div_timer   = 0;

    ppu_cycles       = 0;
    ppu_cycles_count = 20;      /* OAM_SEARCH_CYCLES */
    lcd_enabled      = false;
    oam_list_size    = 0;

    previous_keys = 0xFF;   // active low: nothing pressed
}

void emu_reset(void)
{
    emu_reset_with_cart(2, TYPE_ROM_NO_MBC, ROM_SIZE_32KB, RAM_SIZE_0);
}

uint16_t emu_place_code(const uint8_t *bytes, unsigned len)
{
    const uint16_t base = GB_RAM_1;   /* 0xC000, plain work RAM */
    unsigned i;

    for (i = 0; i < len; i++) {
        set_mem_map_8((uint16_t)(base + i), bytes[i]);
    }
    CPU_REG.PC = base;
    return base;
}

const char *test_write_temp_file(const void *data, unsigned len)
{
    static char path[] = "/tmp/gb_emu_test_XXXXXX";
    static char kept[sizeof(path)];
    int fd;

    memcpy(path, "/tmp/gb_emu_test_XXXXXX", sizeof(path));
    fd = mkstemp(path);
    if (fd < 0) {
        fprintf(stderr, "test_write_temp_file: mkstemp failed\n");
        exit(2);
    }
    if (len > 0 && write(fd, data, len) != (ssize_t)len) {
        fprintf(stderr, "test_write_temp_file: short write\n");
        exit(2);
    }
    close(fd);
    memcpy(kept, path, sizeof(path));
    return kept;
}
