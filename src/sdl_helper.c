#include "sdl_helper.h"

// Baseline initialization.
i32
sdl_init(const char *title, u32 width, u32 height, SDL_Window **win, SDL_Renderer **ren) {
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) return 0;

    *win = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0);
    if(!win) return 0;

    *ren = SDL_CreateRenderer(*win, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if(!ren) return 0;

    return 1;
}
