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


int main(int argc, char *argv[] )
{
   if( argc == 2 ) {
      DEBUG_PRINT("starting %s with %s\n",  argv[0], argv[1]);
   }
   else {
      DEBUG_PRINT("Incorrect arguments\n");
      return 0;
   }

   
   cpu_init();
   init_screen(); 

   if(!load_gb_cart(argv[1])){
       DEBUG_PRINT("Cannot load cart\n");
   }

   load_initial_membanks();
   print_cart_header();
    //print_opcodes();
   init_mem_map();
   //0.00000025s
   while(1){
      
      int keys = get_key_press();
      if(keys == -1){
         break;
      }
      if(boot_rom && CPU_REG.PC >= 0x100){
         load_initial_membanks();
         boot_rom = false;
      }
      gb_cpu();
      ppu();
      gb_timer();
      gb_input(keys);
      
      update_screen(gb_mem_map[LCD_LY]);
   }
   free_screen();
}
