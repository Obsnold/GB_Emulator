# GB_Emulator

This is a simple GameBoy emulator running in C.
currently it can run Tetris and Dr Mario.

## Controls
  * Movement -Arrow keys
  * Select - Space
  * Start - Enter
  * A,B - A,S
  * Exit - Esc, or the window close button

## Build and Run
Requires Linux, SDL2 and gcc.

    sudo apt install build-essential libsdl2-dev     # Debian/Ubuntu
    make
    ./Out/gb_emu PATH_TO_ROM

On Fedora the packages are `gcc make SDL2-devel`; on macOS with Homebrew,
`brew install sdl2` and build with `make CC=clang`.

## Tests and checks
`make check` runs the lot and reports every stage even if an earlier one fails.
The individual stages, cheapest first:

    make test          118 unit tests, ~1s, nothing beyond libc needed
    make analyze       gcc warnings + -fanalyzer (no binary produced)
    make cppcheck      needs `cppcheck`; skips with a message if absent
    make test-asan     the unit tests under AddressSanitizer and UBSan
    make fuzz          300 random cart images under the sanitizers
    make test-roms     third-party test ROMs against a recorded baseline

Useful while working:

    make test TEST_FILTER=timer     only tests whose name contains "timer"
    Build/test_core --no-fork       run in-process, for gdb
    make fuzz FUZZ_COUNT=3000 FUZZ_SEED=42

### Test ROMs
Not committed; fetched as a pinned release of prebuilt ROMs. Needs `curl` and
`python3`:

    make fetch-test-roms
    make test-roms

`make test-roms` fails only on a regression against `Test/roms/expected.txt`,
not on the suites being incompletely passed. See `Test/roms/README.md`.

## Browser version
`make web` builds a WebAssembly version that loads a ROM from your machine
without uploading it. Needs the Emscripten SDK:

    git clone --depth 1 https://github.com/emscripten-core/emsdk.git
    ./emsdk/emsdk install 3.1.69 && ./emsdk/emsdk activate 3.1.69
    . ./emsdk/emsdk_env.sh
    make web

    cd Build/web && python3 -m http.server 8000      # then open localhost:8000

Serve it over HTTP: browsers refuse to fetch the `.wasm` from a `file://` URL.
If `emcc` is not on your PATH, pass it explicitly with
`make web EMCC=/path/to/emcc`. See `Web/README.md`.

## Layout
    Src/, Inc/     the emulator; Src/os/ is the only SDL-dependent part
    Test/          unit tests, the ROM harness and the fuzzer
    Web/           the browser shell
    Out/, Build/   build output, not tracked

## TO DO
Rewrite it completely
