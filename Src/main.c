#include "gb_load.h"
#include "gb_mem_map.h"
#include "gb_ppu.h"
#include "gb_opcode_table.h"
#include "system_interface.h"
#include <stdio.h>


int main(int argc, char *argv[] )
{
   int run = 1;
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
   init_vram_test();
   init_screen();
   while(run){
      ppu();
      update_screen(gb_mem_map[LCD_LY]);
      switch(get_key_press()){
         case -1:
            run = 0;
         break;
         
         case KEY_UP:
               gb_mem_map[LCD_SCY] -= 1;
               //printf("LCD_SCY = %d\n",gb_mem_map[LCD_SCY]);
         break;
         case KEY_DOWN:
               gb_mem_map[LCD_SCY] += 1;
               //printf("LCD_SCY = %d\n",gb_mem_map[LCD_SCY]);
         break;
         case KEY_LEFT:
               gb_mem_map[LCD_SCX] -= 1;
              // printf("LCD_SCX = %d\n",gb_mem_map[LCD_SCX]);
         break;
         case KEY_RIGHT:
               gb_mem_map[LCD_SCX] += 1;
               //printf("LCD_SCX = %d\n",gb_mem_map[LCD_SCX]);
         break;

         default:
         break;
      }
   }
   free_screen();
}
