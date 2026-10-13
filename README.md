# GB_Emulator

This is a simple GameBoy emulator running in C.
currently it can run Tetris and Dr Mario.

## Controls
  * Movement -Arrow keys
  * Select - Space
  * Start - Enter
  * A,B - A,S
  * Exit - Esc

## Build Instructions
Requires Linux and SDL2.
  * browse to directory
  * run make

## Browser version
`make web` builds a WebAssembly version that runs in a browser and loads a ROM
from your machine without uploading it. See `Web/README.md`.

## Run Emulator
  ./Out/gb_emu PATH_TO_ROM

## TO DO
Rewrite it completely
