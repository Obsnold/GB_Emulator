#include "gb_load.h"
#include "gb_mem_map.h"
#include "gb_display.h"
#include "gb_opcode_table.h"
#include <stdio.h>


int main(int argc, char *argv[] )
{
   if( argc == 2 ) {
      printf("starting %s with %s\n",  argv[0], argv[1]);
   }
   else {
      printf("Incorrect arguments\n");
   }

   if(!load_gb_cart(argv[1])){
       printf("Cannot load cart\n");
   }

    load_initial_membanks();
    parse_header();
    print_opcodes();

}
