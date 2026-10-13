/* test_cpu.c - ALU flag semantics, interrupt dispatch and power modes.
 *
 * Most of these lock in behaviour that is already correct: flag handling is
 * the part of an emulator that breaks silently under later refactoring, and a
 * wrong half-carry shows up as a corrupted score counter three games later
 * rather than as a crash.
 */
#include "test.h"
#include "test_support.h"

#include "gb_cpu.h"
#include "gb_mem_map.h"
#include "gb_opcodes.h"

/* Execute exactly one instruction through the real dispatcher.
 *
 * The dispatcher pays for the previous instruction before running the next, so
 * with current_cycle_time zeroed the first one is free and a single nanosecond
 * is enough to retire it. The next instruction's real cost is at least one
 * machine cycle, which that leftover nanosecond cannot cover, so the catch-up
 * loop stops after one. */
static void step(void)
{
    current_cycle_time = 0;
    cpu_cycles         = 0;
    gb_cpu(1);
}

static void load_op(const uint8_t *bytes, unsigned len)
{
    emu_place_code(bytes, len);
    CPU_REG.SP = 0xDFF0;
}

#define F_Z ZERO_FLAG
#define F_N ADD_SUB_FLAG
#define F_H HALF_CARRY_FLAG
#define F_C CARRY_FLAG

static void expect_flags(const char *what, uint8_t expected)
{
    uint8_t got = CPU_REG.F & 0xF0;

    if (got != expected) {
        test_record_failure(__FILE__, __LINE__,
            "%s: flags Z=%d N=%d H=%d C=%d, want Z=%d N=%d H=%d C=%d",
            what,
            !!(got & F_Z), !!(got & F_N), !!(got & F_H), !!(got & F_C),
            !!(expected & F_Z), !!(expected & F_N),
            !!(expected & F_H), !!(expected & F_C));
    }
}

/* --- 8-bit arithmetic -------------------------------------------------- */

TEST(add_sets_half_carry_on_nibble_overflow)
{
    const uint8_t add_a_b[] = { 0x80 };

    emu_reset();
    load_op(add_a_b, sizeof(add_a_b));
    CPU_REG.A = 0x0F;
    CPU_REG.B = 0x01;
    step();

    EXPECT_EQ_X(CPU_REG.A, 0x10);
    expect_flags("ADD A,B 0x0F+0x01", F_H);
}

TEST(add_sets_carry_and_zero_on_byte_overflow)
{
    const uint8_t add_a_b[] = { 0x80 };

    emu_reset();
    load_op(add_a_b, sizeof(add_a_b));
    CPU_REG.A = 0xFF;
    CPU_REG.B = 0x01;
    step();

    EXPECT_EQ_X(CPU_REG.A, 0x00);
    expect_flags("ADD A,B 0xFF+0x01", F_Z | F_H | F_C);
}

TEST(add_with_no_carry_clears_all_flags)
{
    const uint8_t add_a_b[] = { 0x80 };

    emu_reset();
    load_op(add_a_b, sizeof(add_a_b));
    CPU_REG.A = 0x01;
    CPU_REG.B = 0x01;
    CPU_REG.F = 0xF0;              /* all set going in */
    step();

    EXPECT_EQ_X(CPU_REG.A, 0x02);
    expect_flags("ADD A,B 0x01+0x01", 0);
}

TEST(adc_includes_the_carry_in)
{
    const uint8_t adc_a_b[] = { 0x88 };

    emu_reset();
    load_op(adc_a_b, sizeof(adc_a_b));
    CPU_REG.A = 0x0E;
    CPU_REG.B = 0x01;
    CPU_REG.F = F_C;
    step();

    EXPECT_EQ_X(CPU_REG.A, 0x10);
    expect_flags("ADC A,B 0x0E+0x01+1", F_H);
}

TEST(adc_carry_in_can_itself_overflow_the_byte)
{
    const uint8_t adc_a_b[] = { 0x88 };

    emu_reset();
    load_op(adc_a_b, sizeof(adc_a_b));
    CPU_REG.A = 0xFF;
    CPU_REG.B = 0x00;
    CPU_REG.F = F_C;
    step();

    EXPECT_EQ_X(CPU_REG.A, 0x00);
    expect_flags("ADC A,B 0xFF+0x00+1", F_Z | F_H | F_C);
}

TEST(sub_sets_half_carry_on_nibble_borrow)
{
    const uint8_t sub_b[] = { 0x90 };

    emu_reset();
    load_op(sub_b, sizeof(sub_b));
    CPU_REG.A = 0x10;
    CPU_REG.B = 0x01;
    step();

    EXPECT_EQ_X(CPU_REG.A, 0x0F);
    expect_flags("SUB B 0x10-0x01", F_N | F_H);
}

TEST(sub_sets_carry_on_byte_borrow)
{
    const uint8_t sub_b[] = { 0x90 };

    emu_reset();
    load_op(sub_b, sizeof(sub_b));
    CPU_REG.A = 0x00;
    CPU_REG.B = 0x01;
    step();

    EXPECT_EQ_X(CPU_REG.A, 0xFF);
    expect_flags("SUB B 0x00-0x01", F_N | F_H | F_C);
}

TEST(sub_equal_values_sets_zero)
{
    const uint8_t sub_b[] = { 0x90 };

    emu_reset();
    load_op(sub_b, sizeof(sub_b));
    CPU_REG.A = 0x42;
    CPU_REG.B = 0x42;
    step();

    EXPECT_EQ_X(CPU_REG.A, 0x00);
    expect_flags("SUB B 0x42-0x42", F_Z | F_N);
}

TEST(sbc_includes_the_carry_in)
{
    const uint8_t sbc_b[] = { 0x98 };

    emu_reset();
    load_op(sbc_b, sizeof(sbc_b));
    CPU_REG.A = 0x10;
    CPU_REG.B = 0x00;
    CPU_REG.F = F_C;
    step();

    EXPECT_EQ_X(CPU_REG.A, 0x0F);
    expect_flags("SBC A,B 0x10-0x00-1", F_N | F_H);
}

TEST(cp_compares_without_modifying_a)
{
    const uint8_t cp_b[] = { 0xB8 };

    emu_reset();
    load_op(cp_b, sizeof(cp_b));
    CPU_REG.A = 0x42;
    CPU_REG.B = 0x42;
    step();

    EXPECT_EQ_X(CPU_REG.A, 0x42);
    expect_flags("CP B 0x42 vs 0x42", F_Z | F_N);
}

TEST(cp_sets_carry_when_a_is_smaller)
{
    const uint8_t cp_b[] = { 0xB8 };

    emu_reset();
    load_op(cp_b, sizeof(cp_b));
    CPU_REG.A = 0x01;
    CPU_REG.B = 0x02;
    step();

    EXPECT_EQ_X(CPU_REG.A, 0x01);
    expect_flags("CP B 0x01 vs 0x02", F_N | F_H | F_C);
}

TEST(and_sets_half_carry_and_clears_carry)
{
    const uint8_t and_b[] = { 0xA0 };

    emu_reset();
    load_op(and_b, sizeof(and_b));
    CPU_REG.A = 0x0F;
    CPU_REG.B = 0xF0;
    CPU_REG.F = F_C;
    step();

    EXPECT_EQ_X(CPU_REG.A, 0x00);
    expect_flags("AND B 0x0F & 0xF0", F_Z | F_H);
}

TEST(or_clears_every_flag_but_zero)
{
    const uint8_t or_b[] = { 0xB0 };

    emu_reset();
    load_op(or_b, sizeof(or_b));
    CPU_REG.A = 0x0F;
    CPU_REG.B = 0xF0;
    CPU_REG.F = 0xF0;
    step();

    EXPECT_EQ_X(CPU_REG.A, 0xFF);
    expect_flags("OR B 0x0F | 0xF0", 0);
}

TEST(xor_with_self_sets_zero)
{
    const uint8_t xor_a[] = { 0xAF };

    emu_reset();
    load_op(xor_a, sizeof(xor_a));
    CPU_REG.A = 0x5A;
    CPU_REG.F = 0xF0;
    step();

    EXPECT_EQ_X(CPU_REG.A, 0x00);
    expect_flags("XOR A", F_Z);
}

/* INC and DEC leave the carry flag alone; this is the classic one to get
 * wrong, and loops that carry a flag across an INC depend on it. */
TEST(inc_preserves_the_carry_flag)
{
    const uint8_t inc_b[] = { 0x04 };

    emu_reset();
    load_op(inc_b, sizeof(inc_b));
    CPU_REG.B = 0x0F;
    CPU_REG.F = F_C;
    step();

    EXPECT_EQ_X(CPU_REG.B, 0x10);
    expect_flags("INC B 0x0F with carry in", F_H | F_C);
}

TEST(dec_preserves_the_carry_flag)
{
    const uint8_t dec_b[] = { 0x05 };

    emu_reset();
    load_op(dec_b, sizeof(dec_b));
    CPU_REG.B = 0x10;
    CPU_REG.F = F_C;
    step();

    EXPECT_EQ_X(CPU_REG.B, 0x0F);
    expect_flags("DEC B 0x10 with carry in", F_N | F_H | F_C);
}

TEST(inc_wraps_to_zero)
{
    const uint8_t inc_b[] = { 0x04 };

    emu_reset();
    load_op(inc_b, sizeof(inc_b));
    CPU_REG.B = 0xFF;
    CPU_REG.F = 0x00;
    step();

    EXPECT_EQ_X(CPU_REG.B, 0x00);
    expect_flags("INC B 0xFF", F_Z | F_H);
}

TEST(daa_adjusts_after_a_bcd_addition)
{
    const uint8_t add_a_b[] = { 0x80 };
    const uint8_t daa[]     = { 0x27 };

    emu_reset();
    load_op(add_a_b, sizeof(add_a_b));
    CPU_REG.A = 0x09;
    CPU_REG.B = 0x01;
    step();
    EXPECT_EQ_X(CPU_REG.A, 0x0A);

    load_op(daa, sizeof(daa));
    step();
    EXPECT_EQ_X(CPU_REG.A, 0x10);      /* 9 + 1 == 10 decimal */
}

/* --- 16-bit and stack -------------------------------------------------- */

TEST(push_and_pop_round_trip)
{
    const uint8_t push_bc[] = { 0xC5 };
    const uint8_t pop_de[]  = { 0xD1 };

    emu_reset();
    load_op(push_bc, sizeof(push_bc));
    CPU_REG.BC = 0x1234;
    step();
    EXPECT_EQ_X(CPU_REG.SP, 0xDFEE);

    load_op(pop_de, sizeof(pop_de));
    CPU_REG.SP = 0xDFEE;
    step();
    EXPECT_EQ_X(CPU_REG.DE, 0x1234);
    EXPECT_EQ_X(CPU_REG.SP, 0xDFF0);
}

/* The low four bits of F do not exist on hardware and always read as 0. */
TEST(flag_register_low_nibble_is_always_clear)
{
    const uint8_t pop_af[] = { 0xF1 };
    const uint8_t nop[]    = { 0x00 };

    emu_reset();
    load_op(pop_af, sizeof(pop_af));
    CPU_REG.SP = 0xDFF0;
    op_set_mem_map_16(0xDFF0, 0xFFFF);
    step();

    load_op(nop, sizeof(nop));
    step();

    EXPECT_EQ_X(CPU_REG.F & 0x0F, 0x00);
}

TEST(add_hl_sets_carry_from_bit_15)
{
    const uint8_t add_hl_bc[] = { 0x09 };

    emu_reset();
    load_op(add_hl_bc, sizeof(add_hl_bc));
    CPU_REG.HL = 0xFFFF;
    CPU_REG.BC = 0x0001;
    step();

    EXPECT_EQ_X(CPU_REG.HL, 0x0000);
    EXPECT(GET_CARRY_FLAG == 1);
    EXPECT(GET_HALF_CARRY_FLAG == 1);
    EXPECT(GET_ADD_SUB_FLAG == 0);
}

TEST(sixteen_bit_inc_does_not_touch_flags)
{
    const uint8_t inc_bc[] = { 0x03 };

    emu_reset();
    load_op(inc_bc, sizeof(inc_bc));
    CPU_REG.BC = 0x00FF;
    CPU_REG.F  = F_Z | F_C;
    step();

    EXPECT_EQ_X(CPU_REG.BC, 0x0100);
    expect_flags("INC BC", F_Z | F_C);
}

/* --- control flow ------------------------------------------------------ */

TEST(relative_jump_is_signed)
{
    const uint8_t jr_back[] = { 0x18, 0xFC };     /* JR -4 */
    uint16_t at;

    emu_reset();
    at = emu_place_code(jr_back, sizeof(jr_back));
    step();

    /* PC advances past the 2-byte instruction, then the offset applies. */
    EXPECT_EQ_X(CPU_REG.PC, (uint16_t)(at + 2 - 4));
}

TEST(call_pushes_the_return_address)
{
    const uint8_t call[] = { 0xCD, 0x00, 0xD5 };  /* CALL 0xD500 */
    uint16_t at;

    emu_reset();
    at = emu_place_code(call, sizeof(call));
    CPU_REG.SP = 0xDFF0;
    step();

    EXPECT_EQ_X(CPU_REG.PC, 0xD500);
    EXPECT_EQ_X(CPU_REG.SP, 0xDFEE);
    EXPECT_EQ_X(op_get_mem_map_16(0xDFEE), (uint16_t)(at + 3));
}

TEST(rst_jumps_to_its_vector)
{
    const uint8_t rst_28[] = { 0xEF };            /* RST 0x28 */

    emu_reset();
    load_op(rst_28, sizeof(rst_28));
    step();

    EXPECT_EQ_X(CPU_REG.PC, RESTART_ADDR_28);
}

/* A length-0 table entry leaves PC on the offending byte forever. */
TEST(illegal_opcode_does_not_wedge_the_program_counter)
{
    const uint8_t illegal[] = { 0xD3 };
    uint16_t at;

    emu_reset();
    at = emu_place_code(illegal, sizeof(illegal));
    step();

    if (CPU_REG.PC == at) {
        test_record_failure(__FILE__, __LINE__,
            "PC stayed at 0x%04X after illegal opcode 0xD3: the CPU is stuck",
            at);
    }
}

/* --- interrupts and power modes ---------------------------------------- */

struct vector_case {
    uint8_t  flag;
    uint16_t vector;
    const char *name;
};

TEST(interrupts_dispatch_to_the_right_vector)
{
    static const struct vector_case cases[] = {
        { INTERRUPT_V_BLANK,  VBLANK_INTR_ADDR,         "v-blank"   },
        { INTERRUPT_LCD_STAT, LCDC_STAT_INTR_ADDR,      "lcd stat"  },
        { INTERRUPT_TIMER,    TIMER_OVERFLOW_INTR_ADDR, "timer"     },
        { INTERRUPT_SERIAL,   SERIAL_COMPLETE_INTR_ADDR,"serial"    },
        { INTERRUPT_JOYPAD,   HTL_P10_P13_INTR_ADDR,    "joypad"    },
    };
    unsigned i;

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        emu_reset();
        CPU_REG.SP = 0xDFF0;
        CPU_REG.PC = 0xC500;
        set_interrupts(true);
        set_mem_map_8(INTERRUPT_ENABLE, cases[i].flag);
        set_mem_map_8(INTERRUPT_FLAGS, cases[i].flag);

        current_cycle_time = 200;      /* keep the dispatcher from stepping */
        cpu_cycles         = 0;
        gb_cpu(0);

        if (CPU_REG.PC != cases[i].vector) {
            test_record_failure(__FILE__, __LINE__,
                "%s interrupt: PC 0x%04X, want 0x%04X",
                cases[i].name, CPU_REG.PC, cases[i].vector);
            continue;
        }
        if (CPU_REG.SP != 0xDFEE) {
            test_record_failure(__FILE__, __LINE__,
                "%s interrupt: SP 0x%04X, want 0xDFEE", cases[i].name, CPU_REG.SP);
        }
        if (op_get_mem_map_16(0xDFEE) != 0xC500) {
            test_record_failure(__FILE__, __LINE__,
                "%s interrupt: pushed 0x%04X, want 0xC500",
                cases[i].name, op_get_mem_map_16(0xDFEE));
        }
        if (get_mem_map_bit(INTERRUPT_FLAGS, cases[i].flag) != 0) {
            test_record_failure(__FILE__, __LINE__,
                "%s interrupt: request flag still set", cases[i].name);
        }
    }
}

TEST(interrupts_do_not_dispatch_while_disabled)
{
    emu_reset();
    CPU_REG.SP = 0xDFF0;
    CPU_REG.PC = 0xC500;
    set_interrupts(false);
    set_mem_map_8(INTERRUPT_ENABLE, INTERRUPT_V_BLANK);
    set_mem_map_8(INTERRUPT_FLAGS, INTERRUPT_V_BLANK);

    current_cycle_time = 200;
    cpu_cycles         = 0;
    gb_cpu(0);

    EXPECT_EQ_X(CPU_REG.PC, 0xC500);
}

TEST(an_unenabled_interrupt_does_not_dispatch)
{
    emu_reset();
    CPU_REG.SP = 0xDFF0;
    CPU_REG.PC = 0xC500;
    set_interrupts(true);
    set_mem_map_8(INTERRUPT_ENABLE, 0x00);
    set_mem_map_8(INTERRUPT_FLAGS, INTERRUPT_V_BLANK);

    current_cycle_time = 200;
    cpu_cycles         = 0;
    gb_cpu(0);

    EXPECT_EQ_X(CPU_REG.PC, 0xC500);
}

TEST(halt_wakes_on_a_pending_interrupt)
{
    emu_reset();
    power_mode = PWR_HALT;
    set_interrupts(false);
    set_mem_map_8(INTERRUPT_ENABLE, INTERRUPT_TIMER);
    set_mem_map_bit(INTERRUPT_FLAGS, INTERRUPT_TIMER);

    gb_cpu(1000);

    EXPECT_EQ_U(power_mode, PWR_NORMAL);
}

TEST(halt_stays_asleep_without_a_pending_interrupt)
{
    emu_reset();
    power_mode = PWR_HALT;
    set_mem_map_8(INTERRUPT_ENABLE, 0x00);
    set_mem_map_8(INTERRUPT_FLAGS, 0x00);

    gb_cpu(1000);

    EXPECT_EQ_U(power_mode, PWR_HALT);
}

/* STOP is left by a joypad input, and that does not depend on IME. A STOP that
 * can only be left through interrupt dispatch wedges the CPU for good when a
 * game stops with interrupts disabled. */
TEST(stop_mode_can_be_left_by_a_joypad_interrupt)
{
    const uint8_t stop[] = { 0x10, 0x00 };
    int i;

    emu_reset();
    load_op(stop, sizeof(stop));
    step();
    CHECK_EQ_U(power_mode, PWR_STOP);

    set_interrupts(false);
    set_mem_map_8(INTERRUPT_ENABLE, INTERRUPT_JOYPAD);
    set_mem_map_bit(INTERRUPT_FLAGS, INTERRUPT_JOYPAD);

    for (i = 0; i < 100; i++) {
        gb_cpu(10000);
    }

    EXPECT(power_mode != PWR_STOP);
}
