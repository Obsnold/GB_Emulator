# Tests

    make check       every stage below, reporting all of them

    make analyze     GCC warnings + -fanalyzer (two passes, see the Makefile)
    make cppcheck    cppcheck, skipped cleanly if not installed
    make test        97 unit tests, no dependencies, ~1s
    make test-asan   same tests under AddressSanitizer + UBSan
    make fuzz        random cart images under the sanitizers
    make test-roms   third-party test ROMs vs a baseline (see Test/roms/README.md)

    make test TEST_FILTER=timer     run only tests matching a substring
    Build/test_core --no-fork       run in-process, for gdb

`make check` does not stop at the first failing stage, so a red tree shows you
everything at once. CI runs the same stages as separate steps.

A test is reported as `SANITIZER` when its output contains a sanitizer report,
whatever its exit status. UBSan is built recoverable so one bad index does not
abort the run and hide every test after it, and this is what stops that from
quietly turning into a pass.

The emulator core links without SDL and without `main.c`, so the test binaries
are built from everything in `Src/` except `main.c` and `Src/os/linux_interface.c`.
Every piece of emulator state is a non-static global, so `emu_reset()` in
`test_support.c` can reset the machine between cases without production code
needing a test seam. Test objects live under `Build/` and never collide with the
in-tree `.o` files the default build produces.

Each test runs in a forked child with its stderr captured. That is deliberate:
the bugs this suite is aimed at include memory-safety bugs, and without
isolation one bad read takes the whole run down and hides every test after it.

## What is here, and why

| File | Covers |
| --- | --- |
| `test_opcode_table.c` | All 256 opcodes' length and cycle cost against `reference_opcodes.h`, and the CB sub-table |
| `test_cpu.c` | ALU flag semantics, stack, control flow, interrupt dispatch, HALT/STOP |
| `test_timer.c` | DIV and TIMA rates at each TAC setting, overflow, reload, interrupt |
| `test_timing.c` | That the CPU and PPU agree on how long a cycle is, and the per-frame cycle budget |
| `test_mem_map.c` | Address decoding, echo RAM, VRAM/OAM access gating, the 0xFFFF boundary |
| `test_cart.c` | Header validation, ROM/RAM bank selection and clamping, per-MBC behaviour |
| `test_ppu.c` | Window and sprite clipping geometry, OAM search limits, palettes, frame sequencing |
| `test_joypad.c` | Register half-selection and the press/release interrupt edge |
| `test_serial.c` | Link port transfer, start-bit clear, interrupt, no-cable readback |

`reference_opcodes.h` is generated, not hand-written: it encodes the canonical
DMG instruction lengths and M-cycle counts so that a transcription error in
`Src/opcodes/gb_opcodes_table.c` shows up as a diff against a source of truth
rather than as a game running at the wrong speed.

## Two measurements worth knowing about

`test_timing.c` recovers the CPU's nanoseconds-per-cycle constant by binary
searching the threshold at which the dispatcher retires an instruction, rather
than reading the constant directly — `CPU_CYCLE_TIME` is private to
`gb_cpu.c`, and a test that read it would only be checking the constant against
itself. It then drives a whole frame and accounts for retired cycles against
the 17556 M-cycles a real frame contains.

`test_timer.c` feeds the same elapsed time in different chunk sizes. An
accumulator that is only advanced inside its own "has a tick elapsed yet"
branch silently drops every delta smaller than one tick, so a timer can look
fine with coarse input and be completely dead with realistic input.

## Adding a test

One `TEST()` block, nothing else — registration is automatic.

```c
TEST(my_thing_works)
{
    emu_reset();
    set_mem_map_8(0xC000, 0x5A);
    EXPECT_EQ_X(get_mem_map_8(0xC000), 0x5A);
}
```

`CHECK_*` stops the test at the first failure; `EXPECT_*` records it and
carries on, which is what you want when one test sweeps a whole table.
