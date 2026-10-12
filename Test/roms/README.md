# Test ROMs

Third-party test ROMs are **not committed** here. They are fetched as a pinned
release of [c-sp/gameboy-test-roms][bundle], which ships prebuilt `.gb` files,
so nothing needs an assembler:

    make fetch-test-roms      # downloads and extracts into Test/roms/bundle/
    make test-roms            # runs them against expected.txt

`make test-roms` skips cleanly if the bundle is absent, so `make check` works
without it.

[bundle]: https://github.com/c-sp/gameboy-test-roms

## expected.txt is a baseline, not a target

`expected.txt` records what passes today, one line per ROM:

    pass    blargg   blargg/cpu_instrs/individual/01-special.gb
    fail    blargg   blargg/instr_timing/instr_timing.gb
    timeout mooneye  mooneye-test-suite/acceptance/oam_dma_timing.gb

The run fails only on a **regression** - a ROM that used to pass and no longer
does - or on a ROM named in the baseline that is missing from the bundle. A ROM
that starts passing is reported as an improvement without failing the run:

    make test-roms-update     # re-record the baseline

That is deliberate. Most of the mooneye suite tests cycle-exact behaviour this
emulator does not model, so gating on a full pass would mean a permanently red
build that nobody reads.

## Where things stand

19 of 90 ROMs pass. The curated set leaves out blargg's sound tests and the DMG
OAM bug tests entirely: there is no APU and the OAM bug is not emulated, so
they would be permanent noise rather than signal.

| suite | result |
| --- | --- |
| blargg `cpu_instrs` | **11 / 11 pass** |
| blargg `instr_timing`, `mem_timing` | fail - no sub-instruction timing |
| blargg `halt_bug`, `interrupt_time` | timeout |
| mooneye `acceptance` (DMG-relevant) | 8 / 70 pass |

`cpu_instrs` passing in full is the useful part: it is independent confirmation
that the instruction set and flag handling are correct.

The mooneye failures are mostly timing-exactness. The dispatcher retires a whole
instruction at a time, so nothing observes memory or the PPU partway through
one. Closing those means a cycle-stepped CPU, which is a rewrite of the
dispatcher rather than a fix.

## How a result is read

The two suites report differently, and the harness has to be told which to
expect - `LD B,B` and `LD D,D` are ordinary instructions, and blargg's
`06-ld r,r` executes both while testing register loads:

    Build/rom_harness ROM.gb --blargg     # verdict as text on the link port
    Build/rom_harness ROM.gb --mooneye    # LD B,B breakpoint + Fibonacci registers

blargg needs the link port, which `Src/io_ports/gb_serial.c` provides. The
emulator itself also prints that output, so running a test ROM directly shows
its verdict.
