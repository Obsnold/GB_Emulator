#include "gb_ppu.h"
#include "gb_mem_map.h"
#include "gb_test_image.h"
#include <time.h>
#include <math.h>
#include <stdbool.h>

//background tile data 256 tiles
//sprtie tiles 

//screen 20x18 wxh viewport

//draw order
//background map 32x32

//window overlay

//sprites obj
//40 sprites
//10 per line
//priority
//per line
//oam.x!=0
//line + 16 >= oam.y
//line + 16 < oma.y +h


//modes
// oam search 80clocks/20cycles
// pixel_transfer 172-289clocks/43-72 cycles
// h_blank 87-204clocks/22-51 cycles
// v_blank 4560 clocks/1140 cycles

/*The LCD controller operates on a 2^22 Hz = 4.194 MHz dot clock. 
An entire frame is 154 scanlines, 70224 dots, or 16.74 ms. 
On scanlines 0 through 143, the LCD controller cycles through modes 2, 3, 
and 0 once every 456 dots. Scanlines 144 through 153 are mode 1.*/


#define BG_SIZE 32
#define TILE_SIZE 8
#define TILE_MEM_SIZE 0x10

//uint8_t gb_bg_pixels[BG_SIZE*TILE_SIZE][BG_SIZE*TILE_SIZE];
uint32_t gb_display[GB_SCREEN_WIDTH][GB_SCREEN_HEIGHT];

#define OAM_SEARCH_CYCLES 20
#define PIXEL_TRANSFER_CYCLES 43
#define H_BLANK_CYCLES 51
#define V_BLANK_CYCLES 1140

unsigned long prev_tick = 0;
unsigned long ppu_cycles = 0;
unsigned long ppu_cycles_count = OAM_SEARCH_CYCLES;

uint32_t bg_pallet[4] = {green_1, green_2, green_3, green_4};
uint32_t sp_pallet_1[4] = {trans, green_1, green_2, green_3};
uint32_t sp_pallet_2[4] = {trans, green_1, green_2, green_3};

unsigned long get_ticks(){
    struct timespec _t;
    clock_gettime(CLOCK_REALTIME, &_t);
    return ((_t.tv_sec*1000000000 + lround(_t.tv_nsec)) / 1000);
}


void ppu_oam_search(){
    //search for all visible sprites
    for(int i = 0; i < OAM_TABLE_SIZE; i++){
        //check y
        gb_mem_map[OAM_TABLE + (i*OAM_SIZE)];
        //check x
    }
}

void ppu_pixel_transfer(){
    uint8_t screen_x = gb_mem_map[LCD_SCX];
    uint8_t screen_y = gb_mem_map[LCD_SCY];
    uint8_t line = gb_mem_map[LCD_LY];
    int bg_window_tile_mode = GET_MEM_MAP(LCD_CTRL, LCD_CTRL_BG_W_TILE_SELECT);
    bool window_enabled = false;
    uint16_t bg_map = BG_MAP_1;
    uint16_t window_map = BG_MAP_1;
    uint16_t bg_window_tile_set = VRAM_BLOCK_0;
    uint16_t input_buffer_1 = 0;
    uint16_t input_buffer_2 = 0;
    int buffer_shift_count = 0;
    int tile_count = 1;

    //do we need to display the window on this line?
    if(GET_MEM_MAP(LCD_CTRL,LCD_CTRL_WINDOW_ENABLE) && (line >= gb_mem_map[LCD_WY])){
            window_enabled = true;
    }
    
    //get background map
    if(GET_MEM_MAP(LCD_CTRL, LCD_CTRL_BG_MAP_SELECT)){
        bg_map = BG_MAP_2;
    } else {
        bg_map = BG_MAP_1;
    }

    //get window map
    if(GET_MEM_MAP(LCD_CTRL, LCD_CTRL_WINDOW_MAP_SELECT)){
        window_map = BG_MAP_2;
    } else {
        window_map = BG_MAP_1;
    }

    //get tile set
    if(bg_window_tile_mode == 0){
        bg_window_tile_set = VRAM_BLOCK_0;
    } else {
        bg_window_tile_set = VRAM_BLOCK_2;
    }
    
    // work out position of start tile in map
    int map_tile_x = screen_x/TILE_SIZE;
    int map_tile_y = (screen_y+line)/TILE_SIZE;

    //get start pixels from tile
    int tile_pixel_x = screen_x - (map_tile_x*TILE_SIZE);
    int tile_pixel_y = (screen_y+line) - (map_tile_y*TILE_SIZE);
    
    //deal with going off the edge of the screen
    if(map_tile_y >= BG_SIZE){
        map_tile_y-=BG_SIZE;
    }
    
    // get offsete for working out tile poition
    // these wont change as it the line is the same
    int map_line = map_tile_y*BG_SIZE;
    int tile_line_offset = tile_pixel_y*2;

    // get first map tile
    int map_tile_start = map_tile_x + map_line;

    // get first tile from tile set
    uint16_t tile_line = bg_window_tile_set + (gb_mem_map[bg_map + map_tile_start] * TILE_MEM_SIZE) + tile_line_offset;
    input_buffer_1 = gb_mem_map[tile_line] << 8;
    input_buffer_2 = gb_mem_map[tile_line + 1] << 8;

    //deal with going off the edge of the screen
    if((map_tile_x + tile_count) >= BG_SIZE){
        tile_count-=BG_SIZE;
    }

    // get second from tile set
    tile_line = bg_window_tile_set + (gb_mem_map[bg_map + map_tile_start + tile_count] * TILE_MEM_SIZE) + tile_line_offset;
    input_buffer_1 |= gb_mem_map[tile_line];
    input_buffer_2 |= gb_mem_map[tile_line + 1];

    //shift buffers to starting pixel and account for offset
    buffer_shift_count = tile_pixel_x;
    input_buffer_1 = input_buffer_1 << buffer_shift_count;
    input_buffer_2 = input_buffer_2 << buffer_shift_count;
   // printf("buffer_shift_count= %04x \n",buffer_shift_count);

    for(int x = 0; x < GB_SCREEN_WIDTH; x++){
        //check if we need to load the next tile
        if(buffer_shift_count == 8){
            buffer_shift_count = 0;
            tile_count++;
            //deal with going off the edge of the screen
            if((map_tile_x + tile_count) >= BG_SIZE){
                tile_count-=BG_SIZE;
            }

            // get address of tile in tile map
            int tile_map_addr = gb_mem_map[bg_map + map_tile_start + tile_count];

            //need to check correct addresing
            if(bg_window_tile_mode == 1 && tile_map_addr >= 128){
                tile_map_addr -= 256;
                bg_window_tile_set = VRAM_BLOCK_1;
            }

            //load next tile
            tile_line = bg_window_tile_set + (tile_map_addr * TILE_MEM_SIZE) + tile_line_offset;
            input_buffer_1 |= gb_mem_map[tile_line];
            input_buffer_2 |= gb_mem_map[tile_line + 1];
        }


        //check to see if we need to apply the window
        if(window_enabled && (x >= gb_mem_map[LCD_WX])){
            //if window is enabled overwrite whatever is currently in the buffer
            input_buffer_1 = 0;
            input_buffer_2 = 0;
            buffer_shift_count = 0;
            tile_count = 1;

            //reset tilepointers to window map
            //window always starts at 0,0 overlayed starting at wx wy 

            // get first tile from tile set
            uint16_t tile_line = bg_window_tile_set + (gb_mem_map[window_map] * TILE_MEM_SIZE) + tile_line_offset;
            input_buffer_1 = gb_mem_map[tile_line] << 8;
            input_buffer_2 = gb_mem_map[tile_line + 1] << 8;

            // get second from tile set
            tile_line = bg_window_tile_set + (gb_mem_map[window_map + tile_count] * TILE_MEM_SIZE) + tile_line_offset;
            input_buffer_1 |= gb_mem_map[tile_line];
            input_buffer_2 |= gb_mem_map[tile_line + 1];
        }

        //check OAM to see if we need to apply a sprite
        if(GET_MEM_MAP(LCD_CTRL,LCD_CTRL_OBJ_ENABLE)){

        }

        //get latest pixel on to the screen buffer
        uint32_t colour = bg_pallet[0];
        if(input_buffer_1 & 0x8000){
            if(input_buffer_2 & 0x8000){
                colour = bg_pallet[0];
            } else {
                colour = bg_pallet[1];
            }
        } else {
            if(input_buffer_2 & 0x8000){
                colour = bg_pallet[2];
            } else {
                colour = bg_pallet[3];
            }
        }
        
        //draw pixel to display
        gb_display[x][line] = colour;

        //shift pixels and increment counter
        input_buffer_1 = input_buffer_1 << 0x01;
        input_buffer_2 = input_buffer_2 << 0x01;
        buffer_shift_count++;
    }
}


void ppu_h_blank(){
    //do nothing just stuff
}

void ppu_v_blank(){
    //do nothing just stuff
}

uint8_t ppu(){
    uint8_t ppu_mode = GET_MEM_MAP(LCD_STAT,LCD_STAT_MODE);
    if(ppu_cycles == 0){
        switch(ppu_mode){
            case LCD_STAT_MODE_OAM:
                ppu_oam_search();
            break;
            case LCD_STAT_MODE_PIXEL_TRANS:
                ppu_pixel_transfer();
            break;
            case LCD_STAT_MODE_HBLNK:
                ppu_h_blank();
            break;
            case LCD_STAT_MODE_VBLNK:
                ppu_v_blank();
            break;
            default:
            //error
            break;
        }
    }

    unsigned long tick = get_ticks();
    ppu_cycles += tick - prev_tick;
    prev_tick = tick;
   /* printf("tick = %lu \n", tick);
    printf("ppu_cycles = %lu \n", ppu_cycles);
    printf("LCD_LY = %u \n", gb_mem_map[LCD_LY]);
    printf("ppu_mode = %u \n", ppu_mode);*/

    if (ppu_cycles > ppu_cycles_count){
        ppu_cycles = 0;
        switch (ppu_mode){
            case LCD_STAT_MODE_OAM:
                //printf("LCD_STAT_MODE_OAM \n");
                CLR_MEM_MAP(LCD_STAT,LCD_STAT_MODE);
                SET_MEM_MAP(LCD_STAT,LCD_STAT_MODE_PIXEL_TRANS);
                ppu_cycles_count = PIXEL_TRANSFER_CYCLES;
            break;
            case LCD_STAT_MODE_PIXEL_TRANS:
                //printf("LCD_STAT_MODE_PIXEL_TRANS \n");
                CLR_MEM_MAP(LCD_STAT,LCD_STAT_MODE);
                SET_MEM_MAP(LCD_STAT,LCD_STAT_MODE_HBLNK);
                ppu_cycles_count = H_BLANK_CYCLES;
            break;
            case LCD_STAT_MODE_HBLNK:
                //printf("LCD_STAT_MODE_HBLNK \n");
                //printf("LCD_LY = %u \n", gb_mem_map[LCD_LY]);
                gb_mem_map[LCD_LY]++;
                if(gb_mem_map[LCD_LY] >= GB_SCREEN_HEIGHT){
                    gb_mem_map[LCD_LY] = 0x00;
                    CLR_MEM_MAP(LCD_STAT,LCD_STAT_MODE);
                    SET_MEM_MAP(LCD_STAT,LCD_STAT_MODE_VBLNK);
                    ppu_cycles_count = V_BLANK_CYCLES;
                } else {
                    CLR_MEM_MAP(LCD_STAT,LCD_STAT_MODE);
                    SET_MEM_MAP(LCD_STAT,LCD_STAT_MODE_OAM);
                    ppu_cycles_count = OAM_SEARCH_CYCLES;
                }
            break;
            case LCD_STAT_MODE_VBLNK:
                //printf("LCD_STAT_MODE_VBLNK \n");
                CLR_MEM_MAP(LCD_STAT,LCD_STAT_MODE);
                SET_MEM_MAP(LCD_STAT,LCD_STAT_MODE_OAM);
                ppu_cycles_count = OAM_SEARCH_CYCLES;
            break;
            default:
            //error
            break;
        }
    }
    return ppu_mode;
}

