/* test_cart.c - cart loading, header validation and MBC banking. */
#include "test.h"
#include "test_support.h"

#include "gb_cart.h"
#include "gb_load.h"
#include "gb_mem_map.h"

#include <stdlib.h>
#include <string.h>

/* A file too short to contain a cart header must be rejected before any header
 * byte is read; the header runs to 0x14F, so anything under 0x150 is invalid. */
TEST(undersized_rom_is_rejected)
{
    uint8_t tiny[200];
    const char *path;

    memset(tiny, 0xFF, sizeof(tiny));
    path = test_write_temp_file(tiny, sizeof(tiny));

    EXPECT(gb_cart_load((char *)path) < 0);
}

TEST(missing_rom_file_is_rejected)
{
    EXPECT(gb_cart_load("/tmp/gb_emu_definitely_not_a_real_rom") < 0);
}

TEST(valid_rom_loads)
{
    uint8_t *rom = test_build_rom(0x8000, TYPE_ROM_NO_MBC,
                                  ROM_SIZE_32KB, RAM_SIZE_0);
    const char *path = test_write_temp_file(rom, 0x8000);

    EXPECT(gb_cart_load((char *)path) >= 0);
    free(rom);
}

/* Bank 0 is permanently mapped at 0x0000..0x3FFF. */
TEST(fixed_bank_reads_from_bank_zero)
{
    emu_reset_with_cart(4, TYPE_ROM_MBC1, ROM_SIZE_64KB, RAM_SIZE_0);

    g_rom_bank = 0x03;
    EXPECT_EQ_X(gb_cart_read_rom(0x0000), 0xA0);
    EXPECT_EQ_X(gb_cart_read_rom(0x3FFF), 0xA0);
}

TEST(switchable_bank_reads_from_selected_bank)
{
    emu_reset_with_cart(4, TYPE_ROM_MBC1, ROM_SIZE_64KB, RAM_SIZE_0);

    gb_cart_write_rom(0x2000, 0x02);          /* select ROM bank 2 */
    EXPECT_EQ_X(g_rom_bank, 0x02);
    EXPECT_EQ_X(gb_cart_read_rom(0x4000), 0xA2);

    gb_cart_write_rom(0x2000, 0x03);
    EXPECT_EQ_X(gb_cart_read_rom(0x4000), 0xA3);
}

/* MBC1 maps a requested bank 0 to bank 1, because bank 0 is already fixed. */
TEST(mbc1_bank_zero_selects_bank_one)
{
    emu_reset_with_cart(4, TYPE_ROM_MBC1, ROM_SIZE_64KB, RAM_SIZE_0);

    gb_cart_write_rom(0x2000, 0x00);
    EXPECT_EQ_X(g_rom_bank, 0x01);
}

/* A bank number past the end of the image must be masked to the cart's real
 * bank count. Without this, the read indexes past the malloc'd ROM buffer. */
TEST(rom_bank_is_clamped_to_cart_size)
{
    unsigned i;

    emu_reset_with_cart(2, TYPE_ROM_MBC1, ROM_SIZE_32KB, RAM_SIZE_0);

    /* 2 banks exist; sweep every value MBC1 can latch. */
    for (i = 0; i < 0x20; i++) {
        uint8_t data;

        gb_cart_write_rom(0x2000, (uint8_t)i);
        data = gb_cart_read_rom(0x4000);

        /* Bank fill bytes are 0xA0 | bank, so a read may only ever return a
         * bank that exists in this 2-bank image. */
        if (data != 0xA0 && data != 0xA1) {
            test_record_failure(__FILE__, __LINE__,
                "bank request %u read 0x%02X: outside the 2-bank image "
                "(g_rom_bank=%u, cart is %u bytes)",
                i, data, g_rom_bank, g_cart_size);
        }
    }
}

TEST(rom_bank_never_exceeds_bank_count)
{
    unsigned i;

    emu_reset_with_cart(2, TYPE_ROM_MBC1, ROM_SIZE_32KB, RAM_SIZE_0);

    for (i = 0; i < 0x20; i++) {
        gb_cart_write_rom(0x2000, (uint8_t)i);
        if (g_rom_bank >= g_number_of_rom_banks) {
            test_record_failure(__FILE__, __LINE__,
                "bank request %u latched g_rom_bank=%u, cart has %u banks",
                i, g_rom_bank, g_number_of_rom_banks);
        }
    }
}

/* Cart RAM must not retain writes while it is disabled. */
TEST(cart_ram_ignores_writes_while_disabled)
{
    emu_reset_with_cart(4, TYPE_ROM_MBC1_RAM_BAT, ROM_SIZE_64KB, RAM_SIZE_32KB);

    gb_cart_write_rom(0x0000, 0x0A);          /* enable  */
    gb_cart_write_ram(CART_RAM, 0x5A);
    EXPECT_EQ_X(gb_cart_read_ram(CART_RAM), 0x5A);

    gb_cart_write_rom(0x0000, 0x00);          /* disable */
    EXPECT_EQ_X(g_enable_ram, false);

    gb_cart_write_ram(CART_RAM, 0xFF);        /* must be dropped */

    gb_cart_write_rom(0x0000, 0x0A);          /* re-enable and look */
    EXPECT_EQ_X(gb_cart_read_ram(CART_RAM), 0x5A);
}

/* Only 0x0A in the low nibble enables cart RAM. */
TEST(cart_ram_enable_requires_value_0x0a)
{
    emu_reset_with_cart(4, TYPE_ROM_MBC1_RAM_BAT, ROM_SIZE_64KB, RAM_SIZE_32KB);

    gb_cart_write_rom(0x0000, 0x0A);
    EXPECT_EQ_X(g_enable_ram, true);

    gb_cart_write_rom(0x0000, 0x02);
    EXPECT_EQ_X(g_enable_ram, false);

    gb_cart_write_rom(0x0000, 0x08);
    EXPECT_EQ_X(g_enable_ram, false);
}

TEST(mbc1_selects_ram_bank)
{
    emu_reset_with_cart(4, TYPE_ROM_MBC1_RAM_BAT, ROM_SIZE_64KB, RAM_SIZE_32KB);

    gb_cart_write_rom(0x0000, 0x0A);
    gb_cart_write_rom(0x4000, 0x01);
    EXPECT_EQ_X(g_ram_bank, 0x01);

    gb_cart_write_ram(CART_RAM, 0x11);

    gb_cart_write_rom(0x4000, 0x02);
    gb_cart_write_ram(CART_RAM, 0x22);

    gb_cart_write_rom(0x4000, 0x01);
    EXPECT_EQ_X(gb_cart_read_ram(CART_RAM), 0x11);
}

/* MBC5 splits the ROM bank number across 0x2000 and 0x3000 and reaches 512
 * banks. Carts declaring MBC5 currently fall through to the no-banking case. */
TEST(mbc5_switches_rom_banks)
{
    emu_reset_with_cart(8, TYPE_ROM_MBC5, ROM_SIZE_128KB, RAM_SIZE_0);

    gb_cart_write_rom(0x2000, 0x03);
    EXPECT_EQ_X(g_rom_bank, 0x03);
    EXPECT_EQ_X(gb_cart_read_rom(0x4000), 0xA3);
}

/* MBC2 latches a 4-bit ROM bank from writes to 0x2000..0x3FFF that have
 * address bit 8 set. */
TEST(mbc2_switches_rom_banks)
{
    emu_reset_with_cart(4, TYPE_ROM_MBC2, ROM_SIZE_64KB, RAM_SIZE_0);

    gb_cart_write_rom(0x2100, 0x02);
    EXPECT_EQ_X(g_rom_bank, 0x02);
}

/* The mapper latches only the low bits of the bank number, and the "bank 0
 * means bank 1" translation applies to that masked value. Testing the raw byte
 * instead lets a value like 0xA0, which masks to 0, select bank 0. */
TEST(mbc1_bank_zero_translation_uses_the_masked_value)
{
    emu_reset_with_cart(4, TYPE_ROM_MBC1, ROM_SIZE_64KB, RAM_SIZE_0);

    gb_cart_write_rom(0x2000, 0xA0);          /* 0xA0 & 0x1F == 0 */
    EXPECT_EQ_X(g_rom_bank, 0x01);
}

TEST(mbc3_bank_zero_translation_uses_the_masked_value)
{
    emu_reset_with_cart(4, TYPE_ROM_MBC3, ROM_SIZE_64KB, RAM_SIZE_0);

    gb_cart_write_rom(0x2000, 0x80);          /* 0x80 & 0x7F == 0 */
    EXPECT_EQ_X(g_rom_bank, 0x01);
}
