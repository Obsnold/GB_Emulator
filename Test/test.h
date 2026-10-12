/* test.h - minimal test harness for the GB emulator.
 *
 * No external dependencies; tests self-register via constructor attributes, so
 * adding a test is one TEST() block and nothing else.
 *
 *   TEST(my_thing) {
 *       CHECK_EQ_X(get_mem_map_8(0xFFFF), 0x00);   // fails -> aborts this test
 *       EXPECT_EQ_U(count, 3);                     // fails -> records, continues
 *   }
 *
 * CHECK_* stops the current test at the first failure. EXPECT_* records the
 * failure and keeps going, which is what you want when one test sweeps a whole
 * table and you need to see every discrepancy rather than just the first.
 */
#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <stdint.h>

typedef void (*test_fn)(void);

void test_register(const char *name, const char *file, test_fn fn);
void test_record_failure(const char *file, int line, const char *fmt, ...);
int  test_run_all(const char *filter, int fork_each);

/* Per-test failure budget: a sweep over 256 opcodes should not print 256 lines. */
#define TEST_MAX_REPORTED_FAILURES 12

#define TEST(name)                                                            \
    static void name(void);                                                   \
    static void __attribute__((constructor)) test_reg_##name(void) {          \
        test_register(#name, __FILE__, name);                                 \
    }                                                                         \
    static void name(void)

#define EXPECT(cond)                                                          \
    do {                                                                      \
        if (!(cond)) {                                                        \
            test_record_failure(__FILE__, __LINE__, "expected: %s", #cond);   \
        }                                                                     \
    } while (0)

#define CHECK(cond)                                                           \
    do {                                                                      \
        if (!(cond)) {                                                        \
            test_record_failure(__FILE__, __LINE__, "expected: %s", #cond);   \
            return;                                                           \
        }                                                                     \
    } while (0)

/* Unsigned decimal comparison. */
#define EXPECT_EQ_U(actual, expected)                                         \
    do {                                                                      \
        unsigned long long a_ = (unsigned long long)(actual);                 \
        unsigned long long e_ = (unsigned long long)(expected);               \
        if (a_ != e_) {                                                       \
            test_record_failure(__FILE__, __LINE__,                           \
                "%s: got %llu, want %llu", #actual, a_, e_);                  \
        }                                                                     \
    } while (0)

#define CHECK_EQ_U(actual, expected)                                          \
    do {                                                                      \
        unsigned long long a_ = (unsigned long long)(actual);                 \
        unsigned long long e_ = (unsigned long long)(expected);               \
        if (a_ != e_) {                                                       \
            test_record_failure(__FILE__, __LINE__,                           \
                "%s: got %llu, want %llu", #actual, a_, e_);                  \
            return;                                                           \
        }                                                                     \
    } while (0)

/* Hex comparison - the default for registers, addresses and memory contents. */
#define EXPECT_EQ_X(actual, expected)                                         \
    do {                                                                      \
        unsigned long long a_ = (unsigned long long)(actual);                 \
        unsigned long long e_ = (unsigned long long)(expected);               \
        if (a_ != e_) {                                                       \
            test_record_failure(__FILE__, __LINE__,                           \
                "%s: got 0x%llX, want 0x%llX", #actual, a_, e_);              \
        }                                                                     \
    } while (0)

#define CHECK_EQ_X(actual, expected)                                          \
    do {                                                                      \
        unsigned long long a_ = (unsigned long long)(actual);                 \
        unsigned long long e_ = (unsigned long long)(expected);               \
        if (a_ != e_) {                                                       \
            test_record_failure(__FILE__, __LINE__,                           \
                "%s: got 0x%llX, want 0x%llX", #actual, a_, e_);              \
            return;                                                           \
        }                                                                     \
    } while (0)

/* Range check, for timing assertions that cannot be exact. */
#define EXPECT_IN_RANGE(actual, lo, hi)                                       \
    do {                                                                      \
        double a_ = (double)(actual);                                         \
        if (a_ < (double)(lo) || a_ > (double)(hi)) {                         \
            test_record_failure(__FILE__, __LINE__,                           \
                "%s: got %g, want %g..%g", #actual, a_,                       \
                (double)(lo), (double)(hi));                                  \
        }                                                                     \
    } while (0)

/* Annotate a test with the bug it pins down, so failures are self-explaining. */
#define TEST_NOTE(fmt, ...) \
    fprintf(stderr, "      note: " fmt "\n", ##__VA_ARGS__)

#endif /* TEST_H */
