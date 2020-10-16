#include "gb_load.h"
#include "gb_mem_map.h"
#include "gb_ppu.h"
#include "gb_opcode_table.h"
#include "linux_display.h"
#include <stdio.h>


int main(int argc, char *argv[] )
{
   unsigned long gb_tick = 0;
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
   init_screen();
   for(int i=0; i < 5000000; i++){
      ppu();
      if(gb_mem_map[LCD_LY] == 143){
         
         update_screen(gb_mem_map[LCD_LY]);
         break;
      }
   }  
   free_screen();
 

}
