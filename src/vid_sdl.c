#include "vid.h"

void
vid_setmode(i32 width, i32 height) {

}

void
vid_init(void) {

}

void
vid_shutdown(void) {

}

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

i32
sdl_convert_pixels(SDL_Surface *s, u32 format, void *pixels, i32 pitch) {
    return SDL_ConvertPixels(s->w, s->h, s->format->format, s->pixels, s->pitch, format, pixels, pitch);
}

void
sdl_toggle_fullscreen(SDL_Window *w) {
    bool is_fullscreen = SDL_GetWindowFlags(w) & SDL_WINDOW_FULLSCREEN;
    is_fullscreen ? SDL_SetWindowFullscreen(w, 0) : SDL_SetWindowFullscreen(w, SDL_WINDOW_FULLSCREEN);
}
