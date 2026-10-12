/* test_joypad.c - the 0xFF00 joypad register.
 *
 * Key state is active low throughout: a clear bit means pressed. The game
 * selects which half of the pad it wants to read by clearing bit 4 (direction
 * keys) or bit 5 (buttons), then reads the low nibble.
 */
#include "test.h"
#include "test_support.h"

#include "gb_joypad_input.h"
#include "gb_mem_map.h"
#include "system_interface.h"

#define SELECT_BUTTONS    0x10      /* bit 5 clear, bit 4 set */
#define SELECT_DIRECTIONS 0x20      /* bit 4 clear, bit 5 set */
#define NO_KEYS           0xFF

TEST(direction_keys_read_back_in_the_low_nibble)
{
    emu_reset();
    set_mem_map_8(JOYPAD_INPUT, SELECT_DIRECTIONS);

    gb_input((uint8_t)(NO_KEYS & ~KEY_RIGHT));
    EXPECT_EQ_X(get_mem_map_8(JOYPAD_INPUT) & 0x0F, 0x0E);

    set_mem_map_8(JOYPAD_INPUT, SELECT_DIRECTIONS);
    gb_input((uint8_t)(NO_KEYS & ~KEY_DOWN));
    EXPECT_EQ_X(get_mem_map_8(JOYPAD_INPUT) & 0x0F, 0x07);
}

TEST(buttons_read_back_in_the_low_nibble)
{
    emu_reset();
    set_mem_map_8(JOYPAD_INPUT, SELECT_BUTTONS);

    gb_input((uint8_t)(NO_KEYS & ~KEY_A));
    EXPECT_EQ_X(get_mem_map_8(JOYPAD_INPUT) & 0x0F, 0x0E);

    set_mem_map_8(JOYPAD_INPUT, SELECT_BUTTONS);
    gb_input((uint8_t)(NO_KEYS & ~KEY_START));
    EXPECT_EQ_X(get_mem_map_8(JOYPAD_INPUT) & 0x0F, 0x07);
}

TEST(nothing_pressed_reads_as_all_ones)
{
    emu_reset();

    set_mem_map_8(JOYPAD_INPUT, SELECT_DIRECTIONS);
    gb_input(NO_KEYS);
    EXPECT_EQ_X(get_mem_map_8(JOYPAD_INPUT) & 0x0F, 0x0F);

    set_mem_map_8(JOYPAD_INPUT, SELECT_BUTTONS);
    gb_input(NO_KEYS);
    EXPECT_EQ_X(get_mem_map_8(JOYPAD_INPUT) & 0x0F, 0x0F);
}

/* Bits 4 and 5 are written by the game to choose a half of the pad. Reading the
 * pad must not change them, or the selection the game just made is lost and it
 * has to re-select before every single read. */
TEST(reading_the_pad_preserves_the_select_bits)
{
    emu_reset();

    set_mem_map_8(JOYPAD_INPUT, SELECT_BUTTONS);
    gb_input(NO_KEYS);
    EXPECT_EQ_X(get_mem_map_8(JOYPAD_INPUT) & 0x30, SELECT_BUTTONS);

    set_mem_map_8(JOYPAD_INPUT, SELECT_DIRECTIONS);
    gb_input(NO_KEYS);
    EXPECT_EQ_X(get_mem_map_8(JOYPAD_INPUT) & 0x30, SELECT_DIRECTIONS);
}

/* The joypad interrupt is a high-to-low transition, i.e. a key going down.
 * Raising it on release too makes a game's input handler run twice per press. */
TEST(joypad_interrupt_fires_on_press)
{
    emu_reset();
    set_mem_map_8(JOYPAD_INPUT, SELECT_BUTTONS);

    gb_input(NO_KEYS);
    clear_mem_map_bit(INTERRUPT_FLAGS, INTERRUPT_JOYPAD);

    gb_input((uint8_t)(NO_KEYS & ~KEY_A));
    EXPECT(get_mem_map_bit(INTERRUPT_FLAGS, INTERRUPT_JOYPAD) != 0);
}

TEST(joypad_interrupt_does_not_fire_on_release)
{
    emu_reset();
    set_mem_map_8(JOYPAD_INPUT, SELECT_BUTTONS);

    gb_input((uint8_t)(NO_KEYS & ~KEY_A));
    clear_mem_map_bit(INTERRUPT_FLAGS, INTERRUPT_JOYPAD);

    gb_input(NO_KEYS);
    EXPECT_EQ_X(get_mem_map_bit(INTERRUPT_FLAGS, INTERRUPT_JOYPAD), 0);
}

TEST(joypad_interrupt_does_not_fire_when_nothing_changes)
{
    emu_reset();
    set_mem_map_8(JOYPAD_INPUT, SELECT_BUTTONS);

    gb_input(NO_KEYS);
    clear_mem_map_bit(INTERRUPT_FLAGS, INTERRUPT_JOYPAD);

    gb_input(NO_KEYS);
    EXPECT_EQ_X(get_mem_map_bit(INTERRUPT_FLAGS, INTERRUPT_JOYPAD), 0);
}
