/* test_mem_map.c - address decoding and bounds of the 64 KiB memory map. */
#include "test.h"
#include "test_support.h"

#include "gb_mem_map.h"

/* The Game Boy address space is 0x0000..0xFFFF inclusive, so the backing array
 * needs 0x10000 bytes. Sizing it 0xFFFF leaves the interrupt-enable register at
 * 0xFFFF one byte past the end, and the CPU touches IE on every step. */
TEST(memory_map_covers_the_whole_address_space)
{
    EXPECT_EQ_U(GB_MEM_SIZE, 0x10000);
}

TEST(interrupt_enable_register_round_trips)
{
    emu_reset();

    set_mem_map_8(INTERRUPT_ENABLE, 0x1F);
    EXPECT_EQ_X(get_mem_map_8(INTERRUPT_ENABLE), 0x1F);

    set_mem_map_8(INTERRUPT_ENABLE, 0x00);
    EXPECT_EQ_X(get_mem_map_8(INTERRUPT_ENABLE), 0x00);
}

TEST(work_ram_round_trips)
{
    emu_reset();

    set_mem_map_8(GB_RAM_1, 0x5A);
    set_mem_map_8(GB_RAM_2, 0xA5);
    set_mem_map_8(ECHO_RAM - 1, 0x3C);

    EXPECT_EQ_X(get_mem_map_8(GB_RAM_1), 0x5A);
    EXPECT_EQ_X(get_mem_map_8(GB_RAM_2), 0xA5);
    EXPECT_EQ_X(get_mem_map_8(ECHO_RAM - 1), 0x3C);
}

TEST(zero_page_round_trips)
{
    emu_reset();

    set_mem_map_8(ZERO_PAGE, 0x11);
    set_mem_map_8(INTERRUPT_EN - 1, 0x22);

    EXPECT_EQ_X(get_mem_map_8(ZERO_PAGE), 0x11);
    EXPECT_EQ_X(get_mem_map_8(INTERRUPT_EN - 1), 0x22);
}

/* Echo RAM at 0xE000..0xFDFF mirrors work RAM at 0xC000..0xDDFF. */
TEST(echo_ram_mirrors_work_ram)
{
    emu_reset();

    set_mem_map_8(GB_RAM_1 + 0x100, 0x77);
    EXPECT_EQ_X(op_get_mem_map_8(ECHO_RAM + 0x100), 0x77);
}

TEST(sixteen_bit_access_is_little_endian)
{
    emu_reset();

    op_set_mem_map_16(GB_RAM_1, 0xBEEF);
    EXPECT_EQ_X(op_get_mem_map_8(GB_RAM_1),     0xEF);
    EXPECT_EQ_X(op_get_mem_map_8(GB_RAM_1 + 1), 0xBE);
    EXPECT_EQ_X(op_get_mem_map_16(GB_RAM_1),    0xBEEF);
}

TEST(vram_is_readable_outside_pixel_transfer)
{
    emu_reset();

    set_mem_map_8(TILE_RAM_0 + 0x10, 0x3C);

    set_mem_map_8(LCD_STAT, LCD_STAT_MODE_HBLNK);
    EXPECT_EQ_X(op_get_mem_map_8(TILE_RAM_0 + 0x10), 0x3C);

    set_mem_map_8(LCD_STAT, LCD_STAT_MODE_VBLNK);
    EXPECT_EQ_X(op_get_mem_map_8(TILE_RAM_0 + 0x10), 0x3C);
}

/* OAM is unreachable by the CPU during modes 2 and 3. */
TEST(oam_is_blocked_during_oam_scan_and_pixel_transfer)
{
    emu_reset();

    set_mem_map_8(LCD_STAT, LCD_STAT_MODE_HBLNK);
    op_set_mem_map_8(OAM_TABLE, 0x5A);
    EXPECT_EQ_X(op_get_mem_map_8(OAM_TABLE), 0x5A);

    set_mem_map_8(LCD_STAT, LCD_STAT_MODE_OAM);
    op_set_mem_map_8(OAM_TABLE, 0xFF);
    EXPECT_EQ_X(get_mem_map_8(OAM_TABLE), 0x5A);

    set_mem_map_8(LCD_STAT, LCD_STAT_MODE_PIXEL_TRANS);
    op_set_mem_map_8(OAM_TABLE, 0xFF);
    EXPECT_EQ_X(get_mem_map_8(OAM_TABLE), 0x5A);
}

/* Writing through the CPU path must never modify cart ROM contents. */
TEST(cart_rom_is_not_writable_through_the_cpu_path)
{
    uint8_t before;

    emu_reset();

    before = op_get_mem_map_8(0x2000);
    op_set_mem_map_8(0x2000, (uint8_t)~before);
    EXPECT_EQ_X(op_get_mem_map_8(0x2000), before);
}

/* LY is read-only to the CPU and a write resets the counter. Letting a write
 * through put a value past the bottom of the screen into it, which the
 * scanline renderers then used to index gb_display. */
TEST(scanline_counter_is_read_only_to_the_cpu)
{
    emu_reset();

    set_mem_map_8(LCD_LY, 100);
    op_set_mem_map_8(LCD_LY, 200);

    EXPECT_EQ_X(get_mem_map_8(LCD_LY), 0x00);
}

/* STAT's mode (bits 0-1) and coincidence flag (bit 2) are driven by the PPU and
 * read-only to the CPU. A writable mode let a ROM select mode 3 while LY was in
 * v-blank, and the renderer then drew a scanline off the bottom of the buffer. */
TEST(lcd_stat_mode_bits_are_read_only)
{
    emu_reset();

    set_mem_map_8(LCD_STAT, LCD_STAT_MODE_VBLNK);
    op_set_mem_map_8(LCD_STAT, LCD_STAT_MODE_PIXEL_TRANS);

    EXPECT_EQ_X(get_mem_map_8(LCD_STAT) & LCD_STAT_MODE, LCD_STAT_MODE_VBLNK);
}

TEST(lcd_stat_interrupt_enables_are_writable)
{
    emu_reset();

    op_set_mem_map_8(LCD_STAT,
                     (uint8_t)(LCD_STAT_VBLNK_INTR_EN | LCD_STAT_HBLNK_INTR_EN));

    EXPECT(get_mem_map_bit(LCD_STAT, LCD_STAT_VBLNK_INTR_EN) != 0);
    EXPECT(get_mem_map_bit(LCD_STAT, LCD_STAT_HBLNK_INTR_EN) != 0);
}
