#include "gb_ppu.h"
#include "gb_mem_map.h"
#include "gb_test_image.h"
#include <time.h>
#include <math.h>


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
#define TILE_BYTE_SIZE 16

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

/*
void gb_draw_tile(int x_bg, int y_bg, uint16_t tile){
    for (int x_tile = 0; x_tile < TILE_SIZE; x_tile++){
        for (int y_tile = 0; y_tile < TILE_SIZE; y_tile++){
            if(x_tile > 3){
                gb_bg_pixels[(x_bg*TILE_SIZE) + x_tile][(y_bg*TILE_SIZE) + y_tile] = 0x00;
            } else {
                gb_bg_pixels[(x_bg*TILE_SIZE) + x_tile][(y_bg*TILE_SIZE) + y_tile] = 0x00;
            }
            //((gb_mem_map[tile+(y_tile*2)] & (0x01 << x_tile)) << 0x01) + ((gb_mem_map[tile+(y_tile*2)] & (0x01 << x_tile)));
        }
    }
}*/
/*
void gb_draw_bg(){
    //check which map to display
    uint16_t bg_map = BG_MAP_1;
    uint16_t window_map = BG_MAP_1;
    uint16_t bg_window_tile_set = VRAM_BLOCK_0;
    int bg_window_tile_mode = GET_MEM_MAP(LCD_CTRL, LCD_CTRL_BG_TILE_SELECT);

    if(GET_MEM_MAP(LCD_CTRL, LCD_CTRL_BG_MAP_SELECT)){
        // BG_MAP_2 9C00-9FFF
        bg_map = BG_MAP_2;
    } else {
        // BG_MAP_1 9800-9BFF
        bg_map = BG_MAP_1;
    }

    if(bg_window_tile_mode == 0){
        bg_window_tile_set = VRAM_BLOCK_0;
    } else {
        bg_window_tile_set = VRAM_BLOCK_2;
    }

    for(int x = 0; x < BG_SIZE; x++){
        for(int y = 0; y < BG_SIZE; y++){
            int tile_1 = gb_mem_map[bg_map + ( x + (y * BG_SIZE))];

            if(bg_window_tile_mode == 1 && tile_1 >= 128){
                tile_1 -= 128;
                bg_window_tile_set = VRAM_BLOCK_1;
            }
            uint16_t tile = gb_mem_map[bg_window_tile_set + (tile_1 * TILE_BYTE_SIZE)];
            gb_draw_tile(x,y,tile);
        }
    }
}*/

void gb_display_sprites(){

    
}



void ppu_oam_search(){
    //search for all visible sprites

}

void ppu_pixel_transfer(){
    uint8_t screen_x = gb_mem_map[LCD_SCX];
    uint8_t screen_y = gb_mem_map[LCD_SCY];
    uint8_t line = gb_mem_map[LCD_LY];
    uint16_t bg_map = BG_MAP_1;
    uint16_t window_map = BG_MAP_1;
    uint16_t bg_window_tile_set = VRAM_BLOCK_0;
    int bg_window_tile_mode = GET_MEM_MAP(LCD_CTRL, LCD_CTRL_BG_TILE_SELECT);

    if(GET_MEM_MAP(LCD_CTRL, LCD_CTRL_BG_MAP_SELECT)){
        // BG_MAP_2 9C00-9FFF
        bg_map = BG_MAP_2;
    } else {
        // BG_MAP_1 9800-9BFF
        bg_map = BG_MAP_1;
    }

    if(bg_window_tile_mode == 0){
        bg_window_tile_set = VRAM_BLOCK_0;
    } else {
        bg_window_tile_set = VRAM_BLOCK_2;
    }
    
    // work out position of start tile in map
    int map_x = screen_x/TILE_SIZE;
    int map_y = (screen_y+line)/TILE_SIZE;


    int tile_x = screen_x - (map_x*TILE_SIZE);
    int tile_y = (screen_y+line) - (map_y*TILE_SIZE);
    
    if(map_y >= BG_SIZE){
        map_y-=BG_SIZE;
    }
    
    int map_line = map_y*32;
    int tile_line_offset = tile_y*2;



    //printf("gb_display tile_x = %u \n", tile_x);
    for(int x = 0; x < 21; x++){
        int x2_offset = 0;
        int x2_compare = TILE_SIZE;
        if((map_x + x) >= BG_SIZE){
            map_x-=BG_SIZE;
        }
        int tile = map_x + x + map_line;
        if(bg_window_tile_mode == 1 && tile >= 128){
                tile -= 128;
                bg_window_tile_set = VRAM_BLOCK_1;
        }
        uint8_t tile_addr = gb_mem_map[bg_map + tile];
        //printf("tile_addr = %04x, bg_map =%04x, tile = %04x\n", tile_addr,bg_map,tile);
        uint16_t tile_line = bg_window_tile_set + (tile_addr*0x10) + tile_line_offset;
        //printf("tile_line = %04x, bg_window_tile_set =%04x, tile_addr = %04x, tile_line_offset= %04x\n", tile_line,bg_window_tile_set,tile_addr,tile_line_offset);
        if(x==0){
            x2_offset = tile_x;
        }
        
        if(x == 20){
            if(tile_x > 0){
                x2_compare = tile_x;
            } else{
                break;
            }
        }

        //printf("gb_display line = %u,x= %d, x2_compare =%d, x2_offset=%d \n", line,x,x2_compare,x2_offset);
        for(int x2 = 0;x2 < x2_compare;x2++){
            uint32_t colour = bg_pallet[0];
            if(gb_mem_map[tile_line] & (0x01 << (x2+x2_offset))){
                if(gb_mem_map[tile_line+1] & (0x01 <<(x2+x2_offset))){
                    colour = bg_pallet[0];
                } else {
                    colour = bg_pallet[1];
                }
            } else {
                if(gb_mem_map[tile_line+1] & (0x01 <<(x2+x2_offset))){
                    colour = bg_pallet[2];
                } else {
                    colour = bg_pallet[3];
                }
            }
            //printf("gb_display x = %u, line = %u,x= %d, x2 =%d, x2_offset=%d \n", ((x*TILE_SIZE) + x2),line,x,x2,x2_offset);
            if(x==0){
                gb_display[(x*TILE_SIZE) + x2][line] = colour;
            } else {
                gb_display[(x*TILE_SIZE) + x2 - tile_x][line] = colour;
            }
        }
    }



    // get the pallet being used
    /*
    // draw the line indicated by LCD_LY
    for(int x = 0; x < GB_SCREEN_WIDTH; x ++){
        uint16_t colour = green_1;
        switch(gb_bg_pixels[screen_x + x][screen_y + line]){
            case 0x00:
            colour = bg_pallet[0];
            break;
            case 0x01:
            colour = bg_pallet[1];
            break;
            case 0x02:
            colour = bg_pallet[2];
            break;
            case 0x03:
            colour = bg_pallet[3];
            break;
        }
        gb_display[x][line] = colour;
    }*/

    // draw window

    // draw sprites

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




/*
void gb_draw_tile_line(int x_pos, int line, int tile_offset, uint16_t tile){
    for (int x = 0; x < TILE_SIZE; x++){
        for (int y = 0; y < TILE_SIZE; y++){
            gb_bg_pixels[(x_tile*TILE_SIZE) + x][(y_tile*TILE_SIZE) + y] = 
            ((gb_mem_map[tile+(y*2)] & 0x01 << x) << 0x01) + ((gb_mem_map[tile+(y*2)] & 0x01 << x));
        }
    }
}*/
