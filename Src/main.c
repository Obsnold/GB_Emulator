#include "gb_cart.h"
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
#include <time.h>
#include <pthread.h>

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
   pthread_t timer_thread;
   pthread_t ppu_thread;
   pthread_t cpu_thread;
   pthread_t input_thread;

   if( argc < 2 ) {
      PRINT("Incorrect arguments\n");
      return 0;
   }
   PRINT("%s loading %s\n",  argv[0], argv[1]);
   
   
   cpu_init();
   init_screen(); 
   //debug_screen_init();

   init_mem_map();

   if(gb_cart_load(argv[1]) < 0){
       PRINT("Cannot load cart\n");
   }

   int createerror = pthread_create(&timer_thread, NULL, gb_timer, NULL);
   if (0 != createerror) /*check whether the thread creation was successful*/
   {
      PRINT("Error creating pthread");
      pthread_join(timer_thread, NULL); /*wait until the created thread terminates*/
      return 0;
   }
   print_cart_header();
   //print_memory(CART_RAM,GB_RAM_1);
   long temp;
   while(run){
      
      int keys = get_key_press();

      //if escape pressed then exit
      if(keys == KEY_EXIT){
        // print_lcd();
        // print_cpu_reg();
        // print_opcode();
         //print_memory(CART_RAM,GB_RAM_1);
        /* print_memory(OAM_TABLE,NA_MEM);
         print_memory(BG_MAP_1,BG_MAP_2);
         print_memory(BG_MAP_2,CART_RAM);
         print_memory(OAM_TABLE,NA_MEM);*/
         //print_interrupts();
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

      // main loop
      gb_cpu();
      ppu();
      //gb_timer();
      gb_input(keys);
     
      if(get_mem_map_bit(LCD_CTRL,LCD_CTRL_ENABLE)){
         enable_lcd(true);
      } else {
         enable_lcd(false);
      }
      update_screen(get_mem_map_8(LCD_LY));

      //PRINT("MAIN1---\n");
      /*if(temp %100000 == 0){
         debug_screen();
         temp=0;
         //PRINT("DEBUG_PRINT\n");
      }
      temp++;*/
   }
   pthread_cancel(timer_thread);
   free_screen();
   //debug_screen_free();
}
