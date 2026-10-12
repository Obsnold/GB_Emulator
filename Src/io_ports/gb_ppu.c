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
uint32_t gb_display[GB_SCREEN_HEIGHT][GB_SCREEN_WIDTH];

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
    uint8_t lcd_line = get_mem_map_8(LCD_LY);
    oam_list_size = 0;
    uint8_t oam_sprite_size = TILE_SIZE;
    if(get_mem_map_bit(LCD_CTRL,LCD_CTRL_OBJ_SIZE) != 0){
        oam_sprite_size = TILE_SIZE * 2;
    } 
    //search for all visible sprites
    for(int i = 0; i < OAM_TABLE_SIZE; i++){
        //check x > 0, check lcd_line >= y,  lcd_line < y+8
        if( (get_mem_map_8(OAM_TABLE + (i*OAM_SIZE) + OAM_X_POS) > 0) &&
            (lcd_line+16 >= get_mem_map_8(OAM_TABLE + (i*OAM_SIZE) + OAM_Y_POS)) &&
            (lcd_line+16 < (get_mem_map_8(OAM_TABLE + (i*OAM_SIZE) + OAM_Y_POS) + oam_sprite_size))){
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
    uint8_t lcd_line = get_mem_map_8(LCD_LY);
    if(lcd_line >= GB_SCREEN_HEIGHT){
        return;     // nothing to draw past the bottom of the buffer
    }

    uint8_t screen_y_pos = get_mem_map_8(LCD_SCY);
    uint8_t screen_x_pos = get_mem_map_8(LCD_SCX);
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
    // The map entry and the two tile bytes serve all 8 pixels of a tile, so
    // they are fetched once per tile rather than once per pixel. That is 60
    // memory map reads per scanline instead of 480.
    uint8_t plane_0 = 0;
    uint8_t plane_1 = 0;
    int need_fetch = 1;

    for(int x = 0; x < GB_SCREEN_WIDTH; x++){
        if(need_fetch){
            int tile = get_mem_map_8(map + map_x_offset + (map_y_offset * 32));
            if(bg_window_tile_mode == 0) {
                if(tile >= 0x80){
                    tile -= 0x100;
                }
            }
            tile *= 16;
            tile += bg_window_tile_set;
            tile += (map_y_tile_offset*2);

            plane_0 = get_mem_map_8(tile);
            plane_1 = get_mem_map_8(tile + 1);
            need_fetch = 0;
        }

        uint8_t mask = (uint8_t)(0x80 >> map_x_tile_offset);
        uint8_t pixel_1 = plane_0 & mask;
        uint8_t pixel_2 = plane_1 & mask;
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
        gb_display[lcd_line][x] = pix;

        map_x_tile_offset++;
        if(map_x_tile_offset >= 8){
            map_x_tile_offset = 0;
            map_x_offset++;
            if(map_x_offset >= 32){
                map_x_offset = map_x_offset - 32;
            }
            need_fetch = 1;     // next tile needs its own fetch
        }
    }
}

void draw_sprites_line(){
    uint8_t lcd_line = get_mem_map_8(LCD_LY);
    if(lcd_line >= GB_SCREEN_HEIGHT){
        return;     // nothing to draw past the bottom of the buffer
    }

    uint32_t* pallet;
    //check the apllet to use
    uint8_t oam_sprite_size = TILE_SIZE;
    if(get_mem_map_bit(LCD_CTRL,LCD_CTRL_OBJ_SIZE) != 0){
        oam_sprite_size = TILE_SIZE * 2;
    } 

    for(int i = 0; i < oam_list_size; i++){
        uint16_t oam = OAM_TABLE + (oam_list[i] * OAM_SIZE);
        uint8_t oam_x_pos = get_mem_map_8(oam + OAM_X_POS);
        uint8_t sprite_line_offset = (lcd_line+16) - get_mem_map_8(oam + OAM_Y_POS);

        if(get_mem_map_bit((oam + OAM_FLAGS), OAM_FLAGS_PALETTE_NO) == 0){
            pallet = sp_pallet_0;
        } else {
            pallet = sp_pallet_1;
        }

        if(get_mem_map_bit((oam + OAM_FLAGS),OAM_FLAGS_Y_FLIP)){
                sprite_line_offset = oam_sprite_size - sprite_line_offset -1;
        }

        uint8_t tile_index = get_mem_map_8(oam + OAM_TILE);
        if(oam_sprite_size == (TILE_SIZE * 2)){
            // In 8x16 mode the hardware ignores bit 0 of the index: the pair
            // drawn is (index & 0xFE, index | 0x01).
            tile_index &= 0xFE;
        }

        uint16_t tile_line = TILE_RAM_0 + (tile_index * TILE_MEM_SIZE) + (sprite_line_offset*2);

        // The two tile bytes and both flag bits are the same for every column
        // of this sprite, so they are read once rather than four times per
        // column.
        uint8_t plane_0 = get_mem_map_8(tile_line);
        uint8_t plane_1 = get_mem_map_8(tile_line + 1);
        int x_flip = get_mem_map_bit((oam + OAM_FLAGS), OAM_FLAGS_X_FLIP);
        // Priority set puts the sprite behind the background, so it only shows
        // through where the background is colour 0.
        int behind_bg = get_mem_map_bit(oam + OAM_FLAGS, OAM_FLAGS_OBJ_BG_PRIORITY);

        for(int x = 0; x < oam_sprite_size; x++){
            uint8_t mask = x_flip ? (uint8_t)(0x01 << x) : (uint8_t)(0x80 >> x);
            uint8_t pixel_1 = plane_0 & mask;
            uint8_t pixel_2 = plane_1 & mask;

            int x_pos = oam_x_pos + x - TILE_SIZE;
            // Drop a column that falls outside the screen. Clamping it to the
            // boundary instead stacked every hidden column onto one pixel, so
            // the edge showed the last hidden column rather than the last
            // visible one.
            if(x_pos < 0 || x_pos >= GB_SCREEN_WIDTH){
                continue;
            }

            if(!behind_bg || (gb_display[lcd_line][x_pos] == bg_pallet[0])){
                if(pixel_1){
                    if(pixel_2){
                        gb_display[lcd_line][x_pos] = pallet[3];
                    } else {
                        gb_display[lcd_line][x_pos] = pallet[1];
                    }
                } else {
                    if(pixel_2){
                        gb_display[lcd_line][x_pos] = pallet[2];
                    }
                }
            }
        }
    }
}

void draw_window_line(){
    uint8_t lcd_line = get_mem_map_8(LCD_LY);
    if(lcd_line >= GB_SCREEN_HEIGHT){
        return;     // nothing to draw past the bottom of the buffer
    }

    if(get_mem_map_8(LCD_WY) <= lcd_line){
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

        uint8_t window_line = (lcd_line - get_mem_map_8(LCD_WY));
        // WX is unsigned and the left edge is WX-7, so this needs more range
        // than an int8_t: WX=0xF8 gives 241, which wrapped to -8 and then
        // clamped to 0, painting the window across the whole scanline.
        int16_t window_x = (int16_t)get_mem_map_8(LCD_WX) - 7;
        uint16_t map_y_offset = window_line /8;
        uint16_t map_x_offset = 0;
        uint8_t tile_y_offset = window_line % 8;
        uint8_t tile_x_offset = 0;

        if(window_x >= GB_SCREEN_WIDTH){
            return;     // left edge is off the right of the screen
        }
        if(window_x < 0){
            window_x = 0;
        }

        // Same per-tile fetch as the background: the map entry and the two
        // tile bytes cover 8 pixels.
        uint8_t plane_0 = 0;
        uint8_t plane_1 = 0;
        int need_fetch = 1;

        for(int16_t x = window_x; x < GB_SCREEN_WIDTH; x++){
            if(need_fetch){
                int32_t tile = (get_mem_map_8(map + map_x_offset + (map_y_offset * 32)));
                if(bg_window_tile_mode == 0) {
                    if(tile >= 0x80){
                        tile -= 0x100;
                    }
                }
                tile *= 16;
                tile += bg_window_tile_set;
                tile += (tile_y_offset*2);

                plane_0 = get_mem_map_8(tile);
                plane_1 = get_mem_map_8(tile + 1);
                need_fetch = 0;
            }

            uint8_t mask = (uint8_t)(0x80 >> tile_x_offset);
            uint8_t pixel_1 = plane_0 & mask;
            uint8_t pixel_2 = plane_1 & mask;
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
            gb_display[lcd_line][x] = pix;

            tile_x_offset++;
            if(tile_x_offset >= 8){
                tile_x_offset = 0;
                map_x_offset++;
                if(map_x_offset >= 32){
                    map_x_offset = map_x_offset - 32;
                }
                need_fetch = 1;
            }
        }
    }
}


void ppu_h_blank(){
    uint8_t temp = get_mem_map_8(LCD_LY);
    temp++;
    set_mem_map_8(LCD_LY,temp);
}

void ppu_v_blank(){
    uint8_t temp = get_mem_map_8(LCD_LY);
    temp++;
    set_mem_map_8(LCD_LY,temp);
}

void oam_dma(){
    uint16_t source = get_mem_map_8(LCD_DMA) * 0x100;
    for(uint16_t i = 0; i < 0xA0; i++){
        uint8_t temp = get_mem_map_8(source+i);
        set_mem_map_8(OAM_TABLE+i,temp);
    }
}


uint8_t ppu(unsigned long delta_time){
    if(get_mem_map_bit(LCD_CTRL,LCD_CTRL_ENABLE) ){
        lcd_enabled = true;

        //have we received a oma dma request?
        if(get_mem_map_8(LCD_DMA) != 0){
            oam_dma();
            set_mem_map_8(LCD_DMA,0);
        }

        // get time
        ppu_cycles += delta_time;

        if (ppu_cycles > (ppu_cycles_count* CYCLE_TIME)){
            DEBUG_PRINT("ppu_cycles= %lu ppu_cycles_count = %lu\n",ppu_cycles, ppu_cycles_count*CYCLE_TIME);
            ppu_cycles -= (ppu_cycles_count* CYCLE_TIME);
            switch (get_mem_map_bit(LCD_STAT,LCD_STAT_MODE)){
                case LCD_STAT_MODE_OAM:
                    ppu_oam_search();
                    if(get_mem_map_bit(LCD_STAT,LCD_STAT_OAM_INTR_EN)){
                        set_mem_map_bit(INTERRUPT_FLAGS,INTERRUPT_LCD_STAT);
                    }

                    //---------------------------------------------------------
                    clear_mem_map_bit(LCD_STAT,LCD_STAT_MODE);
                    set_mem_map_bit(LCD_STAT,LCD_STAT_MODE_PIXEL_TRANS);
                    ppu_cycles_count = PIXEL_TRANSFER_CYCLES;
                break;
                case LCD_STAT_MODE_PIXEL_TRANS:
                    set_pallets();
                    draw_background_line();
                    if(get_mem_map_bit(LCD_CTRL,LCD_CTRL_OBJ_ENABLE)){
                        draw_sprites_line();
                    }
                    if(get_mem_map_bit(LCD_CTRL,LCD_CTRL_WINDOW_ENABLE) && (get_mem_map_8(LCD_LY) >= get_mem_map_8(LCD_WY))){
                        draw_window_line();
                    }

                    //---------------------------------------------------------
                    clear_mem_map_bit(LCD_STAT,LCD_STAT_MODE);
                    set_mem_map_bit(LCD_STAT,LCD_STAT_MODE_HBLNK);
                    ppu_cycles_count = H_BLANK_CYCLES;
                break;
                case LCD_STAT_MODE_HBLNK:
                    ppu_h_blank();
                    if(get_mem_map_bit(LCD_STAT,LCD_STAT_HBLNK_INTR_EN)){
                        set_mem_map_bit(INTERRUPT_FLAGS,INTERRUPT_LCD_STAT);
                    }

                    //---------------------------------------------------------
                    if(get_mem_map_8(LCD_LY) >= GB_SCREEN_HEIGHT){
                        clear_mem_map_bit(LCD_STAT,LCD_STAT_MODE);
                        set_mem_map_bit(LCD_STAT,LCD_STAT_MODE_VBLNK);
                        ppu_cycles_count = V_BLANK_STEP_CYCLES;
                        // One v-blank per frame, raised on entry to mode 1.
                        // Raising it on each of the ten v-blank scanlines ran
                        // a game's handler ten times a frame.
                        set_mem_map_bit(INTERRUPT_FLAGS,INTERRUPT_V_BLANK);
                    } else {
                        clear_mem_map_bit(LCD_STAT,LCD_STAT_MODE);
                        set_mem_map_bit(LCD_STAT,LCD_STAT_MODE_OAM);
                        ppu_cycles_count = OAM_SEARCH_CYCLES;
                    }
                    if(get_mem_map_bit(LCD_STAT,LCD_STAT_LCY_INTR_EN)){
                        if(get_mem_map_8(LCD_LY) == get_mem_map_8(LCD_LYC)){
                            set_mem_map_bit(INTERRUPT_FLAGS,INTERRUPT_LCD_STAT);
                        }
                    }
                break;
                case LCD_STAT_MODE_VBLNK:
                    ppu_v_blank();
                    if(get_mem_map_bit(LCD_STAT,LCD_STAT_VBLNK_INTR_EN)){
                        set_mem_map_bit(INTERRUPT_FLAGS,INTERRUPT_LCD_STAT);
                    }

                    //---------------------------------------------------------
                    if(get_mem_map_8(LCD_LY) > (GB_SCREEN_HEIGHT + GB_SCREEN_HEIGHT_V_BLANK)){
                        set_mem_map_8(LCD_LY, 0);
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
        if(lcd_enabled == true){
            lcd_enabled = false;
            set_mem_map_8(LCD_LY, 0);
            set_mem_map_8(LCD_STAT,0x00);
            set_mem_map_bit(LCD_STAT,LCD_STAT_MODE_OAM);
            ppu_cycles_count = OAM_SEARCH_CYCLES;
            ppu_cycles = 0;
            prev_tick = 0;
            set_mem_map_bit(INTERRUPT_FLAGS,INTERRUPT_V_BLANK);
            set_mem_map_bit(INTERRUPT_FLAGS,INTERRUPT_LCD_STAT);
        }
    }
    return 0;
}

