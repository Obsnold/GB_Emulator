#include "gb_ppu.h"
#include "gb_mem_map.h"
#include "gb_test_image.h"
#include "debug_print.h"
#include <string.h>
#include <stdbool.h>

#ifdef DEBUG
#define DEBUG_PRINT(fmt, args...)    PRINT(fmt, ## args)
#else
#define DEBUG_PRINT(fmt, args...)
#endif

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

uint32_t display_line[GB_SCREEN_WIDTH];

#define OAM_SEARCH_CYCLES 20
#define PIXEL_TRANSFER_CYCLES 43
#define H_BLANK_CYCLES 51
#define V_BLANK_STEP_CYCLES 114  //1140/10ish
#define V_BLANK_CYCLES 1140

unsigned long prev_tick = 0;
unsigned long ppu_cycles = 0;
unsigned long ppu_cycles_count = OAM_SEARCH_CYCLES;
uint8_t oam_list[10];
uint8_t oam_list_size = 0;

uint32_t bg_pallet[4] = {PAL_WHITE, PAL_L_GRAY, PAL_D_GRAY, PAL_BLACK};
uint32_t sp_pallet_0[4] = {PAL_TRANS, PAL_WHITE, PAL_L_GRAY, PAL_D_GRAY};
uint32_t sp_pallet_1[4] = {PAL_TRANS, PAL_WHITE, PAL_L_GRAY, PAL_D_GRAY};

bool lcd_enabled = false;


void ppu_oam_search(){
    uint8_t lcd_line = gb_mem_map[LCD_LY];
    oam_list_size = 0;
    uint8_t oam_sprite_size = TILE_SIZE;
    if(get_mem_map_bit(LCD_CTRL,LCD_CTRL_OBJ_SIZE) != 0){
        oam_sprite_size = TILE_SIZE * 2;
    } 
    //search for all visible sprites
    for(int i = 0; i < OAM_TABLE_SIZE; i++){
        //check x > 0, check lcd_line >= y,  lcd_line < y+8
        if( (gb_mem_map[OAM_TABLE + (i*OAM_SIZE) + OAM_X_POS] > 0) &&
            (lcd_line+16 >= gb_mem_map[OAM_TABLE + (i*OAM_SIZE) + OAM_Y_POS]) &&
            (lcd_line+16 < (gb_mem_map[OAM_TABLE + (i*OAM_SIZE) + OAM_Y_POS] + oam_sprite_size))){
                //if sprite is on the lcd_line then add to list
                oam_list[oam_list_size] = i;
                oam_list_size++;
                //limit of 10 sprites per lcd_line
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

void set_pallets(){
    for(int i = 0; i < 4; i++){
        //set the correct background palette
        switch(get_mem_map_bit(LCD_BGP,0x03 << (i*2)) >> (i*2)){
            case 0:
            bg_pallet[i] = PAL_WHITE;
            break;
            case 1:
            bg_pallet[i] = PAL_L_GRAY;
            break;
            case 2:
            bg_pallet[i] = PAL_D_GRAY;
            break;
            case 3:
            bg_pallet[i] = PAL_BLACK;
            break;
        }

        //set the correct OAM palettes
        switch(get_mem_map_bit(LCD_OBP0,0x03 << (i*2)) >> (i*2)){
            case 0:
            sp_pallet_0[i] = PAL_WHITE;
            break;
            case 1:
            sp_pallet_0[i] = PAL_L_GRAY;
            break;
            case 2:
            sp_pallet_0[i] = PAL_D_GRAY;
            break;
            case 3:
            sp_pallet_0[i] = PAL_BLACK;
            break;
        }

        switch(get_mem_map_bit(LCD_OBP1,0x03 << (i*2)) >> (i*2)){
            case 0:
            sp_pallet_1[i] = PAL_WHITE;
            break;
            case 1:
            sp_pallet_1[i] = PAL_L_GRAY;
            break;
            case 2:
            sp_pallet_1[i] = PAL_D_GRAY;
            break;
            case 3:
            sp_pallet_1[i] = PAL_BLACK;
            break;
        }
    }
    sp_pallet_0[0]=PAL_TRANS;
    sp_pallet_1[0]=PAL_TRANS;

}

void draw_background_line(){
    uint8_t lcd_line = gb_mem_map[LCD_LY];
    uint8_t screen_y_pos = gb_mem_map[LCD_SCY];
    uint8_t screen_x_pos = gb_mem_map[LCD_SCX];
    uint16_t map = 0; // the map address with references to the tiles
    uint16_t bg_window_tile_set = 0; //the address of the tile map set
    uint8_t bg_window_tile_mode = get_mem_map_bit(LCD_CTRL, LCD_CTRL_BG_W_TILE_SELECT); // which addressing method are we using
    if( bg_window_tile_mode == 0){
        bg_window_tile_set = TILE_RAM_2;
    } else {
        bg_window_tile_set = TILE_RAM_0;
    }
    if(get_mem_map_bit(LCD_CTRL, LCD_CTRL_BG_MAP_SELECT)){
        map = BG_MAP_2;
    } else {
        map = BG_MAP_1;
    }

    uint8_t map_y_offset = ((lcd_line + screen_y_pos) / 8);
    uint8_t map_y_tile_offset = ((lcd_line + screen_y_pos) % 8);
    uint8_t map_x_offset = (screen_x_pos / 8);
    uint8_t map_x_tile_offset = (screen_x_pos % 8);

    if(map_y_offset >= 32){
        map_y_offset = map_y_offset - 32;
    }
    for(int x = 0; x < GB_SCREEN_WIDTH; x++){
        int tile = (gb_mem_map[map + map_x_offset + (map_y_offset * 32)]);
        if(bg_window_tile_mode == 0) {
            if(tile >= 0x80){
                tile -= 0x100;
            }
        }
        tile *= 16;
        tile += bg_window_tile_set;  
        tile += (map_y_tile_offset*2);
        
        uint8_t pixel_1 = (gb_mem_map[tile] & (0x80 >> map_x_tile_offset));
        uint8_t pixel_2 = (gb_mem_map[tile + 1] & (0x80 >> map_x_tile_offset));
        int pix = 0;
        if(pixel_1){
            if(pixel_2){
                pix = bg_pallet[3];
            } else {
                pix = bg_pallet[1];
            }
        } else {
            if(pixel_2){
                pix = bg_pallet[2];
            } else {
                pix = bg_pallet[0];
            }
        }
        gb_display[x][lcd_line] = pix;
        
        map_x_tile_offset++;
        if(map_x_tile_offset >= 8){
            map_x_tile_offset = 0;
            map_x_offset++;
            if(map_x_offset >= 32){
                map_x_offset = map_x_offset - 32;
            }
        }
    }
}

void draw_sprites_line(){
    uint8_t lcd_line = gb_mem_map[LCD_LY];
    uint32_t* pallet;
    //check the apllet to use
    uint8_t oam_sprite_size = TILE_SIZE;
    if(get_mem_map_bit(LCD_CTRL,LCD_CTRL_OBJ_SIZE) != 0){
        oam_sprite_size = TILE_SIZE * 2;
    } 

    for(int i = 0; i < oam_list_size; i++){
        uint16_t oam = OAM_TABLE + (oam_list[i] * OAM_SIZE);
        uint8_t oam_x_pos = gb_mem_map[oam + OAM_X_POS];
        uint8_t sprite_line_offset = (lcd_line+16) - gb_mem_map[oam + OAM_Y_POS];

        if(get_mem_map_bit((oam + OAM_FLAGS), OAM_FLAGS_PALETTE_NO) == 0){
            pallet = sp_pallet_0;
        } else {
            pallet = sp_pallet_1;
        }

        if(get_mem_map_bit((oam + OAM_FLAGS),OAM_FLAGS_Y_FLIP)){
                sprite_line_offset = oam_sprite_size - sprite_line_offset -1;
        }

        uint16_t tile_line = TILE_RAM_0 + (gb_mem_map[oam + OAM_TILE] * TILE_MEM_SIZE) + (sprite_line_offset*2);

        for(int x = 0; x < oam_sprite_size; x++){
            uint8_t pixel_1 = 0;
            uint8_t pixel_2 = 0;

            if(get_mem_map_bit((oam + OAM_FLAGS),OAM_FLAGS_X_FLIP)){
                pixel_1 = (gb_mem_map[tile_line] & (0x01 << x));
                pixel_2 = (gb_mem_map[tile_line + 1] & (0x01 << x));
            } else {
                pixel_1 = (gb_mem_map[tile_line] & (0x80 >> x));
                pixel_2 = (gb_mem_map[tile_line + 1] & (0x80 >> x));
            }

            int x_pos = oam_x_pos + x - TILE_SIZE;
            if(x_pos < 0){
                x_pos = 0;
            } else if (x_pos >= GB_SCREEN_WIDTH){
                x_pos = GB_SCREEN_WIDTH -1;
            }
            int priority = get_mem_map_bit(oam + OAM_FLAGS,OAM_FLAGS_OBJ_BG_PRIORITY);

            if((priority == 0) || 
             (gb_display[x_pos][lcd_line] == bg_pallet[0] && priority > 0)){
                if(pixel_1){
                    if(pixel_2){
                        gb_display[x_pos][lcd_line] = pallet[3];
                    } else {
                        gb_display[x_pos][lcd_line] = pallet[1];
                    }
                } else {
                    if(pixel_2){
                        gb_display[x_pos][lcd_line] = pallet[2];
                    } 
                }
            }
        }
    }
}

void draw_window_line(){
    uint8_t lcd_line = gb_mem_map[LCD_LY];
    if(gb_mem_map[LCD_WY] <= lcd_line){
        uint16_t bg_window_tile_set = 0; //the address of the tile map set
        uint8_t bg_window_tile_mode = get_mem_map_bit(LCD_CTRL, LCD_CTRL_BG_W_TILE_SELECT); // which addressing method are we using
        if( bg_window_tile_mode == 0){
            bg_window_tile_set = TILE_RAM_2;
        } else {
            bg_window_tile_set = TILE_RAM_0;
        }
        uint16_t map = BG_MAP_1;
        if(get_mem_map_bit(LCD_CTRL, LCD_CTRL_WINDOW_MAP_SELECT)){
            map = BG_MAP_2;
        }

        uint8_t window_line = (lcd_line - gb_mem_map[LCD_WY]);
        int8_t window_x = gb_mem_map[LCD_WX] - 7;
        uint16_t map_y_offset = window_line /8;
        uint16_t map_x_offset = 0;
        uint8_t tile_y_offset = window_line % 8;
        uint8_t tile_x_offset = 0;

        if(window_x < 0){
            window_x = 0;
        } 

        for(int16_t x = window_x; x < GB_SCREEN_WIDTH; x++){
            int32_t tile = (gb_mem_map[map + map_x_offset + (map_y_offset * 32)]);
            if(bg_window_tile_mode == 0) {
                if(tile >= 0x80){
                    tile -= 0x100;
                }
            }
            tile *= 16;
            tile += bg_window_tile_set;  
            tile += (tile_y_offset*2);
            
            uint8_t pixel_1 = (gb_mem_map[tile] & (0x80 >> tile_x_offset));
            uint8_t pixel_2 = (gb_mem_map[tile + 1] & (0x80 >> tile_x_offset));
            int pix = 0;
            if(pixel_1){
                if(pixel_2){
                    pix = bg_pallet[3];
                } else {
                    pix = bg_pallet[1];
                }
            } else {
                if(pixel_2){
                    pix = bg_pallet[2];
                } else {
                    pix = bg_pallet[0];
                }
            }
            gb_display[x][lcd_line] = pix;
            
            tile_x_offset++;
            if(tile_x_offset >= 8){
                tile_x_offset = 0;
                map_x_offset++;
                if(map_x_offset >= 32){
                    map_x_offset = map_x_offset - 32;
                }
            }
        }


    }
}


void ppu_h_blank(){
    gb_mem_map[LCD_LY]++;
}

void ppu_v_blank(){
    //set interrupt
    set_mem_map_bit(INTERRUPT_FLAGS,INTERRUPT_V_BLANK);
    gb_mem_map[LCD_LY]++;
}

void oam_dma(){
    uint16_t source = gb_mem_map[LCD_DMA] * 0x100;
    memcpy(&gb_mem_map[OAM_TABLE],&gb_mem_map[source],0xA0);
}


uint8_t ppu(){
    if(get_mem_map_bit(LCD_CTRL,LCD_CTRL_ENABLE) ){
        uint8_t ppu_mode = get_mem_map_bit(LCD_STAT,LCD_STAT_MODE);

        //have we received a oma dma request?
        if(gb_mem_map[LCD_DMA] != 0){
            oam_dma();
            gb_mem_map[LCD_DMA]=0;
        }


        // if we are in a new mode run the appropriate code
        if(ppu_cycles == 0){
            switch(ppu_mode){
                case LCD_STAT_MODE_OAM:
                    ppu_oam_search();
                    if(get_mem_map_bit(LCD_STAT,LCD_STAT_OAM_INTR_EN)){
                        set_mem_map_bit(INTERRUPT_FLAGS,INTERRUPT_LCD_STAT);
                    }
                break;
                case LCD_STAT_MODE_PIXEL_TRANS:
                    //ppu_pixel_transfer();
                    set_pallets();
                    draw_background_line();
                    if(get_mem_map_bit(LCD_CTRL,LCD_CTRL_OBJ_ENABLE)){
                        draw_sprites_line();
                    }
                    if(get_mem_map_bit(LCD_CTRL,LCD_CTRL_WINDOW_ENABLE) && (gb_mem_map[LCD_LY] >= gb_mem_map[LCD_WY])){
                        draw_window_line();
                    }
                break;
                case LCD_STAT_MODE_HBLNK:
                    ppu_h_blank();
                    if(get_mem_map_bit(LCD_STAT,LCD_STAT_HBLNK_INTR_EN)){
                        set_mem_map_bit(INTERRUPT_FLAGS,INTERRUPT_LCD_STAT);
                    }
                break;
                case LCD_STAT_MODE_VBLNK:
                    ppu_v_blank();
                    if(get_mem_map_bit(LCD_STAT,LCD_STAT_VBLNK_INTR_EN)){
                        set_mem_map_bit(INTERRUPT_FLAGS,INTERRUPT_LCD_STAT);
                    }
                break;
                default:
                //error
                break;
            }
        }

        // get time 
        unsigned long tick = get_ns();
        ppu_cycles += tick - prev_tick;
        prev_tick = tick;

    
    
        // If the cycle time has elapsed set the appropriate flags and change the cycle count
        if (ppu_cycles > (ppu_cycles_count* CYCLE_TIME)){
            DEBUG_PRINT("ppu_cycles= %ld ppu_cycles_count = %ld\n",ppu_cycles, ppu_cycles_count*250);
            ppu_cycles = 0;
            switch (ppu_mode){
                case LCD_STAT_MODE_OAM:
                    clear_mem_map_bit(LCD_STAT,LCD_STAT_MODE);
                    set_mem_map_bit(LCD_STAT,LCD_STAT_MODE_PIXEL_TRANS);
                    ppu_cycles_count = PIXEL_TRANSFER_CYCLES;
                break;
                case LCD_STAT_MODE_PIXEL_TRANS:
                    clear_mem_map_bit(LCD_STAT,LCD_STAT_MODE);
                    set_mem_map_bit(LCD_STAT,LCD_STAT_MODE_HBLNK);
                    ppu_cycles_count = H_BLANK_CYCLES;
                break;
                case LCD_STAT_MODE_HBLNK:
                    if(gb_mem_map[LCD_LY] >= GB_SCREEN_HEIGHT){
                        clear_mem_map_bit(LCD_STAT,LCD_STAT_MODE);
                        set_mem_map_bit(LCD_STAT,LCD_STAT_MODE_VBLNK);
                        ppu_cycles_count = V_BLANK_STEP_CYCLES;
                    } else {
                        clear_mem_map_bit(LCD_STAT,LCD_STAT_MODE);
                        set_mem_map_bit(LCD_STAT,LCD_STAT_MODE_OAM);
                        ppu_cycles_count = OAM_SEARCH_CYCLES;
                    }
                    if(get_mem_map_bit(LCD_STAT,LCD_STAT_LCY_INTR_EN)){
                        if(gb_mem_map[LCD_LY] == gb_mem_map[LCD_LYC]){
                            set_mem_map_bit(INTERRUPT_FLAGS,INTERRUPT_LCD_STAT);
                        }
                    }
                break;
                case LCD_STAT_MODE_VBLNK:
                    if(gb_mem_map[LCD_LY] > (GB_SCREEN_HEIGHT + GB_SCREEN_HEIGHT_V_BLANK)){
                        gb_mem_map[LCD_LY] = 0;
                        clear_mem_map_bit(LCD_STAT,LCD_STAT_MODE);
                        set_mem_map_bit(LCD_STAT,LCD_STAT_MODE_OAM);
                        ppu_cycles_count = OAM_SEARCH_CYCLES;
                    }
                break;
                default:
                //error
                break;
            }
        }
    } else {
        if(lcd_enabled != get_mem_map_bit(LCD_CTRL,LCD_CTRL_ENABLE)){
            gb_mem_map[LCD_LY] = 0;
            uint32_t temp23 = 0xFF;
            clear_mem_map_bit(LCD_STAT,temp23);
            set_mem_map_bit(LCD_STAT,LCD_STAT_MODE_OAM);
            ppu_cycles_count = OAM_SEARCH_CYCLES;
            ppu_cycles = 0;
            prev_tick = 0;
          /*  set_mem_map_bit(INTERRUPT_FLAGS,INTERRUPT_V_BLANK);
            set_mem_map_bit(INTERRUPT_FLAGS,INTERRUPT_LCD_STAT);*/
            printf("asdaksdajksdh\n");
        }
        lcd_enabled = get_mem_map_bit(LCD_CTRL,LCD_CTRL_ENABLE);
    }
    return 0;
}

