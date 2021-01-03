#include "gb_load.h"
#include "gb_mem_map.h"
#include "gb_ppu.h"
#include "gb_cpu.h"
#include "gb_opcodes.h"
#include "gb_joypad_input.h"
#include "gb_timer.h"
#include "system_interface.h"
#include "debug_print.h"
#include <stdio.h>
#include <unistd.h>

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

int main(int argc, char *argv[] )
{
   if( argc < 2 ) {
      PRINT("Incorrect arguments\n");
      return 0;
      
   }
   PRINT("%s loading %s\n",  argv[0], argv[1]);
   
   
   cpu_init();
   init_screen(); 

   if(!load_gb_cart(argv[1])){
       PRINT("Cannot load cart\n");
   }

   print_cart_header();

   init_mem_map();
   //0.00000025s
   while(run){
      
      int keys = get_key_press();

      //if escape pressed then exit
      if(keys == -1){
         print_lcd();
         print_cpu_reg();
         print_opcode();
         print_memory(TILE_RAM_0,BG_MAP_1);
         print_memory(BG_MAP_1,BG_MAP_2);
         print_memory(BG_MAP_2,CART_RAM);
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

      // TODO change this later so it is handled in the 
      // mem map or cartidge layer 
      if(boot_rom && CPU_REG.PC >= 0x100){
         load_initial_membanks();
         boot_rom = false;
      }

      // main loop
      gb_cpu();
      ppu();
      gb_timer();
      gb_input(keys);
      
      update_screen(gb_mem_map[LCD_LY]);
   }
   free_screen();
}
