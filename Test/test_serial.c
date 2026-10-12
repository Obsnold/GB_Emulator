/* test_serial.c - the link port.
 *
 * Nothing is attached to the link, so a started transfer completes at once.
 * This is what the blargg test ROMs write their results to, so the harness
 * depends on the recorded text being right.
 */
#include "test.h"
#include "test_support.h"

#include "gb_mem_map.h"
#include "gb_serial.h"

#include <string.h>

/* Write one byte the way a ROM does: load SB, then start a transfer via SC. */
static void serial_send(uint8_t byte)
{
    op_set_mem_map_8(SERIAL_SB, byte);
    op_set_mem_map_8(SERIAL_SC, (uint8_t)(SERIAL_SC_START |
                                          SERIAL_SC_CLOCK_INTERNAL));
}

TEST(serial_records_a_transferred_byte)
{
    emu_reset();

    serial_send('A');

    CHECK_EQ_U(gb_serial_length(), 1);
    EXPECT_EQ_X(gb_serial_text()[0], 'A');
}

TEST(serial_records_bytes_in_order)
{
    emu_reset();

    serial_send('P');
    serial_send('a');
    serial_send('s');
    serial_send('s');

    CHECK_EQ_U(gb_serial_length(), 4);
    EXPECT(strcmp(gb_serial_text(), "Pass") == 0);
}

/* The transfer is instantaneous, so by the time the ROM reads SC back the
 * start bit has to be clear; a ROM that polls it would otherwise hang. */
TEST(serial_transfer_clears_the_start_bit)
{
    emu_reset();

    serial_send('x');

    EXPECT_EQ_X(get_mem_map_bit(SERIAL_SC, SERIAL_SC_START), 0x00);
}

TEST(serial_transfer_raises_the_interrupt)
{
    emu_reset();
    clear_mem_map_bit(INTERRUPT_FLAGS, INTERRUPT_SERIAL);

    serial_send('x');

    EXPECT(get_mem_map_bit(INTERRUPT_FLAGS, INTERRUPT_SERIAL) != 0);
}

/* With no cable the other end drives the line high, so the byte shifted in
 * reads back as 0xFF. */
TEST(serial_reads_back_all_ones_with_no_cable)
{
    emu_reset();

    serial_send(0x42);

    EXPECT_EQ_X(get_mem_map_8(SERIAL_SB), 0xFF);
}

/* Writing SC without the start bit configures the port; it must not transmit. */
TEST(serial_does_not_transfer_without_the_start_bit)
{
    emu_reset();

    op_set_mem_map_8(SERIAL_SB, 'z');
    op_set_mem_map_8(SERIAL_SC, SERIAL_SC_CLOCK_INTERNAL);

    EXPECT_EQ_U(gb_serial_length(), 0);
    EXPECT_EQ_X(get_mem_map_8(SERIAL_SB), 'z');
}

TEST(serial_buffer_is_empty_after_reset)
{
    emu_reset();
    serial_send('a');
    CHECK_EQ_U(gb_serial_length(), 1);

    emu_reset();
    EXPECT_EQ_U(gb_serial_length(), 0);
    EXPECT_EQ_X(gb_serial_text()[0], '\0');
}
