#include <SDL2/SDL.h>
#include "system_interface.h"
#include "gb_ppu.h"
#include "debug_print.h"
#include "gb_mem_map.h"
#include <string.h>

#ifdef DEBUG
#define DEBUG_PRINT(fmt, args...)    PRINT(fmt, ## args)
#else
#define DEBUG_PRINT(fmt, args...)
#endif

SDL_Window* window = NULL;
SDL_Renderer *renderer;
SDL_Texture* texture;
SDL_Rect line_rect;
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
        
        // Tell the renderer the content is one Game Boy screen and let it map
        // that onto whatever the output size turns out to be, recomputing if
        // the output changes. RenderSetScale did this by hand, which only held
        // while the output was exactly SCREEN_MULT times the content - and the
        // two conflict, because a logical size sets a scale of its own.
        SDL_RenderSetLogicalSize(renderer, GB_SCREEN_WIDTH, GB_SCREEN_HEIGHT);

#ifndef __EMSCRIPTEN__
        // Native: open the window at a whole multiple of the screen. In a
        // browser the canvas is sized by the page's CSS, so the backing store
        // is left at the content size and the page scales it - which also
        // keeps the upscale crisp via image-rendering: pixelated.
        SDL_SetWindowSize(window, GB_SCREEN_WIDTH * SCREEN_MULT,
                                  GB_SCREEN_HEIGHT * SCREEN_MULT);
#endif
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
            if(g_enable_lcd){
                // A scanline is contiguous, so the whole row is one copy.
                memcpy(pixels, gb_display[line], sizeof(gb_display[line]));
            } else {
                uint32_t* dst = (uint32_t*)pixels;
                for(int x = 0; x < GB_SCREEN_WIDTH; x++){
                    dst[x] = rgb888_green_1;
                }
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
    int quit = 0;
    SDL_Event event;

    // Drain the queue, do not just pump it. SDL_PumpEvents fills the queue and
    // nothing was reading from it, so it grew without bound, the window close
    // button did nothing, and SIGTERM - which SDL turns into SDL_QUIT rather
    // than letting it kill the process - was swallowed too.
    //
    // SDL_PollEvent pumps when the queue is empty, so this also keeps
    // SDL_GetKeyboardState below current.
    while(SDL_PollEvent(&event)){
        switch(event.type){
            case SDL_QUIT:
                quit = 1;
                break;
            case SDL_WINDOWEVENT:
                if(event.window.event == SDL_WINDOWEVENT_CLOSE){
                    quit = 1;
                }
                break;
            default:
                // Everything else is read and discarded; the pad is sampled
                // from the keyboard state rather than from key events.
                break;
        }
    }

    if(quit){
        return KEY_EXIT;
    }

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
