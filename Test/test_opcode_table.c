/* test_opcode_table.c - the opcode table as data, and the cycle counts the
 * handlers actually return, both checked against reference_opcodes.h.
 *
 * These are table-sweep tests on purpose: a single case covering all 256
 * opcodes catches a whole class of transcription error at once, which is how
 * cycle-count and instruction-length mistakes tend to arrive.
 */
#include "test.h"
#include "test_support.h"
#include "reference_opcodes.h"

#include "gb_opcodes.h"

/* Condition code encoded in a conditional opcode: 0=NZ 1=Z 2=NC 3=C */
static void set_condition(int cond, int taken)
{
    CPU_REG.F = 0x00;
    switch (cond) {
    case 0: if (!taken) { CPU_REG.F |= ZERO_FLAG;  } break;  /* NZ */
    case 1: if ( taken) { CPU_REG.F |= ZERO_FLAG;  } break;  /* Z  */
    case 2: if (!taken) { CPU_REG.F |= CARRY_FLAG; } break;  /* NC */
    case 3: if ( taken) { CPU_REG.F |= CARRY_FLAG; } break;  /* C  */
    default: break;
    }
}

/* Which condition code, if any, an opcode branches on. -1 == unconditional. */
static int opcode_condition(uint8_t op)
{
    switch (op) {
    case 0x20: case 0x28: case 0x30: case 0x38:            /* JR cc   */
        return (op >> 3) - 4;
    case 0xC0: case 0xC8: case 0xD0: case 0xD8:            /* RET cc  */
    case 0xC2: case 0xCA: case 0xD2: case 0xDA:            /* JP cc   */
    case 0xC4: case 0xCC: case 0xD4: case 0xDC:            /* CALL cc */
        return (op >> 3) & 0x03;
    default:
        return -1;
    }
}

/* Run one opcode in isolation and return the cycle count it reports. */
static uint8_t run_opcode_cycles(uint8_t op, uint8_t operand_1, uint8_t operand_2)
{
    uint8_t bytes[3];
    uint16_t at;

    bytes[0] = op;
    bytes[1] = operand_1;
    bytes[2] = operand_2;

    at = emu_place_code(bytes, 3);

    /* Park the indirect registers in work RAM so (HL)/(BC)/(DE) accesses and
     * stack pushes land in plain storage rather than in cart ROM or in an MBC
     * control register. */
    CPU_REG.HL = 0xD000;
    CPU_REG.BC = 0xD010;
    CPU_REG.DE = 0xD020;
    CPU_REG.SP = 0xDFF0;
    power_mode = PWR_NORMAL;

    return opcode_table[op].operation(at);
}

TEST(instruction_length_matches_reference)
{
    int op;

    for (op = 0; op < 256; op++) {
        const struct ref_opcode *ref = &reference_opcodes[op];

        if (opcode_table[op].length != ref->length) {
            test_record_failure(__FILE__, __LINE__,
                "opcode 0x%02X (%s): length %u, want %u",
                op, opcode_table[op].description,
                opcode_table[op].length, ref->length);
        }
    }
}

/* A zero length means the dispatcher cannot advance PC past the instruction,
 * so the CPU spins on that byte forever. No opcode may have length 0, illegal
 * ones included. */
TEST(no_opcode_has_zero_length)
{
    int op;

    for (op = 0; op < 256; op++) {
        if (opcode_table[op].length == 0) {
            test_record_failure(__FILE__, __LINE__,
                "opcode 0x%02X (%s) has length 0: PC can never advance past it",
                op, opcode_table[op].description);
        }
    }
}

/* Every slot must be dispatchable: a NULL handler is an immediate crash. */
TEST(every_opcode_has_a_handler)
{
    int op;

    for (op = 0; op < 256; op++) {
        if (opcode_table[op].operation == NULL) {
            test_record_failure(__FILE__, __LINE__,
                "opcode 0x%02X has a NULL handler", op);
        }
    }
}

/* The dispatcher gates on cpu_cycles > cycles * CPU_CYCLE_TIME, so a zero
 * cycle count makes the gate always true and the opcode free. */
TEST(no_opcode_costs_zero_cycles)
{
    int op;

    for (op = 0; op < 256; op++) {
        if ((opcode_table[op].cycles & CYCLE_MASK_1) == 0 &&
            (opcode_table[op].cycles & CYCLE_MASK_2) == 0) {
            test_record_failure(__FILE__, __LINE__,
                "opcode 0x%02X (%s) costs 0 cycles",
                op, opcode_table[op].description);
        }
    }
}

/* The cycle count a handler returns is what the CPU actually bills, so this is
 * the test that matters for timing. Unconditional opcodes only. */
TEST(unconditional_opcode_cycles_match_reference)
{
    int op;

    for (op = 0; op < 256; op++) {
        const struct ref_opcode *ref = &reference_opcodes[op];
        uint8_t got;

        if (ref->illegal || ref->cycles_untaken != 0) {
            continue;                       /* covered by other cases */
        }
        if (op == 0xCB) {
            continue;                       /* cost depends on the sub-opcode */
        }

        emu_reset();
        got = run_opcode_cycles((uint8_t)op, 0x00, 0xD0);

        if (got != ref->cycles_taken) {
            test_record_failure(__FILE__, __LINE__,
                "opcode 0x%02X (%s): returned %u cycles, want %u",
                op, opcode_table[op].description, got, ref->cycles_taken);
        }
    }
}

TEST(conditional_opcode_cycles_match_reference)
{
    int op;

    for (op = 0; op < 256; op++) {
        const struct ref_opcode *ref = &reference_opcodes[op];
        int cond = opcode_condition((uint8_t)op);
        uint8_t got;

        if (ref->cycles_untaken == 0 || cond < 0) {
            continue;
        }

        emu_reset();
        set_condition(cond, 1);
        got = run_opcode_cycles((uint8_t)op, 0x00, 0xD0);
        if (got != ref->cycles_taken) {
            test_record_failure(__FILE__, __LINE__,
                "opcode 0x%02X (%s) taken: returned %u cycles, want %u",
                op, opcode_table[op].description, got, ref->cycles_taken);
        }

        emu_reset();
        set_condition(cond, 0);
        got = run_opcode_cycles((uint8_t)op, 0x00, 0xD0);
        if (got != ref->cycles_untaken) {
            test_record_failure(__FILE__, __LINE__,
                "opcode 0x%02X (%s) not taken: returned %u cycles, want %u",
                op, opcode_table[op].description, got, ref->cycles_untaken);
        }
    }
}

/* CB-prefixed ops: register forms cost 2 M-cycles, (HL) forms cost 4, except
 * BIT n,(HL) which is 3 because it does not write back. */
static uint8_t cb_reference_cycles(uint8_t sub)
{
    uint8_t x = (uint8_t)GET_OPCODE_X(sub);
    uint8_t z = (uint8_t)GET_OPCODE_Z(sub);

    if (z != 0x06) {
        return 2;
    }
    return (x == 0x01) ? 3 : 4;        /* BIT n,(HL) is 3, the rest are 4 */
}

TEST(cb_prefixed_opcode_cycles_match_reference)
{
    int sub;

    for (sub = 0; sub < 256; sub++) {
        uint8_t want = cb_reference_cycles((uint8_t)sub);
        uint8_t got;

        emu_reset();
        got = run_opcode_cycles(0xCB, (uint8_t)sub, 0x00);

        if (got != want) {
            test_record_failure(__FILE__, __LINE__,
                "CB 0x%02X: returned %u cycles, want %u", sub, got, want);
        }
    }
}

/* BIT is a pure test. If it writes its operand back and HL happens to point
 * into cart ROM, the write is routed to the MBC and can switch banks. */
TEST(bit_opcode_does_not_write_memory)
{
    uint8_t bytes[2];
    uint16_t at;

    /* An MBC1 cart, so a stray write into ROM actually reaches the mapper and
     * the test can observe it. A ROM-only cart discards it and passes either
     * way. */
    emu_reset_with_cart(4, TYPE_ROM_MBC1, ROM_SIZE_64KB, RAM_SIZE_0);

    bytes[0] = 0xCB;
    bytes[1] = 0x46;                   /* BIT 0,(HL) */
    at = emu_place_code(bytes, 2);

    CPU_REG.HL = 0x2000;               /* MBC1 ROM bank-number register */
    g_rom_bank = 0x01;

    opcode_table[0xCB].operation(at);

    EXPECT_EQ_X(g_rom_bank, 0x01);
}
