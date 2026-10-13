/* test_timing.c - the emulator's clock model.
 *
 * The CPU and the PPU both bill work in the cycle units used by the opcode
 * table, and both convert those units to wall-clock nanoseconds. If the two
 * conversion factors disagree, the machine still runs and still draws, but the
 * CPU and the display are on different clocks and nothing is the right speed.
 * These tests measure each side independently so a mismatch says which half
 * is wrong, rather than just "timing is off".
 */
#include "test.h"
#include "test_support.h"

#include "gb_cpu.h"
#include "gb_mem_map.h"
#include "gb_ppu.h"
#include "gb_timer.h"

/* A DMG frame is 154 scanlines x 456 dots = 70224 dots = 17556 M-cycles,
 * which at 4.194304 MHz takes 16.742 ms. */
#define DOTS_PER_FRAME      70224UL
#define MCYCLES_PER_FRAME   (DOTS_PER_FRAME / 4)       /* 17556 */
#define FRAME_NS            16742706UL

static const uint8_t self_loop[2] = { 0x18, 0xFE };    /* JR -2 */

/* Recover the CPU's nanoseconds-per-cycle-unit without reading its private
 * constant: the dispatcher retires an instruction once the accumulated delta
 * exceeds cycles * ns_per_unit, so binary-search that threshold.
 */
#define PROBE_UNITS 200     /* fits in current_cycle_time (uint8_t) */

static unsigned long measure_cpu_ns_per_cycle_unit(void)
{
    unsigned long lo = 1, hi = 8000000;

    while (lo < hi) {
        unsigned long mid = lo + (hi - lo) / 2;

        emu_reset();
        emu_place_code(self_loop, sizeof(self_loop));

        /* Claim the instruction in flight costs PROBE_UNITS cycle units, then
         * offer mid nanoseconds and see whether the dispatcher retires it. */
        current_cycle_time = PROBE_UNITS;
        cpu_cycles         = 0;
        gb_cpu(mid);

        if (cpu_cycles < mid) {
            hi = mid;                      /* retired: threshold is at or below */
        } else {
            lo = mid + 1;                  /* did not retire: threshold is above */
        }
    }

    /* The gate is `accumulated > units * ns_per_unit`, so the smallest delta
     * that retires is units * ns_per_unit + 1. */
    return (lo - 1) / PROBE_UNITS;
}

TEST(cpu_and_ppu_agree_on_the_duration_of_one_cycle_unit)
{
    unsigned long cpu_ns = measure_cpu_ns_per_cycle_unit();

    /* CYCLE_TIME is the PPU's figure and is the correct one: an M-cycle is
     * 4 dots at 4.194304 MHz, i.e. 954 ns. 238 ns is one dot, not one
     * M-cycle, and the opcode table counts M-cycles. */
    EXPECT_EQ_U(cpu_ns, CYCLE_TIME);
}

TEST(one_cycle_unit_is_954_nanoseconds)
{
    EXPECT_EQ_U(CYCLE_TIME, 954);
}

/* Run the PPU until the scanline counter wraps, and report how long that took
 * and how much CPU work was retired in the meantime. */
struct frame_stats {
    unsigned long ns_elapsed;
    unsigned long cpu_units_retired;
    int           completed;
};

/* A fine step keeps the measurement precise. It used to be load-bearing: the
 * dispatcher retired at most one instruction per gb_cpu() call, so a coarse
 * slice capped the retirement rate and understated the mismatch. The catch-up
 * loop drains the accumulator now, so the rate no longer depends on the step
 * size - this just keeps the frame boundary sharp. */
static struct frame_stats run_one_frame(unsigned long ns_per_unit)
{
    const unsigned long step_ns = 50;
    const unsigned long guard   = 2000000;     /* 100 ms of 50 ns steps */
    struct frame_stats s = { 0, 0, 0 };
    unsigned long retired_ns = 0;
    unsigned long i;
    uint8_t prev_ly;

    emu_reset();
    emu_place_code(self_loop, sizeof(self_loop));
    set_mem_map_bit(LCD_CTRL, LCD_CTRL_ENABLE);

    /* Align to the start of a frame. */
    for (i = 0; i < guard; i++) {
        uint8_t ly = get_mem_map_8(LCD_LY);
        prev_ly = ly;
        gb_timer(step_ns);
        gb_cpu(step_ns);
        ppu(step_ns);
        if (prev_ly != 0 && get_mem_map_8(LCD_LY) == 0) {
            break;
        }
    }

    prev_ly = get_mem_map_8(LCD_LY);
    for (i = 0; i < guard; i++) {
        unsigned long before = cpu_cycles;
        uint8_t ly;

        gb_timer(step_ns);
        gb_cpu(step_ns);
        ppu(step_ns);

        /* cpu_cycles grew by step_ns and shrank by whatever was retired. */
        retired_ns += step_ns - (cpu_cycles - before);
        s.ns_elapsed += step_ns;

        ly = get_mem_map_8(LCD_LY);
        if (prev_ly != 0 && ly == 0) {
            s.completed = 1;
            break;
        }
        prev_ly = ly;
    }

    s.cpu_units_retired = ns_per_unit ? retired_ns / ns_per_unit : 0;
    return s;
}

/* The PPU's own scanline budget: this is the half that is currently correct,
 * so it isolates the mismatch to the CPU side. */
TEST(a_frame_takes_one_sixtieth_of_a_second)
{
    struct frame_stats s = run_one_frame(CYCLE_TIME);

    CHECK(s.completed);
    EXPECT_IN_RANGE(s.ns_elapsed, FRAME_NS * 0.95, FRAME_NS * 1.05);
}

/* The headline integration assertion: over one displayed frame the CPU must
 * retire one frame's worth of instruction cycles. */
TEST(cpu_retires_one_frame_of_cycles_per_displayed_frame)
{
    unsigned long ns_per_unit = measure_cpu_ns_per_cycle_unit();
    struct frame_stats s;

    CHECK(ns_per_unit > 0);
    s = run_one_frame(ns_per_unit);

    CHECK(s.completed);
    EXPECT_IN_RANGE(s.cpu_units_retired,
                    MCYCLES_PER_FRAME * 0.95, MCYCLES_PER_FRAME * 1.05);
}

/* Same measurement expressed as the ratio people actually notice: if this is
 * 4.0 the game logic runs at four times its intended speed. The authoritative
 * check is cpu_and_ppu_agree_on_the_duration_of_one_cycle_unit above; this one
 * is the end-to-end confirmation. */
TEST(cpu_speed_relative_to_the_display_is_1x)
{
    unsigned long ns_per_unit = measure_cpu_ns_per_cycle_unit();
    struct frame_stats s;
    double ratio;

    CHECK(ns_per_unit > 0);
    s = run_one_frame(ns_per_unit);
    CHECK(s.completed);

    ratio = (double)s.cpu_units_retired / (double)MCYCLES_PER_FRAME;
    EXPECT_IN_RANGE(ratio, 0.95, 1.05);
}
