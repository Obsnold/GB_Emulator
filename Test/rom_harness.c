/* rom_harness.c - run the emulator core headlessly against a ROM image.
 *
 * Two modes:
 *
 *   rom_harness --fuzz N [--seed S]
 *       Generate N pseudo-random cart images and run each for a bounded number
 *       of steps. Each ROM runs in a forked child, so a crash is reported with
 *       the seed that produced it instead of taking the harness down with it.
 *       Built with -fsanitize=address this is the cheapest way to find
 *       unchecked bank numbers, short headers and stray writes.
 *
 *   rom_harness ROM.gb [--frames N]
 *       Run one ROM and watch for either of the two conventions test suites
 *       use to report a result:
 *
 *         blargg  (default) writes its verdict as text to the link port, so
 *                 the run ends when "Passed" or "Failed" appears there.
 *         mooneye (--mooneye) ends on a LD B,B breakpoint with the Fibonacci
 *                 values 3,5,8,13,21,34 in B,C,D,E,H,L for a pass, and on
 *                 LD D,D for a failure.
 *
 *       The convention has to be stated rather than guessed: LD B,B and LD D,D
 *       are ordinary instructions, and blargg's 06-ld r,r and instr_timing both
 *       execute them while testing register loads. Treating that as a mooneye
 *       breakpoint ends the run early and reports a false failure.
 *
 * Exit status is 0 on success, non-zero otherwise.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "gb_cart.h"
#include "gb_cpu.h"
#include "gb_mem_map.h"
#include "gb_ppu.h"
#include "gb_serial.h"
#include "gb_timer.h"

#define STEP_NS          1000UL
#define STEPS_PER_FRAME  16743UL          /* ~16.74 ms at 1 us per step */
#define FUZZ_STEPS       200000UL         /* ~12 frames of emulated time */

#define OPCODE_LD_B_B    0x40             /* mooneye: test finished  */
#define OPCODE_LD_D_D    0x52             /* mooneye: soft failure   */

enum run_result {
    RUN_TIMEOUT = 0,
    RUN_PASS,
    RUN_FAIL
};

static void emu_boot(void)
{
    cpu_init();
    init_mem_map();
    gb_serial_init();
}

static void emu_step(void)
{
    gb_timer(STEP_NS);
    gb_cpu(STEP_NS);
    ppu(STEP_NS);
}

/* mooneye signals a pass with Fibonacci values in the register file. */
static int mooneye_registers_pass(void)
{
    return CPU_REG.B == 3  && CPU_REG.C == 5  &&
           CPU_REG.D == 8  && CPU_REG.E == 13 &&
           CPU_REG.H == 21 && CPU_REG.L == 34;
}

enum suite { SUITE_BLARGG, SUITE_MOONEYE };

static enum run_result run_until_verdict(unsigned long max_steps,
                                         enum suite suite,
                                         const char **how)
{
    unsigned int last_len = 0;
    unsigned long i;

    *how = "no verdict";

    for (i = 0; i < max_steps; i++) {
        if (suite == SUITE_MOONEYE) {
            uint8_t op = get_mem_map_8(CPU_REG.PC);

            if (op == OPCODE_LD_B_B) {
                *how = "mooneye breakpoint";
                return mooneye_registers_pass() ? RUN_PASS : RUN_FAIL;
            }
            if (op == OPCODE_LD_D_D) {
                *how = "mooneye failure opcode";
                return RUN_FAIL;
            }
        } else {
            /* blargg: text on the link port. Scanned only when something new
             * has arrived, so this is not a strstr over a growing buffer on
             * every step. */
            unsigned int len = gb_serial_length();

            if (len != last_len) {
                const char *text = gb_serial_text();
                last_len = len;
                if (strstr(text, "Passed") != NULL) {
                    *how = "blargg serial output";
                    return RUN_PASS;
                }
                if (strstr(text, "Failed") != NULL) {
                    *how = "blargg serial output";
                    return RUN_FAIL;
                }
            }
        }

        emu_step();
    }
    return RUN_TIMEOUT;
}

/* --- fuzzing ----------------------------------------------------------- */

static uint32_t xorshift(uint32_t *state)
{
    uint32_t x = *state;

    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x ? x : 0x1234567u;
    return *state;
}

/* Cart types the memory map will actually route through an MBC. */
static const uint8_t fuzz_cart_types[] = {
    TYPE_ROM_NO_MBC, TYPE_ROM_MBC1, TYPE_ROM_MBC1_RAM, TYPE_ROM_MBC1_RAM_BAT,
    TYPE_ROM_MBC2, TYPE_ROM_MBC2_BAT, TYPE_ROM_MBC3, TYPE_ROM_MBC3_RAM_BAT,
    TYPE_ROM_MBC5, TYPE_ROM_MBC5_RAM_BAT
};

static const uint8_t fuzz_rom_sizes[] = {
    ROM_SIZE_32KB, ROM_SIZE_64KB, ROM_SIZE_128KB, ROM_SIZE_256KB
};

static const uint8_t fuzz_ram_sizes[] = {
    RAM_SIZE_0, RAM_SIZE_2KB, RAM_SIZE_8KB, RAM_SIZE_32KB
};

/* Build a cart image whose header is plausible but whose code is noise, and
 * whose declared size deliberately may not match its real size. */
static uint8_t *fuzz_build_rom(uint32_t *rng, unsigned *size_out)
{
    static const unsigned bank_choices[] = { 2, 2, 4, 8, 16 };
    unsigned banks = bank_choices[xorshift(rng) % 5];
    unsigned size  = banks * 0x4000;
    uint8_t *rom;
    unsigned i;

    /* One in eight images is truncated mid-header to exercise the loader. */
    if ((xorshift(rng) % 8) == 0) {
        size = xorshift(rng) % 0x400;
    }

    rom = malloc(size ? size : 1);
    if (rom == NULL) {
        return NULL;
    }
    for (i = 0; i < size; i++) {
        rom[i] = (uint8_t)xorshift(rng);
    }

    if (size > CART_RAM_SIZE) {
        rom[CART_TYPE]     = fuzz_cart_types[xorshift(rng) % sizeof(fuzz_cart_types)];
        rom[CART_ROM_SIZE] = fuzz_rom_sizes[xorshift(rng) % sizeof(fuzz_rom_sizes)];
        rom[CART_RAM_SIZE] = fuzz_ram_sizes[xorshift(rng) % sizeof(fuzz_ram_sizes)];
    }

    *size_out = size;
    return rom;
}

/* Run one fuzz case in a child. Returns 0 if the child exited cleanly. */
static int fuzz_one(uint32_t seed)
{
    uint32_t rng = seed ? seed : 1;
    unsigned size = 0;
    uint8_t *rom;
    const char *path;
    char tmp[] = "/tmp/gb_fuzz_XXXXXX";
    int fd;
    pid_t pid;
    int status = 0;

    rom = fuzz_build_rom(&rng, &size);
    if (rom == NULL) {
        return 0;
    }

    fd = mkstemp(tmp);
    if (fd < 0) {
        free(rom);
        return 0;
    }
    if (size > 0 && write(fd, rom, size) != (ssize_t)size) {
        close(fd);
        unlink(tmp);
        free(rom);
        return 0;
    }
    close(fd);
    free(rom);
    path = tmp;

    pid = fork();
    if (pid < 0) {
        unlink(tmp);
        return 0;
    }
    if (pid == 0) {
        unsigned long i;

        if (gb_cart_load((char *)path) < 0) {
            _exit(0);                      /* rejecting a bad ROM is correct */
        }
        emu_boot();
        for (i = 0; i < FUZZ_STEPS; i++) {
            emu_step();
        }
        _exit(0);
    }

    waitpid(pid, &status, 0);
    unlink(tmp);

    if (WIFSIGNALED(status)) {
        fprintf(stderr,
            "  seed %-10u CRASH: killed by signal %d (%s), ROM was %u bytes\n",
            seed, WTERMSIG(status), strsignal(WTERMSIG(status)), size);
        return 1;
    }
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        fprintf(stderr,
            "  seed %-10u FAIL: exit status %d (sanitizer report above), "
            "ROM was %u bytes\n",
            seed, WEXITSTATUS(status), size);
        return 1;
    }
    return 0;
}

static int run_fuzz(unsigned long count, uint32_t base_seed)
{
    unsigned long i;
    int failures = 0;

    fprintf(stderr, "fuzzing %lu cart images from seed %u\n", count, base_seed);

    for (i = 0; i < count; i++) {
        failures += fuzz_one((uint32_t)(base_seed + i));
    }

    fprintf(stderr, "\n%lu images, %d crashed\n", count, failures);
    return failures != 0;
}

/* --- single ROM -------------------------------------------------------- */

/* Collapse the port text onto one line so a verdict fits in a log. */
static void print_serial(void)
{
    const char *text = gb_serial_text();
    int i;

    if (text[0] == '\0') {
        return;
    }
    fprintf(stderr, "      serial: ");
    for (i = 0; text[i] != '\0'; i++) {
        fputc((text[i] == '\n' || text[i] == '\r') ? ' ' : text[i], stderr);
    }
    fputc('\n', stderr);
}

static int run_rom(const char *path, unsigned long frames, enum suite suite)
{
    enum run_result result;
    const char *how = "";

    if (gb_cart_load((char *)path) < 0) {
        fprintf(stderr, "%s: could not load\n", path);
        return 2;
    }
    emu_boot();

    result = run_until_verdict(frames * STEPS_PER_FRAME, suite, &how);

    switch (result) {
    case RUN_PASS:
        fprintf(stderr, "PASS    %s  (%s)\n", path, how);
        return 0;
    case RUN_FAIL:
        fprintf(stderr, "FAIL    %s  (%s)\n", path, how);
        if (strcmp(how, "mooneye breakpoint") == 0) {
            fprintf(stderr,
                "      registers: B=%u C=%u D=%u E=%u H=%u L=%u"
                "  want 3 5 8 13 21 34\n",
                CPU_REG.B, CPU_REG.C, CPU_REG.D,
                CPU_REG.E, CPU_REG.H, CPU_REG.L);
        }
        print_serial();
        return 1;
    default:
        fprintf(stderr, "TIMEOUT %s  (%lu frames, PC=0x%04X)\n",
                path, frames, CPU_REG.PC);
        print_serial();
        return 1;
    }
}

int main(int argc, char *argv[])
{
    if (argc >= 2 && strcmp(argv[1], "--fuzz") == 0) {
        unsigned long count = (argc >= 3) ? strtoul(argv[2], NULL, 10) : 200;
        uint32_t seed = 1;

        if (argc >= 5 && strcmp(argv[3], "--seed") == 0) {
            seed = (uint32_t)strtoul(argv[4], NULL, 10);
        }
        return run_fuzz(count, seed);
    }

    if (argc >= 2) {
        unsigned long frames = 600;        /* ~10 seconds of emulated time */
        enum suite suite = SUITE_BLARGG;
        int i;

        for (i = 2; i < argc; i++) {
            if (strcmp(argv[i], "--frames") == 0 && i + 1 < argc) {
                frames = strtoul(argv[++i], NULL, 10);
            } else if (strcmp(argv[i], "--mooneye") == 0) {
                suite = SUITE_MOONEYE;
            } else if (strcmp(argv[i], "--blargg") == 0) {
                suite = SUITE_BLARGG;
            } else {
                fprintf(stderr, "unknown option: %s\n", argv[i]);
                return 2;
            }
        }
        return run_rom(argv[1], frames, suite);
    }

    fprintf(stderr,
        "usage: rom_harness --fuzz N [--seed S]\n"
        "       rom_harness ROM.gb [--frames N] [--blargg|--mooneye]\n");
    return 2;
}
