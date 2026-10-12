# Test ROMs

This directory is for third-party Game Boy test ROMs. They are **not committed**
here — fetch them yourself and drop the `.gb` files in this directory (or in
subdirectories; the runner searches recursively).

    make test-roms

## mooneye-test-suite — works today

The harness understands the mooneye result convention, which needs no serial
port: a test ends on a `LD B,B` breakpoint and passes when the register file
holds the Fibonacci values `B=3 C=5 D=8 E=13 H=21 L=34`. A `LD D,D` means the
test failed.

    https://github.com/Gekkio/mooneye-test-suite

Start with `acceptance/timer/` — those pin down exactly the DIV and TIMA
behaviour the unit tests in `Test/test_timer.c` cover, and `acceptance/bits/`
and `acceptance/instr/` for the instruction set.

## blargg's cpu_instrs / instr_timing — needs a serial port first

These are the standard instruction-correctness ROMs, but they report results by
writing text to the link port at `0xFF01`/`0xFF02`, which this emulator does
not implement yet. Implementing serial output is small: on a write to `0xFF02`
with bit 7 set, append `0xFF01` to a buffer and raise the serial interrupt.
Once that exists, `rom_harness` can match on the strings "Passed" and "Failed"
and this becomes the most valuable suite in the set.

    https://github.com/retrio/gb-test-roms

The same ROMs also write a result signature to `0xA000..0xA003` when cart RAM
is present, which is an alternative to serial if you would rather not add it.
