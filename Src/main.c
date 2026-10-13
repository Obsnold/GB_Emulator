#include "gb_cart.h"
#include "gb_mem_map.h"
#include "gb_ppu.h"
#include "gb_cpu.h"
#include "gb_opcodes.h"
#include "gb_joypad_input.h"
#include "gb_serial.h"
#include "gb_timer.h"
#include "system_interface.h"
#include "debug_print.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#ifdef DEBUG
#define DEBUG_PRINT(fmt, args...)    PRINT(fmt, ## args)
#else
#define DEBUG_PRINT(fmt, args...)
#endif

bool debug_mode =false;
bool debug_step = false;
unsigned long long step_count = 0;
bool boot_rom = true;
int breakpoint=-1;
bool run = true;

// One frame of emulated time, and the slice it is advanced in.
//
// A slice of one machine cycle is what the PPU needs: it advances at most one
// mode transition per call, so handing it a whole frame at once would leave
// most of the frame sitting in its accumulator. 17556 slices per frame gives
// it far more calls than the ~616 mode transitions it has to make.
#define FRAME_NS 16742706UL
#define SLICE_NS CYCLE_TIME

// Bytes already echoed to stdout, so each one is printed once.
unsigned int serial_printed = 0;

// Anything a ROM writes to the link port goes to stdout. Test ROMs report
// their results this way, so running one directly shows its verdict.
void print_new_serial(){
   const char *text = gb_serial_text();
   unsigned int len = gb_serial_length();

   while(serial_printed < len){
      putchar(text[serial_printed]);
      serial_printed++;
   }
   if(serial_printed > 0){
      fflush(stdout);
   }
}

void gb_init(){
   cpu_init();
   init_screen(); 
   //debug_screen_init();
   init_mem_map();
   gb_serial_init();
   serial_printed = 0;
}

// Advance one frame of emulated time and upload each scanline as it is
// finished. Driven by a fixed budget rather than by measured wall clock: the
// emulated speed is then a property of the emulator rather than of how fast
// the host happens to get round the loop.
void run_frame(int keys){
   unsigned long elapsed = 0;
   uint8_t prev_line;

   gb_input(keys);
   prev_line = get_mem_map_8(LCD_LY);

   while(elapsed < FRAME_NS){
      gb_timer(SLICE_NS);
      gb_cpu(SLICE_NS);
      ppu(SLICE_NS);
      elapsed += SLICE_NS;

      // Only when the PPU has moved on a line; update_screen is a no-op
      // otherwise, and this keeps it to 154 calls a frame instead of 17556.
      uint8_t line = get_mem_map_8(LCD_LY);
      if(line != prev_line){
         enable_lcd(get_mem_map_bit(LCD_CTRL,LCD_CTRL_ENABLE) != 0);
         update_screen(line);
         prev_line = line;
      }
   }

   // Once more unconditionally: with the LCD off the PPU stops moving LY, so
   // the branch above never runs and the screen would keep showing the last
   // frame instead of blanking.
   enable_lcd(get_mem_map_bit(LCD_CTRL,LCD_CTRL_ENABLE) != 0);

   print_new_serial();
}


int main(int argc, char *argv[])
{
   if( argc < 2 ) {
      PRINT("Incorrect arguments\n");
      return 0;
   }
   PRINT("%s loading %s\n",  argv[0], argv[1]);

   if(gb_cart_load(argv[1]) < 0){
       PRINT("Cannot load cart\n");
   }

   gb_init();

   // Wall clock the next frame is due at. Pacing against a running schedule
   // rather than sleeping a fixed amount per frame stops small overruns
   // accumulating into drift.
   unsigned long next_frame = get_ns() + FRAME_NS;

   while(run){
      
      int keys = get_key_press();

      //if escape pressed then exit
      if(keys == KEY_EXIT){
         run = false;
      }

      // if debug key pressed or breakpoint reached then enable
      // debug mode
      if(keys == KEY_DEBUG || CPU_REG.PC == breakpoint){
         debug_mode = true;
         PRINT("-----------------------------------\n");
         PRINT("ENABLE DEBUG MODE\n");
         PRINT("-----------------------------------\n");
      }

      // debug mode
      if(debug_mode){
         char c = getchar();
         switch (c){
            case 'r':
               debug_mode = false;
            break;
            case 'q':
               run = false;
            break;
            default:
               print_step();
               print_opcode();
            break;
         }
      }

      run_frame(keys);

      unsigned long now = get_ns();
      if(next_frame > now){
         sleep_ns(next_frame - now);
         next_frame += FRAME_NS;
      } else {
         // Behind schedule. Resync instead of trying to make up the shortfall,
         // so a slow patch does not turn into a burst of fast frames.
         next_frame = now + FRAME_NS;
      }
   }
   free_screen();
   //debug_screen_free();
}
