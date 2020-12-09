#include <SDL2/SDL.h>
#include "system_interface.h"
#include "gb_ppu.h"

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
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
    } else {
        SDL_CreateWindowAndRenderer(GB_SCREEN_WIDTH*SCREEN_MULT, GB_SCREEN_HEIGHT*SCREEN_MULT, 0, &window, &renderer);
        //window = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, GB_SCREEN_WIDTH, GB_SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
        //mTexture = SDL_CreateTexture( gRenderer, SDL_PIXELFORMAT_RGB565 SDL_TEXTUREACCESS_STREAMING, width, height );
        if( window == NULL || renderer == NULL)
        {
            printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
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

int print = 1;

void update_screen(uint8_t line){
    for(int x = 0; x < GB_SCREEN_WIDTH; x++){
        //for(int i = 0; i < 2; i++){
            int pixel = (line * GB_SCREEN_WIDTH * RGB_CHANNELS)+(x*RGB_CHANNELS);
            pixel_screen[pixel] = ((gb_display[x][line] & 0xFF0000) >> 0x10);
            pixel_screen[pixel+1] = ((gb_display[x][line] & 0xFF00) >> 0x08);
            pixel_screen[pixel+2] = (gb_display[x][line] & 0xFF);
        //}
    }

    if(line == 0 && print == 1){
        print = 0;
        //printf("Print screen\n");
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
    } else if(line !=0){
        print = 1;
    }
    //SDL_Delay( 4000 );
}

int get_key_press(){
    int ret = 0;
    int numkeys = 0;

    SDL_PumpEvents();

    const uint8_t *array = SDL_GetKeyboardState(&numkeys);
    
    if( array[SDL_SCANCODE_ESCAPE] == 1 )
    {
        ret = -1;
    }

    if( array[SDL_SCANCODE_UP] == 1 )
    {
        ret |= KEY_UP;
    }
    if( array[SDL_SCANCODE_DOWN] == 1 )
    {
        ret |= KEY_DOWN;
    }
    if( array[SDL_SCANCODE_LEFT] == 1 )
    {
        ret |= KEY_LEFT;
    }
    if( array[SDL_SCANCODE_RIGHT] == 1 )
    {
        ret |= KEY_RIGHT;
    }
    if( array[SDL_SCANCODE_A] == 1 )
    {
        ret |= KEY_A;
    }
    if( array[SDL_SCANCODE_S] == 1 )
    {
        ret |= KEY_B;
    }
    if( array[SDL_SCANCODE_RETURN] == 1 )
    {
        ret |= KEY_START;
    }
    if( array[SDL_SCANCODE_SPACE] == 1 )
    {
        ret |= KEY_SELECT;
    }
    if( array[SDL_SCANCODE_BACKSPACE] == 1 )
    {
        ret |= KEY_DEBUG;
    }

    //printf("keypress: %02x\n", ret);
    return ret;
}
