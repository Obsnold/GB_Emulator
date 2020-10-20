#include "gb_load.h"
#include "gb_mem_map.h"
#include "gb_ppu.h"
#include "gb_opcode_table.h"
#include "linux_display.h"
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
   init_screen();
   while(run){
      ppu();
      //if(gb_mem_map[LCD_LY] == 143){
      update_screen(gb_mem_map[LCD_LY]);
      //}
      switch(get_key_press()){
         case 1:
         run = 0;
         break;
         case 2:
            if(gb_mem_map[LCD_SCY] == 0){
               gb_mem_map[LCD_SCY] = 0xFF;
            } else {
               gb_mem_map[LCD_SCY] -= 1;
            }
         break;
         case 3:
            if(gb_mem_map[LCD_SCY] == 0xFF){
               gb_mem_map[LCD_SCY] = 1;
            } else {
               gb_mem_map[LCD_SCY] += 1;
            }
         break;
         case 4:
            if(gb_mem_map[LCD_SCX] == 0){
               gb_mem_map[LCD_SCX] = 0xFF;
            } else {
               gb_mem_map[LCD_SCX] -= 1;
            }
         break;
         case 5:
            if(gb_mem_map[LCD_SCX] == 0xFF){
               gb_mem_map[LCD_SCX] = 1;
            } else {
               gb_mem_map[LCD_SCX] += 1;
            }
         break;

         default:
         break;
      }
   }
   free_screen();
}
