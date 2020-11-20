#include "gb_load.h"
#include "gb_mem_map.h"
#include "gb_ppu.h"
#include "gb_cpu.h"
#include "gb_opcodes.h"
#include "gb_input.h"
#include "gb_boot_rom.h"
#include "system_interface.h"
#include <stdio.h>

int main(int argc, char *argv[] )
{
   /*if( argc == 2 ) {
      printf("starting %s with %s\n",  argv[0], argv[1]);
   }
   else {
      printf("Incorrect arguments\n");
   }

   if(!load_gb_cart(argv[1])){
       printf("Cannot load cart\n");
   }*/

    /*load_initial_membanks();
    parse_header();
    print_opcodes();*/
   init_mem_map();
   // load boot rom
   /*for(int i = 0; i < 0x100; i++){
      gb_mem_map[i] = gb_boot_rom[i];
   }*/
   // init_vram_test();
   init_screen();
   while(1){
      gb_cpu();
      ppu();
      update_screen(gb_mem_map[LCD_LY]);
      int keys = get_key_press();
      if(keys == -1){
         break;
      }
      gb_input(keys);
   }
   free_screen();
}
