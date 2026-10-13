# Browser build

    make web                                      # emcc on PATH
    make web EMCC=/path/to/emsdk/.../emcc         # otherwise

Output lands in `Build/web/` as `index.html`, `index.js` and `index.wasm`. It is
static: serve the directory over HTTP and open it. `file://` will not work,
because the browser refuses to fetch the `.wasm` from a file URL.

    cd Build/web && python3 -m http.server 8000

CI builds this on every push to `main` and publishes it to GitHub Pages. That
needs the repository's Pages source set to **GitHub Actions** in Settings →
Pages; until it is, the job builds fine and fails at the deploy step.

## What differs from the native build

The same sources, SDL included — Emscripten's SDL2 port covers everything
`Src/os/linux_interface.c` uses. Two things change, both in `Src/main.c` behind
`#ifdef __EMSCRIPTEN__`:

**The browser owns the loop.** `run_frame()` is registered as a
`requestAnimationFrame` callback via `emscripten_set_main_loop()` instead of
being paced with `sleep_ns()`. That is why the dispatcher had to be able to
retire a whole frame of instructions in one call: a browser callback runs ~60
times a second, so one instruction per call would have run the CPU at 60 Hz.

**The cart arrives as a buffer.** There is no filesystem to read a path from, so
the page reads the picked file into the wasm heap and calls
`gb_cart_load_buffer()` through the exported `web_load_rom`. JS owns that
buffer and frees the previous one when a new ROM is loaded, because the
emulator maps the cart straight out of it.

## The ROM never leaves the browser

The file input is read with `FileReader` and copied into wasm memory. There is
no server side and nothing is uploaded, which is also why no ROMs are
distributed here — the page is only the emulator.

## Notes

- Click the canvas before playing: SDL reads the keyboard from whatever has
  focus.
- `-sMINIFY_HTML=0` is set because the HTML minifier is an optional npm
  dependency that some Emscripten installs lack, and minifying the shell buys
  nothing.
- `-sALLOW_MEMORY_GROWTH=1` so a large cart fits without sizing the heap for
  the worst case up front.
