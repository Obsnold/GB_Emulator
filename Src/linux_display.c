#include <SDL2/SDL.h>
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

void update_screen(uint8_t line){
    for(int x = 0; x < GB_SCREEN_WIDTH; x++){
        //for(int i = 0; i < 2; i++){
            int pixel = (line * GB_SCREEN_WIDTH * RGB_CHANNELS)+(x*RGB_CHANNELS);
            pixel_screen[pixel] = ((gb_display[x][line] & 0xFF0000) >> 0x10);
            pixel_screen[pixel+1] = ((gb_display[x][line] & 0xFF00) >> 0x08);
            pixel_screen[pixel+2] = (gb_display[x][line] & 0xFF);
        //}
    }

    if(line == 0){
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
    //SDL_Delay( 4000 );
}

uint32_t get_key_press(){
    uint32_t ret = 0;
    if (SDL_PollEvent( &e ) !=0){
        //User requests quit
        if( e.type == SDL_QUIT )
        {
            ret = 1;
        }
        //User presses a key
        else if( e.type == SDL_KEYDOWN )
        {
            //Select surfaces based on key press
            switch( e.key.keysym.sym )
            {
                case SDLK_UP:
                ret = 2;
                break;

                case SDLK_DOWN:
                ret = 3;
                break;

                case SDLK_LEFT:
                ret = 4;
                break;

                case SDLK_RIGHT:
                ret = 5;
                break;

                default:
                ret = 0;
                break;
            }
        }
    }
    return ret;
}
