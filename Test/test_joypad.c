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
    EXPECT_EQ_X(op_get_mem_map_8(JOYPAD_INPUT) & 0x0F, 0x0E);

    set_mem_map_8(JOYPAD_INPUT, SELECT_DIRECTIONS);
    gb_input((uint8_t)(NO_KEYS & ~KEY_DOWN));
    EXPECT_EQ_X(op_get_mem_map_8(JOYPAD_INPUT) & 0x0F, 0x07);
}

TEST(buttons_read_back_in_the_low_nibble)
{
    emu_reset();
    set_mem_map_8(JOYPAD_INPUT, SELECT_BUTTONS);

    gb_input((uint8_t)(NO_KEYS & ~KEY_A));
    EXPECT_EQ_X(op_get_mem_map_8(JOYPAD_INPUT) & 0x0F, 0x0E);

    set_mem_map_8(JOYPAD_INPUT, SELECT_BUTTONS);
    gb_input((uint8_t)(NO_KEYS & ~KEY_START));
    EXPECT_EQ_X(op_get_mem_map_8(JOYPAD_INPUT) & 0x0F, 0x07);
}

TEST(nothing_pressed_reads_as_all_ones)
{
    emu_reset();

    set_mem_map_8(JOYPAD_INPUT, SELECT_DIRECTIONS);
    gb_input(NO_KEYS);
    EXPECT_EQ_X(op_get_mem_map_8(JOYPAD_INPUT) & 0x0F, 0x0F);

    set_mem_map_8(JOYPAD_INPUT, SELECT_BUTTONS);
    gb_input(NO_KEYS);
    EXPECT_EQ_X(op_get_mem_map_8(JOYPAD_INPUT) & 0x0F, 0x0F);
}

/* Bits 4 and 5 are written by the game to choose a half of the pad. Reading the
 * pad must not change them, or the selection the game just made is lost and it
 * has to re-select before every single read. */
TEST(reading_the_pad_preserves_the_select_bits)
{
    emu_reset();

    set_mem_map_8(JOYPAD_INPUT, SELECT_BUTTONS);
    gb_input(NO_KEYS);
    EXPECT_EQ_X(op_get_mem_map_8(JOYPAD_INPUT) & 0x30, SELECT_BUTTONS);

    set_mem_map_8(JOYPAD_INPUT, SELECT_DIRECTIONS);
    gb_input(NO_KEYS);
    EXPECT_EQ_X(op_get_mem_map_8(JOYPAD_INPUT) & 0x30, SELECT_DIRECTIONS);
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

/* A game selects a group and reads in the same breath, and reads both groups
 * within one frame:
 *
 *     ld a,$20 / ld ($ff00),a / ld a,($ff00)    ; directions
 *     ld a,$10 / ld ($ff00),a / ld a,($ff00)    ; buttons
 *
 * so the low nibble has to reflect the selection at the moment of the read,
 * not the selection that happened to be set when input was last polled. The
 * CPU reads the port through op_get_mem_map_8, which is what these use. */
TEST(pad_reflects_a_selection_made_after_input_was_polled)
{
    emu_reset();

    /* Input polled once, with neither group selected. */
    set_mem_map_8(JOYPAD_INPUT, 0x30);
    gb_input((uint8_t)(NO_KEYS & ~KEY_RIGHT & ~KEY_A));

    /* The game now selects directions and reads. */
    op_set_mem_map_8(JOYPAD_INPUT, SELECT_DIRECTIONS);
    EXPECT_EQ_X(op_get_mem_map_8(JOYPAD_INPUT) & 0x0F, 0x0E);   /* right down */

    /* ...then buttons, still without input being polled again. */
    op_set_mem_map_8(JOYPAD_INPUT, SELECT_BUTTONS);
    EXPECT_EQ_X(op_get_mem_map_8(JOYPAD_INPUT) & 0x0F, 0x0E);   /* A down */
}

/* Both groups selected pulls a line low if either half has that key down. */
TEST(pad_combines_both_groups_when_both_are_selected)
{
    emu_reset();

    op_set_mem_map_8(JOYPAD_INPUT, 0x00);
    gb_input((uint8_t)(NO_KEYS & ~KEY_START));      /* bit 3 of the button half */

    EXPECT_EQ_X(op_get_mem_map_8(JOYPAD_INPUT) & 0x0F, 0x07);
}

/* The low nibble is driven by the key matrix, so the CPU cannot write it. */
TEST(pad_low_nibble_is_not_writable)
{
    emu_reset();

    op_set_mem_map_8(JOYPAD_INPUT, SELECT_DIRECTIONS);
    gb_input(NO_KEYS);
    op_set_mem_map_8(JOYPAD_INPUT, (uint8_t)(SELECT_DIRECTIONS | 0x00));

    EXPECT_EQ_X(op_get_mem_map_8(JOYPAD_INPUT) & 0x0F, 0x0F);
}
