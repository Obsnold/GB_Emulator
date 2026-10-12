/* test_ppu.c - scanline rendering edge cases.
 *
 * The per-line draw functions in gb_ppu.c are not declared in gb_ppu.h but are
 * external, so tests can set up VRAM and OAM by hand and call one line of
 * rendering in isolation. That keeps these cases about geometry and clipping
 * rather than about frame sequencing.
 */
#include "test.h"
#include "test_support.h"

#include "gb_mem_map.h"
#include "gb_ppu.h"
#include "gb_timer.h"

/* Defined in Src/io_ports/gb_ppu.c. */
void set_pallets(void);
void draw_background_line(void);
void draw_window_line(void);
void draw_sprites_line(void);
void ppu_oam_search(void);

#define TEST_LINE 50

static void fill_map(uint16_t map, uint8_t tile)
{
    uint16_t i;

    for (i = 0; i < 32 * 32; i++) {
        set_mem_map_8((uint16_t)(map + i), tile);
    }
}

static void write_tile(uint8_t index, uint8_t plane_0, uint8_t plane_1)
{
    uint16_t base = (uint16_t)(TILE_RAM_0 + index * 0x10);
    uint16_t i;

    for (i = 0; i < 16; i += 2) {
        set_mem_map_8((uint16_t)(base + i),     plane_0);
        set_mem_map_8((uint16_t)(base + i + 1), plane_1);
    }
}

/* Background on BG_MAP_2 with a solid colour, window on BG_MAP_1 with a
 * different solid colour, 0x8000 unsigned tile addressing. */
static void setup_bg_and_window(void)
{
    emu_reset();

    set_mem_map_8(LCD_CTRL, (uint8_t)(LCD_CTRL_ENABLE |
                                      LCD_CTRL_BG_W_TILE_SELECT |
                                      LCD_CTRL_BG_MAP_SELECT));
    set_mem_map_8(LCD_BGP, 0xC0);        /* index 0 -> white, index 3 -> black */
    set_mem_map_8(LCD_SCX, 0);
    set_mem_map_8(LCD_SCY, 0);
    set_mem_map_8(LCD_LY, TEST_LINE);
    set_mem_map_8(LCD_WY, 0);

    write_tile(0, 0x00, 0x00);           /* colour index 0 */
    write_tile(1, 0xFF, 0xFF);           /* colour index 3 */
    fill_map(BG_MAP_2, 1);               /* background: index 3 */
    fill_map(BG_MAP_1, 0);               /* window:     index 0 */

    set_pallets();
}

/* WX is an unsigned 8-bit register; the window's left edge is WX-7. A WX past
 * the right of the screen means no window on this line. Computing WX-7 in a
 * signed 8-bit variable wraps 0xF8 to -8, which then clamps to 0 and paints
 * the window across the entire scanline. */
TEST(window_is_not_drawn_when_wx_is_off_screen)
{
    uint32_t background;

    setup_bg_and_window();
    draw_background_line();
    background = gb_display[0][TEST_LINE];

    set_mem_map_8(LCD_WX, 0xF8);         /* left edge at x=241, off screen */
    draw_window_line();

    EXPECT_EQ_X(gb_display[0][TEST_LINE],   background);
    EXPECT_EQ_X(gb_display[80][TEST_LINE],  background);
    EXPECT_EQ_X(gb_display[159][TEST_LINE], background);
}

TEST(window_is_drawn_from_wx_minus_seven)
{
    uint32_t background;

    setup_bg_and_window();
    draw_background_line();
    background = gb_display[0][TEST_LINE];

    set_mem_map_8(LCD_WX, 87);           /* left edge at x=80 */
    draw_window_line();

    EXPECT_EQ_X(gb_display[79][TEST_LINE], background);
    EXPECT(gb_display[80][TEST_LINE] != background);
}

TEST(window_covers_the_line_when_wx_is_zero)
{
    uint32_t background;

    setup_bg_and_window();
    draw_background_line();
    background = gb_display[0][TEST_LINE];

    set_mem_map_8(LCD_WX, 0);
    draw_window_line();

    EXPECT(gb_display[0][TEST_LINE] != background);
    EXPECT(gb_display[159][TEST_LINE] != background);
}

/* --- sprites ----------------------------------------------------------- */

static void place_sprite(uint8_t slot, uint8_t y, uint8_t x,
                         uint8_t tile, uint8_t flags)
{
    uint16_t oam = (uint16_t)(OAM_TABLE + slot * OAM_SIZE);

    set_mem_map_8((uint16_t)(oam + OAM_Y_POS), y);
    set_mem_map_8((uint16_t)(oam + OAM_X_POS), x);
    set_mem_map_8((uint16_t)(oam + OAM_TILE),  tile);
    set_mem_map_8((uint16_t)(oam + OAM_FLAGS), flags);
}

static void setup_sprites(void)
{
    emu_reset();

    set_mem_map_8(LCD_CTRL, (uint8_t)(LCD_CTRL_ENABLE |
                                      LCD_CTRL_BG_W_TILE_SELECT |
                                      LCD_CTRL_OBJ_ENABLE));
    set_mem_map_8(LCD_BGP,  0xC0);
    set_mem_map_8(LCD_OBP0, 0xE4);       /* 0,1,2,3 -> trans, light, dark, black */
    set_mem_map_8(LCD_LY, TEST_LINE);
    set_mem_map_8(LCD_SCX, 0);
    set_mem_map_8(LCD_SCY, 0);

    write_tile(0, 0x00, 0x00);
    fill_map(BG_MAP_1, 0);
    set_pallets();
    draw_background_line();
}

/* Columns of a sprite that fall outside 0..159 must be dropped, not clamped
 * onto the boundary pixel: clamping stacks every hidden column onto the edge,
 * so the edge shows the last hidden column instead of the last visible one.
 *
 * The tile below puts colour 1 in column 3 and colour 2 in column 7. With the
 * sprite at X=164, column 3 is the rightmost visible column (x=159) and
 * columns 4..7 are off screen, so x=159 must show colour 1.
 */
TEST(sprite_columns_past_the_right_edge_are_dropped)
{
    setup_sprites();

    /* plane 0 bit for column 3 == 0x80 >> 3; plane 1 bit for column 7 == 0x01 */
    set_mem_map_8(TILE_RAM_0 + 0, 0x10);
    set_mem_map_8(TILE_RAM_0 + 1, 0x01);

    place_sprite(0, TEST_LINE + 16, 164, 0, 0x00);
    ppu_oam_search();
    draw_sprites_line();

    EXPECT_EQ_X(gb_display[159][TEST_LINE], PAL_L_GRAY);
}

/* Mirror case on the left: at X=4, columns 0..3 are off screen and column 4 is
 * the leftmost visible one. Colour 1 in column 0 and colour 2 in column 4
 * means x=0 must show colour 2.
 */
TEST(sprite_columns_before_the_left_edge_are_dropped)
{
    setup_sprites();

    /* plane 0 bit for column 0 == 0x80; plane 1 bit for column 4 == 0x08 */
    set_mem_map_8(TILE_RAM_0 + 0, 0x80);
    set_mem_map_8(TILE_RAM_0 + 1, 0x08);

    place_sprite(0, TEST_LINE + 16, 4, 0, 0x00);
    ppu_oam_search();
    draw_sprites_line();

    EXPECT_EQ_X(gb_display[0][TEST_LINE], PAL_D_GRAY);
}

/* A sprite entirely off the left edge must leave the background alone. */
TEST(fully_offscreen_sprite_does_not_draw)
{
    uint32_t background;

    setup_sprites();
    background = gb_display[0][TEST_LINE];

    write_tile(0, 0xFF, 0xFF);
    place_sprite(0, TEST_LINE + 16, 0, 0, 0x00);   /* X=0 is "hidden" on DMG */
    ppu_oam_search();
    draw_sprites_line();

    EXPECT_EQ_X(gb_display[0][TEST_LINE], background);
}

/* In 8x16 mode the hardware ignores bit 0 of the tile index: the pair is
 * (index & 0xFE, index | 0x01). An odd index must therefore render the same
 * top half as the even index below it. */
TEST(tall_sprites_mask_the_low_bit_of_the_tile_index)
{
    uint32_t with_even_index;

    setup_sprites();
    set_mem_map_bit(LCD_CTRL, LCD_CTRL_OBJ_SIZE);

    write_tile(2, 0xFF, 0xFF);       /* colour 3 */
    write_tile(3, 0x00, 0x00);       /* colour 0, transparent */

    place_sprite(0, TEST_LINE + 16, 20, 2, 0x00);
    ppu_oam_search();
    draw_sprites_line();
    with_even_index = gb_display[16][TEST_LINE];

    setup_sprites();
    set_mem_map_bit(LCD_CTRL, LCD_CTRL_OBJ_SIZE);
    write_tile(2, 0xFF, 0xFF);
    write_tile(3, 0x00, 0x00);

    place_sprite(0, TEST_LINE + 16, 20, 3, 0x00);   /* odd index */
    ppu_oam_search();
    draw_sprites_line();

    EXPECT_EQ_X(gb_display[16][TEST_LINE], with_even_index);
}

TEST(oam_search_finds_at_most_ten_sprites_per_line)
{
    uint8_t slot;

    setup_sprites();

    for (slot = 0; slot < 40; slot++) {
        place_sprite(slot, TEST_LINE + 16, (uint8_t)(8 + slot), 0, 0x00);
    }
    ppu_oam_search();

    EXPECT_EQ_U(oam_list_size, 10);
}

TEST(oam_search_skips_sprites_on_other_lines)
{
    setup_sprites();

    place_sprite(0, TEST_LINE + 16,      20, 0, 0x00);   /* on this line  */
    place_sprite(1, TEST_LINE + 16 + 32, 30, 0, 0x00);   /* well below    */
    ppu_oam_search();

    EXPECT_EQ_U(oam_list_size, 1);
}

/* --- frame sequencing -------------------------------------------------- */

/* V-blank is one event per frame. Raising it on each of the ten v-blank
 * scanlines makes a game's v-blank handler run ten times per frame. */
TEST(vblank_interrupt_is_raised_once_per_frame)
{
    const unsigned long step_ns = 1000;
    unsigned long i;
    int raises = 0;
    uint8_t prev_ly;
    int frames = 0;

    emu_reset();
    set_mem_map_bit(LCD_CTRL, LCD_CTRL_ENABLE);
    clear_mem_map_bit(INTERRUPT_FLAGS, INTERRUPT_V_BLANK);

    /* Align to a frame boundary. */
    prev_ly = get_mem_map_8(LCD_LY);
    for (i = 0; i < 200000; i++) {
        uint8_t ly;
        ppu(step_ns);
        ly = get_mem_map_8(LCD_LY);
        if (prev_ly != 0 && ly == 0) {
            break;
        }
        prev_ly = ly;
    }

    clear_mem_map_bit(INTERRUPT_FLAGS, INTERRUPT_V_BLANK);
    prev_ly = get_mem_map_8(LCD_LY);

    for (i = 0; i < 200000 && frames == 0; i++) {
        uint8_t ly;

        ppu(step_ns);

        if (get_mem_map_bit(INTERRUPT_FLAGS, INTERRUPT_V_BLANK)) {
            raises++;
            clear_mem_map_bit(INTERRUPT_FLAGS, INTERRUPT_V_BLANK);
        }

        ly = get_mem_map_8(LCD_LY);
        if (prev_ly != 0 && ly == 0) {
            frames = 1;
        }
        prev_ly = ly;
    }

    CHECK_EQ_U(frames, 1);
    EXPECT_EQ_U(raises, 1);
}

/* LY must stay within 0..153. */
TEST(scanline_counter_stays_in_range)
{
    const unsigned long step_ns = 1000;
    unsigned long i;

    emu_reset();
    set_mem_map_bit(LCD_CTRL, LCD_CTRL_ENABLE);

    for (i = 0; i < 100000; i++) {
        uint8_t ly;

        ppu(step_ns);
        ly = get_mem_map_8(LCD_LY);
        if (ly > 153) {
            test_record_failure(__FILE__, __LINE__,
                "LCD_LY reached %u, must stay within 0..153", ly);
            return;
        }
    }
}

/* Background palette indices must map through BGP rather than being fixed. */
TEST(background_palette_follows_bgp)
{
    emu_reset();

    set_mem_map_8(LCD_BGP, 0x00);        /* every index -> shade 0 */
    set_pallets();
    EXPECT_EQ_X(bg_pallet[0], PAL_WHITE);
    EXPECT_EQ_X(bg_pallet[3], PAL_WHITE);

    set_mem_map_8(LCD_BGP, 0xFF);        /* every index -> shade 3 */
    set_pallets();
    EXPECT_EQ_X(bg_pallet[0], PAL_BLACK);
    EXPECT_EQ_X(bg_pallet[3], PAL_BLACK);

    set_mem_map_8(LCD_BGP, 0xE4);        /* 3,2,1,0 */
    set_pallets();
    EXPECT_EQ_X(bg_pallet[0], PAL_WHITE);
    EXPECT_EQ_X(bg_pallet[1], PAL_L_GRAY);
    EXPECT_EQ_X(bg_pallet[2], PAL_D_GRAY);
    EXPECT_EQ_X(bg_pallet[3], PAL_BLACK);
}
