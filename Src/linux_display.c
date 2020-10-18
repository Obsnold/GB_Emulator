#include <SDL2/SDL.h>
#include "gb_ppu.h"

SDL_Window* window = NULL;
SDL_Renderer *renderer;
int screen_size = 3;

void init_screen(){
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
    } else {
        SDL_CreateWindowAndRenderer(GB_SCREEN_WIDTH*screen_size, GB_SCREEN_HEIGHT*screen_size, 0, &window, &renderer);
        //window = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, GB_SCREEN_WIDTH, GB_SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
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

void update_screen(){
    for(int x = 0; x < GB_SCREEN_WIDTH; x++){
        for(int y = 0; y < GB_SCREEN_WIDTH; y++){
            SDL_SetRenderDrawColor(renderer, ((gb_display[x][y] & 0xFF0000) >> 0x10), ((gb_display[x][y] & 0xFF00) >> 0x08), (gb_display[x][y] & 0xFF), 0xFF);
            SDL_RenderDrawPoint(renderer, x*screen_size, y*screen_size);
            SDL_RenderDrawPoint(renderer, x*screen_size+1, y*screen_size);
            SDL_RenderDrawPoint(renderer, x*screen_size, y*screen_size+1);
            SDL_RenderDrawPoint(renderer, x*screen_size+1, y*screen_size+1);
        }
    }
    SDL_RenderPresent(renderer);
    SDL_Delay( 4000 );
}
