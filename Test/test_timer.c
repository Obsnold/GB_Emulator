/* test_timer.c - DIV and TIMA rates, reload and interrupt.
 *
 * gb_timer() is driven by a wall-clock delta in nanoseconds, so these tests
 * feed a known total in a given chunk size and check the register advanced the
 * right number of times. Chunk size matters: an accumulator that is only
 * updated inside its own "has it elapsed yet" branch silently drops every
 * delta smaller than one tick, so each rate is checked with small chunks
 * (the realistic case) as well as large ones.
 */
#include "test.h"
#include "test_support.h"

#include "gb_timer.h"
#include "gb_mem_map.h"

#define DIV_PERIOD_NS   61035UL         /* 16384 Hz */
#define TIMA_1024_NS   244141UL         /*  4096 Hz */
#define TIMA_16_NS       3815UL         /* 262144 Hz */
#define TIMA_64_NS      15259UL         /*  65536 Hz */
#define TIMA_256_NS     61035UL         /*  16384 Hz */

/* Feed total_ns to the timer in fixed chunks, counting 8-bit register wraps so
 * the caller sees a full tick count rather than a value mod 256. */
static unsigned long run_timer(uint16_t reg, unsigned long total_ns,
                               unsigned long chunk_ns)
{
    unsigned long fed = 0;
    unsigned long ticks = 0;
    uint8_t prev = get_mem_map_8(reg);

    while (fed < total_ns) {
        unsigned long step = (total_ns - fed < chunk_ns) ? total_ns - fed : chunk_ns;
        uint8_t now;

        gb_timer(step);
        fed += step;

        now = get_mem_map_8(reg);
        ticks += (uint8_t)(now - prev);     /* modular difference */
        prev = now;
    }
    return ticks;
}

TEST(div_increments_at_16384hz_with_small_deltas)
{
    unsigned long ticks;

    emu_reset();

    /* 100 ms fed in 5 us slices, which is the order of a real main-loop step. */
    ticks = run_timer(TIMER_DIV, 100000000UL, 5000UL);

    EXPECT_IN_RANGE(ticks, 1630, 1650);     /* 100 ms / 61035 ns = 1638 */
}

TEST(div_increments_at_16384hz_with_large_deltas)
{
    unsigned long ticks;

    emu_reset();

    ticks = run_timer(TIMER_DIV, 100000000UL, 250000UL);

    EXPECT_IN_RANGE(ticks, 1630, 1650);
}

/* The rate must not depend on how the same elapsed time is chopped up. */
TEST(div_rate_is_independent_of_chunk_size)
{
    unsigned long small, large;

    emu_reset();
    small = run_timer(TIMER_DIV, 50000000UL, 2000UL);

    emu_reset();
    large = run_timer(TIMER_DIV, 50000000UL, 500000UL);

    EXPECT_IN_RANGE((double)small - (double)large, -20, 20);
}

/* A long stall must bill the elapsed time once, not compound it. */
TEST(div_does_not_run_away_after_a_long_stall)
{
    unsigned long before, after;

    emu_reset();

    run_timer(TIMER_DIV, 10000000UL, 5000UL);       /* warm up   */
    gb_timer(500000000UL);                          /* 500 ms stall */

    before = get_mem_map_8(TIMER_DIV);
    after  = run_timer(TIMER_DIV, 10000000UL, 5000UL);
    (void)before;

    /* 10 ms after the stall is still 10 ms: ~164 ticks, not thousands. */
    EXPECT_IN_RANGE(after, 155, 175);
}

static void enable_tima(uint8_t clock_select)
{
    set_mem_map_8(TIMER_TAC, (uint8_t)(TAC_ENABLE | clock_select));
    set_mem_map_8(TIMER_TIMA, 0x00);
    set_mem_map_8(TIMER_TMA, 0x00);
}

TEST(tima_counts_at_4096hz)
{
    unsigned long ticks;

    emu_reset();
    enable_tima(TAC_CLOCK_1024);

    ticks = run_timer(TIMER_TIMA, 100000000UL, 5000UL);

    EXPECT_IN_RANGE(ticks, 405, 415);       /* 100 ms / 244141 ns = 409 */
}

TEST(tima_counts_at_262144hz)
{
    unsigned long ticks;

    emu_reset();
    enable_tima(TAC_CLOCK_16);

    ticks = run_timer(TIMER_TIMA, 10000000UL, 1000UL);

    EXPECT_IN_RANGE(ticks, 2590, 2630);     /* 10 ms / 3815 ns = 2621 */
}

TEST(tima_counts_at_65536hz)
{
    unsigned long ticks;

    emu_reset();
    enable_tima(TAC_CLOCK_64);

    ticks = run_timer(TIMER_TIMA, 10000000UL, 2000UL);

    EXPECT_IN_RANGE(ticks, 645, 665);       /* 10 ms / 15259 ns = 655 */
}

TEST(tima_counts_at_16384hz)
{
    unsigned long ticks;

    emu_reset();
    enable_tima(TAC_CLOCK_256);

    ticks = run_timer(TIMER_TIMA, 10000000UL, 2000UL);

    EXPECT_IN_RANGE(ticks, 155, 175);       /* 10 ms / 61035 ns = 164 */
}

TEST(tima_does_not_count_while_disabled)
{
    unsigned long ticks;

    emu_reset();
    set_mem_map_8(TIMER_TAC, TAC_CLOCK_1024);       /* enable bit clear */
    set_mem_map_8(TIMER_TIMA, 0x00);

    ticks = run_timer(TIMER_TIMA, 100000000UL, 5000UL);

    EXPECT_EQ_U(ticks, 0);
}

/* On overflow TIMA reloads from TMA and raises the timer interrupt. */
TEST(tima_overflow_reloads_tma_and_raises_interrupt)
{
    unsigned long fed = 0;

    emu_reset();
    enable_tima(TAC_CLOCK_16);
    set_mem_map_8(TIMER_TMA, 0x42);
    set_mem_map_8(TIMER_TIMA, 0xFF);
    clear_mem_map_bit(INTERRUPT_FLAGS, INTERRUPT_TIMER);

    /* Advance just past one tick so TIMA goes 0xFF -> overflow. */
    while (fed < TIMA_16_NS * 3) {
        gb_timer(1000UL);
        fed += 1000UL;
    }

    EXPECT(get_mem_map_bit(INTERRUPT_FLAGS, INTERRUPT_TIMER) != 0);
    EXPECT(get_mem_map_8(TIMER_TIMA) >= 0x42);
}

/* Changing TAC must change the rate, which pins the clock-select decode. */
TEST(tac_clock_select_changes_the_rate)
{
    unsigned long slow, fast;

    emu_reset();
    enable_tima(TAC_CLOCK_1024);
    slow = run_timer(TIMER_TIMA, 10000000UL, 1000UL);

    emu_reset();
    enable_tima(TAC_CLOCK_16);
    fast = run_timer(TIMER_TIMA, 10000000UL, 1000UL);

    /* 1024 vs 16 is a factor of 64. */
    EXPECT(fast > slow);
    if (slow > 0) {
        EXPECT_IN_RANGE((double)fast / (double)slow, 48.0, 80.0);
    }
}
