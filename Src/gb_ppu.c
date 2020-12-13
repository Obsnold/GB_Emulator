#include "gb_ppu.h"
#include "gb_mem_map.h"
#include "gb_test_image.h"
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

// OAM layout
// Byte0 - y position
// Byte1 - x position
// Byte2 - tile number
// Byte3 - flags n stuff

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
uint8_t oam_list[10];
uint8_t oam_list_size = 0;

uint32_t bg_pallet[4] = {green_1, green_2, green_3, green_4};
uint32_t sp_pallet_1[4] = {trans, green_1, green_2, green_3};
uint32_t sp_pallet_2[4] = {trans, green_1, green_2, green_3};




void ppu_oam_search(){
    uint8_t line = gb_mem_map[LCD_LY];
    oam_list_size = 0;
    //search for all visible sprites
    for(int i = 0; i < OAM_TABLE_SIZE; i++){
        //check x > 0, check line >= y,  line < y+8
        if( (gb_mem_map[OAM_TABLE + (i*OAM_SIZE) + OAM_X_POS] > 0) &&
            (line >= gb_mem_map[OAM_TABLE + (i*OAM_SIZE) + OAM_Y_POS] ) &&
            (line < (gb_mem_map[OAM_TABLE + (i*OAM_SIZE) + OAM_Y_POS] + TILE_SIZE))){
                //if sprite is on the line then add to list
                oam_list[oam_list_size] = i;
                oam_list_size++;
                //limit of 10 sprites per line
                if(oam_list_size == 10){
                    break;
                }
        }
    }
}

//revers bits used for fliping sprite tile x axis
unsigned char reverse_bits(unsigned char b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

void ppu_pixel_transfer(){
    uint8_t screen_x = gb_mem_map[LCD_SCX];
    uint8_t screen_y = gb_mem_map[LCD_SCY];
    uint8_t line = gb_mem_map[LCD_LY];
    int bg_window_tile_mode = GET_MEM_MAP(LCD_CTRL, LCD_CTRL_BG_W_TILE_SELECT);
    bool window_enabled = false;
    bool display_sprite = false;
    uint16_t map = BG_MAP_1;
    uint16_t bg_window_tile_set = VRAM_BLOCK_0;
    uint16_t input_buffer_1 = 0;
    uint16_t input_buffer_2 = 0;
    int buffer_shift_count = 0;
    int tile_count = 1;
    uint8_t sprite_line_1 = 0;
    uint8_t sprite_line_2 = 0;
    uint8_t sprite_list_num = 0;
    uint8_t sprite_buf_shift_count = 0;

    //do we need to display the window on this line?
    if(GET_MEM_MAP(LCD_CTRL,LCD_CTRL_WINDOW_ENABLE) && (line >= gb_mem_map[LCD_WY])){
            window_enabled = true;
    }
    
    //get background map
    if(GET_MEM_MAP(LCD_CTRL, LCD_CTRL_BG_MAP_SELECT)){
        map = BG_MAP_2;
    } else {
        map = BG_MAP_1;
    }

    //get tile set
    if(bg_window_tile_mode == 0){
        bg_window_tile_set = VRAM_BLOCK_2;
    } else {
        bg_window_tile_set = VRAM_BLOCK_0;
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

    // get address of tile in tile map
    int tile_map_addr = gb_mem_map[map + map_tile_start];

    //need to check correct addresing
    if(bg_window_tile_mode == 0) {
        if(tile_map_addr >= 0x80){
            tile_map_addr -= 0x100;
            bg_window_tile_set = VRAM_BLOCK_1;
        } else {
            bg_window_tile_set = VRAM_BLOCK_2;
        }
    }

    // get first tile from tile set
    uint16_t tile_line = bg_window_tile_set + (tile_map_addr * TILE_MEM_SIZE) + tile_line_offset;
    input_buffer_1 = gb_mem_map[tile_line] << 8;
    input_buffer_2 = gb_mem_map[tile_line + 1] << 8;

    //deal with going off the edge of the screen
    if((map_tile_x + tile_count) >= BG_SIZE){
        tile_count-=BG_SIZE;
    }

    // get address of tile in tile map
    tile_map_addr = gb_mem_map[map + map_tile_start + tile_count];

    //need to check correct addresing
    if(bg_window_tile_mode == 0) {
        if(tile_map_addr >= 0x80){
            tile_map_addr -= 0x100;
            bg_window_tile_set = VRAM_BLOCK_1;
        } else {
            bg_window_tile_set = VRAM_BLOCK_2;
        }
    }

    // get second from tile set
    tile_line = bg_window_tile_set + (gb_mem_map[map + map_tile_start + tile_count] * TILE_MEM_SIZE) + tile_line_offset;
    input_buffer_1 |= gb_mem_map[tile_line];
    input_buffer_2 |= gb_mem_map[tile_line + 1];

    //shift buffers to starting pixel and account for offset
    buffer_shift_count = tile_pixel_x;
    input_buffer_1 = input_buffer_1 << buffer_shift_count;
    input_buffer_2 = input_buffer_2 << buffer_shift_count;

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
            tile_map_addr = gb_mem_map[map + map_tile_start + tile_count];

            //need to check correct addresing
            if(bg_window_tile_mode == 0) {
                if(tile_map_addr >= 0x80){
                    tile_map_addr -= 0x100;
                    bg_window_tile_set = VRAM_BLOCK_1;
                } else {
                    bg_window_tile_set = VRAM_BLOCK_2;
                }
            }

            //load next tile
            tile_line = bg_window_tile_set + (tile_map_addr * TILE_MEM_SIZE) + tile_line_offset;
            input_buffer_1 |= gb_mem_map[tile_line];
            input_buffer_2 |= gb_mem_map[tile_line + 1];
        }


        //check to see if we need to apply the window
        if(window_enabled && (x >= gb_mem_map[LCD_WX] - 7)){
            //disable 
            window_enabled=false;
            //if window is enabled overwrite whatever is currently in the buffer
            input_buffer_1 = 0;
            input_buffer_2 = 0;
            buffer_shift_count = 0;
            tile_count = 1;
            
            //get window map
            if(GET_MEM_MAP(LCD_CTRL, LCD_CTRL_WINDOW_MAP_SELECT)){
                map = BG_MAP_2;
            } else {
                map = BG_MAP_1;
            }

            //reset tilepointers to window map
            //window always starts at 0,0 overlayed starting at wx wy
            int window_line=(line - gb_mem_map[LCD_WY]);
            map_tile_x = 0;
            map_tile_y = window_line/TILE_SIZE;

            //get start pixels from tile
            tile_pixel_x = 0;
            tile_pixel_y = window_line - (map_tile_y*TILE_SIZE);
            
            //deal with going off the edge of the screen
            if(map_tile_y >= BG_SIZE){
                map_tile_y-=BG_SIZE;
            }
            
            // get offsete for working out tile poition
            // these wont change as it the line is the same
            map_line = map_tile_y*BG_SIZE;
            tile_line_offset = tile_pixel_y*2;

            // get first map tile
            map_tile_start = map_tile_x + map_line;

            // get address of tile in tile map
            tile_map_addr = gb_mem_map[map + map_tile_start];

            //need to check correct addresing
            if(bg_window_tile_mode == 0) {
                if(tile_map_addr >= 0x80){
                    tile_map_addr -= 0x100;
                    bg_window_tile_set = VRAM_BLOCK_1;
                } else {
                    bg_window_tile_set = VRAM_BLOCK_2;
                }
            }

            // get first tile from tile set
            tile_line = bg_window_tile_set + (tile_map_addr * TILE_MEM_SIZE) + tile_line_offset;
            input_buffer_1 = gb_mem_map[tile_line] << 8;
            input_buffer_2 = gb_mem_map[tile_line + 1] << 8;

            // get address of tile in tile map
            tile_map_addr = gb_mem_map[map + map_tile_start + tile_count];

            //need to check correct addresing
            if(bg_window_tile_mode == 0) {
                if(tile_map_addr >= 0x80){
                    tile_map_addr -= 0x100;
                    bg_window_tile_set = VRAM_BLOCK_1;
                } else {
                    bg_window_tile_set = VRAM_BLOCK_2;
                }
            }

            // get second from tile set
            tile_line = bg_window_tile_set + (tile_map_addr * TILE_MEM_SIZE) + tile_line_offset;
            input_buffer_1 |= gb_mem_map[tile_line];
            input_buffer_2 |= gb_mem_map[tile_line + 1];
        }

        //check OAM to see if we need to apply a sprite
        // also check we are not currently displaying a sprite
        if(GET_MEM_MAP(LCD_CTRL,LCD_CTRL_OBJ_ENABLE) && (sprite_buf_shift_count == 0)){
            
            for(int i = 0; i < oam_list_size; i++){
                uint16_t oam = OAM_TABLE + (oam_list[i] * OAM_SIZE);
                uint8_t oam_x_pos = gb_mem_map[oam + OAM_X_POS];
                
                //need to check we are in the right x position
                if((x >= (oam_x_pos - TILE_SIZE)) && (x < oam_x_pos)){
                    
                    int sprite_line_offset = line - gb_mem_map[oam + OAM_Y_POS];
                    sprite_line_1 = 0;
                    sprite_line_2 = 0;
                    sprite_list_num = i;
                    sprite_buf_shift_count = 0;
                    
                    //flip y if we need to
                    if(GET_MEM_MAP((oam + OAM_FLAGS),OAM_FLAGS_Y_FLIP)){
                        sprite_line_offset = TILE_SIZE - sprite_line_offset -1;
                    }
                    
                    // get line
                    tile_line = VRAM_BLOCK_0 + (gb_mem_map[oam + OAM_TILE] * TILE_MEM_SIZE) + (sprite_line_offset*2);
                    sprite_line_1 = gb_mem_map[tile_line];
                    sprite_line_2 = gb_mem_map[tile_line+1];

                    //flip x if we need to
                    if(GET_MEM_MAP((oam + OAM_FLAGS),OAM_FLAGS_X_FLIP)){
                        sprite_line_1 = reverse_bits(sprite_line_1);
                        sprite_line_2 = reverse_bits(sprite_line_2);
                    }

                    // check if we need to shift off the left side of the screen
                    if(oam_x_pos < TILE_SIZE){
                        sprite_line_1 = sprite_line_1 << (TILE_SIZE - oam_x_pos);
                        sprite_line_2 = sprite_line_2 << (TILE_SIZE - oam_x_pos);
                        sprite_buf_shift_count = TILE_SIZE - oam_x_pos;
                        // is the sprite also overlapped?
                        if(x > 0){
                            sprite_line_1 = sprite_line_1 << x;
                            sprite_line_2 = sprite_line_2 << x;
                            sprite_buf_shift_count += x;
                        }
                    }else if(x > (oam_x_pos - TILE_SIZE)){ //check if we need to shift due to sprite overlap
                        int shift = (x + TILE_SIZE) - oam_x_pos;
                        sprite_line_1 = sprite_line_1 << shift;
                        sprite_line_2 = sprite_line_2 << shift;
                        sprite_buf_shift_count += shift;
                    }

                    display_sprite = true;

                    break;
                }
            }
        }

        //get latest pixel on to the screen buffer
        uint32_t bg_colour = bg_pallet[0];
        if(input_buffer_1 & 0x8000){
            if(input_buffer_2 & 0x8000){
                bg_colour = bg_pallet[0];
            } else {
                bg_colour = bg_pallet[1];
            }
        } else {
            if(input_buffer_2 & 0x8000){
                bg_colour = bg_pallet[2];
            } else {
                bg_colour = bg_pallet[3];
            }
        }
        
        //draw pixel to display
        gb_display[x][line] = bg_colour;

        //  do we need to overlay a sprite?
        if(GET_MEM_MAP(LCD_CTRL,LCD_CTRL_OBJ_ENABLE) && (sprite_buf_shift_count < TILE_SIZE) && display_sprite){
            uint32_t sprite_colour = 0;
            uint32_t* pallet;
            uint16_t oam = OAM_TABLE + (oam_list[sprite_list_num] * OAM_SIZE);
            
            //if the pixel is not transparent
            if(!(((sprite_line_1 & 0x80) == 0) && ((sprite_line_2 & 0x80) == 0))){
                
                //check the apllet to use
                if(GET_MEM_MAP((oam + OAM_FLAGS), OAM_FLAGS_PALETTE_NO) == 0){
                    pallet = sp_pallet_1;
                } else {
                    pallet = sp_pallet_2;
                }

                //need to check sprite priority
                int priority = GET_MEM_MAP(oam + OAM_FLAGS,OAM_FLAGS_OBJ_BG_PRIORITY);

                if((priority == 0) || (priority &&
                ((input_buffer_1 & 0x80) == 0) && ((input_buffer_2 & 0x80) == 0))){
                    if(sprite_line_1 & 0x80){
                        if(sprite_line_2 & 0x80){
                            sprite_colour = pallet[1];
                        } else {
                            sprite_colour = pallet[2];
                        }
                    } else {
                        if(sprite_line_2 & 0x80){
                            sprite_colour = pallet[3];
                        }
                    }
                    
                    //draw pixel to display
                    gb_display[x][line] = sprite_colour;
                }
            }

            //shift pixels and increment counter
            sprite_line_1 = sprite_line_1 << 0x01;
            sprite_line_2 = sprite_line_2 << 0x01;
            sprite_buf_shift_count++;

            if(sprite_buf_shift_count == TILE_SIZE){
                // we have finished displaying the sprite so reset counter
                sprite_buf_shift_count = 0;
                display_sprite = false;
            }
        } 

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

    // if we are in a new mode run the appropriate code
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

    // get time 
    unsigned long tick = get_ticks();
    ppu_cycles += tick - prev_tick;
    prev_tick = tick;

    // If the cycle time has elapsed set the appropriate flags and change the cycle count
    if (ppu_cycles > (ppu_cycles_count* 250)){
        //printf("ppu_cycles= %ld ppu_cycles_count = %ld\n",ppu_cycles, ppu_cycles_count*250);
        ppu_cycles = 0;
        switch (ppu_mode){
            case LCD_STAT_MODE_OAM:
                CLR_MEM_MAP(LCD_STAT,LCD_STAT_MODE);
                SET_MEM_MAP(LCD_STAT,LCD_STAT_MODE_PIXEL_TRANS);
                ppu_cycles_count = PIXEL_TRANSFER_CYCLES;
            break;
            case LCD_STAT_MODE_PIXEL_TRANS:
                CLR_MEM_MAP(LCD_STAT,LCD_STAT_MODE);
                SET_MEM_MAP(LCD_STAT,LCD_STAT_MODE_HBLNK);
                ppu_cycles_count = H_BLANK_CYCLES;
            break;
            case LCD_STAT_MODE_HBLNK:
                gb_mem_map[LCD_LY]++;
                if(gb_mem_map[LCD_LY] > GB_SCREEN_HEIGHT){
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

