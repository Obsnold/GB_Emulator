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
SDL_Rect line_rect;
SDL_Event event;
#define SCREEN_MULT 3


int init_screen(){
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        DEBUG_PRINT( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
        return 1;
    } else {
        SDL_CreateWindowAndRenderer(GB_SCREEN_WIDTH, GB_SCREEN_HEIGHT, 0, &window, &renderer);
        
        if( !window || !renderer )
        {
            PRINT( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
            return 1;
        } 
        
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, GB_SCREEN_WIDTH, GB_SCREEN_HEIGHT);
        if (!texture) {
            PRINT( "Texture could not be created! SDL_Error: %s\n", SDL_GetError() );
            return 1;
        }
        
        SDL_RenderSetScale(renderer, SCREEN_MULT, SCREEN_MULT);
        SDL_SetWindowSize(window, GB_SCREEN_WIDTH * SCREEN_MULT, GB_SCREEN_HEIGHT * SCREEN_MULT);
        SDL_SetRenderDrawColor(renderer, 0x9B, 0xBC, 0x0F, 0xFF);
        SDL_RenderClear(renderer);
        line_rect.x=0;
        line_rect.y=0;
        line_rect.w=GB_SCREEN_WIDTH;
        line_rect.h=1;
    }

    return 0;
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

bool g_enable_lcd = false; // should maybe move this somewhere else

void enable_lcd(bool set){
    g_enable_lcd = set;
}

int update_screen(uint8_t line){
    void *pixels;
    int pitch;
    
    if(line != line_rect.y && line < GB_SCREEN_HEIGHT){
        line_rect.y = line;
        if (SDL_LockTexture(texture, &line_rect, &pixels, &pitch) < 0) {
            PRINT("Couldn't lock texture: %s\n", SDL_GetError());
            return 1;
        } else {
            uint32_t* dst = (uint32_t*)pixels;
            for(int x = 0; x < GB_SCREEN_WIDTH; x++){
                if(g_enable_lcd){
                    *dst = gb_display[x][line];
                } else {
                    *dst = rgb888_green_1; //should work out which colour this should be set to
                }
                dst++;
            }
            SDL_UnlockTexture(texture);
        }

        if(line == 0){
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
        }
    }
    return 0;
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

    //DEBUG_PRINT("keypress: %d\n", ret);
    return ret;
}
