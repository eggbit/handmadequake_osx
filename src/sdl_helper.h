#ifndef sdl_helper_h
#define sdl_helper_h

#include "quakedef.h"

// Helper macros
#define sdl_pump_events() \
    SDL_PumpEvents()

#define sdl_flush_events() \
    SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT)

#define sdl_event_exists(sdl_event, e_type) \
    SDL_PeepEvents(sdl_event, 1, SDL_GETEVENT, e_type, e_type)

#define sdl_display_buffer(renderer, buffer, texture, pitch) \
    do { \
        SDL_UpdateTexture(texture, NULL, buffer, pitch); \
        SDL_RenderClear(renderer); \
        SDL_RenderCopy(renderer, texture, NULL, NULL); \
        SDL_RenderPresent(renderer); \
    } while(0)

i32 sdl_init(const char *title, u32 width, u32 height, SDL_Window **win, SDL_Renderer **ren);
void sdl_toggle_fullscreen(SDL_Window *w);

#endif // sdl_helper_h
