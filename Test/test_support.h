/* test_support.h - fixtures for driving the emulator core from tests.
 *
 * The core links without SDL and without main.c, and every piece of emulator
 * state is a non-static global, so tests can reset the machine between cases
 * without any production code having to grow a test seam. The externs below
 * are those globals; if one of them ever becomes static, the link will break
 * loudly rather than silently skipping a reset.
 */
#ifndef TEST_SUPPORT_H
#define TEST_SUPPORT_H

#include <stdint.h>
#include <stdbool.h>

#include "gb_cpu.h"
#include "gb_mem_map.h"

/* --- emulator globals used as test seams ------------------------------- */

/* Src/gb_cpu.c */
extern uint8_t        current_cycle_time;
extern bool           interrupts_enabled;
extern unsigned long  cpu_cycles;
extern enum cpu_power power_mode;

/* Src/io_ports/gb_timer.c */
extern unsigned long clock_timer;
extern unsigned long div_timer;

/* Src/io_ports/gb_ppu.c */
extern unsigned long ppu_cycles;
extern unsigned long ppu_cycles_count;
extern bool          lcd_enabled;
extern uint8_t       oam_list[10];
extern uint8_t       oam_list_size;
extern uint32_t      bg_pallet[4];

/* Src/io_ports/gb_cart.c */
extern uint8_t     *g_cart;
extern unsigned int g_cart_size;
extern bool         g_enable_ram;
extern uint16_t     g_rom_bank;
extern uint8_t      g_ram_bank;
extern uint8_t      g_banking_mode;
extern uint16_t     g_number_of_rom_banks;
extern uint16_t     g_number_of_ram_banks;

/* Src/io_ports/gb_joypad_input.c */
extern uint8_t previous_keys;

/* --- fixtures ---------------------------------------------------------- */

/* Header byte offsets are CART_TYPE / CART_ROM_SIZE / CART_RAM_SIZE. */
#define TEST_ROM_BANK_BYTES 0x4000

/* Reset the whole machine and install a synthetic cart.
 *
 * banks         number of 16 KiB ROM banks to allocate (>= 2)
 * cart_type     value for header byte 0x147 (TYPE_ROM_* )
 * rom_size_code value for header byte 0x148 (ROM_SIZE_*)
 * ram_size_code value for header byte 0x149 (RAM_SIZE_*)
 *
 * Every byte of bank n is filled with (0xA0 | n) so a test can tell which
 * bank a read actually landed in. The cart is owned by the fixture.
 */
void emu_reset_with_cart(unsigned banks, uint8_t cart_type,
                         uint8_t rom_size_code, uint8_t ram_size_code);

/* emu_reset_with_cart(2, TYPE_ROM_NO_MBC, ROM_SIZE_32KB, RAM_SIZE_0) */
void emu_reset(void);

/* Place executable bytes in work RAM and point PC at them.
 * Returns the address the bytes were written to (0xC000).
 * Work RAM is plain gb_mem_map storage, so this avoids cart banking entirely
 * when the thing under test is an opcode rather than the memory map.
 */
uint16_t emu_place_code(const uint8_t *bytes, unsigned len);

/* Write a buffer to a temp file and return the path (valid until next call). */
const char *test_write_temp_file(const void *data, unsigned len);

/* Build a ROM image with a valid-enough header. Caller frees. */
uint8_t *test_build_rom(unsigned size, uint8_t cart_type,
                        uint8_t rom_size_code, uint8_t ram_size_code);

#endif /* TEST_SUPPORT_H */
