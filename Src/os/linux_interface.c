#include <SDL2/SDL.h>
#include "system_interface.h"
#include "gb_ppu.h"
#include "debug_print.h"
#include "gb_mem_map.h"

#ifdef DEBUG
#define DEBUG_PRINT(fmt, args...)    PRINT(fmt, ## args)
#else
#define DEBUG_PRINT(fmt, args...)
#endif

SDL_Window* window = NULL;
SDL_Renderer *renderer;
SDL_Texture* texture;
//Event handler
SDL_Event e;
#define SCREEN_MULT 3
#define RGB_CHANNELS 3
char pixel_screen[GB_SCREEN_HEIGHT * GB_SCREEN_WIDTH * RGB_CHANNELS * SCREEN_MULT];

void init_screen(){
    memset(pixel_screen,0,GB_SCREEN_HEIGHT * GB_SCREEN_WIDTH * RGB_CHANNELS);
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        DEBUG_PRINT( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
    } else {
        SDL_CreateWindowAndRenderer(GB_SCREEN_WIDTH*SCREEN_MULT, GB_SCREEN_HEIGHT*SCREEN_MULT, 0, &window, &renderer);
        //window = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, GB_SCREEN_WIDTH, GB_SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
        //mTexture = SDL_CreateTexture( gRenderer, SDL_PIXELFORMAT_RGB565 SDL_TEXTUREACCESS_STREAMING, width, height );
        if( window == NULL || renderer == NULL)
        {
            DEBUG_PRINT( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
        } else{
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
            SDL_RenderClear(renderer);
            SDL_SetRenderDrawColor(renderer, 0x9B, 0xBC, 0x0F, 0xFF);
        } 
    }

}

void free_screen(){
    SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow( window );
	SDL_Quit();
}

#define rgb888_green_1 0x9BBC0F
#define rgb888_green_2 0x8BAC0F
#define rgb888_green_3 0x306230
#define rgb888_green_4 0x0F380F

int gLine = 0;
bool g_enable_lcd = false;

void enable_lcd(bool set){
    g_enable_lcd = set;
}


void update_screen(uint8_t line){
    
    if(line != gLine){
        gLine = line;
        if(g_enable_lcd){
            for(int x = 0; x < GB_SCREEN_WIDTH; x++){
                int pixel = (line * GB_SCREEN_WIDTH * RGB_CHANNELS)+(x*RGB_CHANNELS);
                pixel_screen[pixel] = ((gb_display[x][line] & 0xFF0000) >> 0x10);
                pixel_screen[pixel+1] = ((gb_display[x][line] & 0xFF00) >> 0x08);
                pixel_screen[pixel+2] = (gb_display[x][line] & 0xFF);
            }
        } else {
            memset(pixel_screen, 0x00, (GB_SCREEN_HEIGHT * GB_SCREEN_WIDTH * RGB_CHANNELS * SCREEN_MULT));
        }

        if(line == 0){
            DEBUG_PRINT("Print screen\n");
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            SDL_Surface *surface = SDL_CreateRGBSurfaceFrom((void*)pixel_screen,
                        GB_SCREEN_WIDTH,
                        GB_SCREEN_HEIGHT,
                        RGB_CHANNELS * 8,          // bits per pixel = 24
                        GB_SCREEN_WIDTH * RGB_CHANNELS,  // pitch
                        0x0000FF,              // red mask
                        0x00FF00,              // green mask
                        0xFF0000,              // blue mask
                        0);
            texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            
            SDL_RenderPresent(renderer);
        }
    }
}

uint16_t get_key_press(){
    int ret = 0xFF;
    int numkeys = 0;

    SDL_PumpEvents();

    const uint8_t *array = SDL_GetKeyboardState(&numkeys);
    
    // handle normal key presses
    if( array[SDL_SCANCODE_UP] == 1 )
    {
        DEBUG_PRINT("KEY_UP\n");
        ret ^= KEY_UP;
    }
    if( array[SDL_SCANCODE_DOWN] == 1 )
    {
        DEBUG_PRINT("KEY_DOWN\n");
        ret ^= KEY_DOWN;
    }
    if( array[SDL_SCANCODE_LEFT] == 1 )
    {
        DEBUG_PRINT("KEY_LEFT\n");
        ret ^= KEY_LEFT;
    }
    if( array[SDL_SCANCODE_RIGHT] == 1 )
    {
        DEBUG_PRINT("KEY_RIGHT\n");
        ret ^= KEY_RIGHT;
    }
    if( array[SDL_SCANCODE_A] == 1 )
    {
        DEBUG_PRINT("KEY_A\n");
        ret ^= KEY_A;
    }
    if( array[SDL_SCANCODE_S] == 1 )
    {
        DEBUG_PRINT("KEY_B\n");
        ret ^= KEY_B;
    }
    if( array[SDL_SCANCODE_RETURN] == 1 )
    {
        DEBUG_PRINT("KEY_START\n");
        ret ^= KEY_START;
    }
    if( array[SDL_SCANCODE_SPACE] == 1 )
    {
        DEBUG_PRINT("KEY_SELECT\n");
        ret ^= KEY_SELECT;
    }

    // hanlde special key presses
    if( array[SDL_SCANCODE_BACKSPACE] == 1 )
    {
        ret = KEY_DEBUG;
    }
    if( array[SDL_SCANCODE_ESCAPE] == 1 )
    {
        ret = KEY_EXIT;
    }

    DEBUG_PRINT("keypress: %d\n", ret);
    return ret;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//uint8_t debug_mode = 0x00;
//128*192 pixels to display all tiles
SDL_Window* d_window = NULL;
SDL_Renderer *d_renderer;
SDL_Texture* d_texture;
#define DEBUG_SCREEN_HEIGHT 192
#define DEBUG_SCREEN_WIDTH 128

char d_pixel_screen[DEBUG_SCREEN_HEIGHT * DEBUG_SCREEN_WIDTH * RGB_CHANNELS * SCREEN_MULT];

void debug_screen_init(){
    memset(d_pixel_screen,0,DEBUG_SCREEN_HEIGHT * DEBUG_SCREEN_WIDTH * RGB_CHANNELS);
    SDL_CreateWindowAndRenderer(DEBUG_SCREEN_WIDTH*SCREEN_MULT, DEBUG_SCREEN_HEIGHT*SCREEN_MULT, 0, &d_window, &d_renderer);
    if( d_window == NULL || d_renderer == NULL)
    {
        DEBUG_PRINT( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
    } else {
        SDL_SetRenderDrawColor(d_renderer, 0, 0, 0, 0);
        SDL_RenderClear(d_renderer);
        SDL_SetRenderDrawColor(d_renderer, 0x9B, 0xBC, 0x0F, 0xFF);
    }
}

void draw_tile(uint8_t x_pos, uint8_t y_pos, int tile_address){
    for(int x = 0; x < 8; x++){
        for(int y = 0; y < 8; y++){
            uint8_t pixel_1 = (gb_mem_map[tile_address + (y*2)] & (0x80 >> x));
            uint8_t pixel_2 = (gb_mem_map[tile_address + (y*2) + 1] & (0x80 >> x));
            //PRINT("%02x, %02x\n",pixel_1,pixel_2);
            int pix = 0;
            if(pixel_1){
                if(pixel_2){
                    pix = PAL_BLACK;
                } else {
                    pix = PAL_D_GRAY;
                }
            } else {
                if(pixel_2){
                    pix = PAL_L_GRAY;
                } else {
                    pix = PAL_WHITE;
                }
            }
            d_pixel_screen[(x_pos * RGB_CHANNELS) + (x * RGB_CHANNELS) + (y_pos * DEBUG_SCREEN_WIDTH * RGB_CHANNELS) + (y * DEBUG_SCREEN_WIDTH * RGB_CHANNELS)] = ((pix & 0xFF0000) >> 0x10);
            d_pixel_screen[(x_pos * RGB_CHANNELS) + (x * RGB_CHANNELS) + (y_pos * DEBUG_SCREEN_WIDTH * RGB_CHANNELS) + (y * DEBUG_SCREEN_WIDTH * RGB_CHANNELS)+1] = ((pix & 0xFF00) >> 0x08);
            d_pixel_screen[(x_pos * RGB_CHANNELS) + (x * RGB_CHANNELS) + (y_pos * DEBUG_SCREEN_WIDTH * RGB_CHANNELS) + (y * DEBUG_SCREEN_WIDTH * RGB_CHANNELS)+2] = (pix & 0xFF);
        }
    }
}

void debug_screen_free(){
    SDL_DestroyRenderer(d_renderer);
	SDL_DestroyWindow(d_window);
}

void debug_screen(){
    for(int i = 0; i < 16; i++){ // columns
        for(int j = 0; j < 24; j++){ //Rows
            draw_tile(i*8, j*8, TILE_RAM_0+((i*16) + (j*16*16)));
        }
    }

    SDL_SetRenderDrawColor(d_renderer, 0, 0, 0, 255);
    SDL_RenderClear(d_renderer);
    SDL_Surface *surface = SDL_CreateRGBSurfaceFrom((void*)d_pixel_screen,
                DEBUG_SCREEN_WIDTH,
                DEBUG_SCREEN_HEIGHT,
                RGB_CHANNELS * 8,          // bits per pixel = 24
                DEBUG_SCREEN_WIDTH * RGB_CHANNELS,  // pitch
                0x0000FF,              // red mask
                0x00FF00,              // green mask
                0xFF0000,              // blue mask
                0);
    d_texture = SDL_CreateTextureFromSurface(d_renderer, surface);
    SDL_FreeSurface(surface);
    SDL_RenderCopy(d_renderer, d_texture, NULL, NULL);
    
    SDL_RenderPresent(d_renderer);
}
