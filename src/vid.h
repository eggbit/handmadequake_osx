#ifndef vid_h
#define vid_h

#include "quakedef.h"

// Helper macros
#define sdl_pump_events() \
    SDL_PumpEvents()

#define sdl_flush_events() \
    SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT)

#define sdl_event_exists(sdl_event, e_type) \
    SDL_PeepEvents(sdl_event, 1, SDL_GETEVENT, e_type, e_type)

bool vid_init(void);
void vid_shutdown(void);
void vid_update(void);
bool vid_setmode(const char *title, i32 width, i32 height);

// NOTE: SDL helper functions.
i32 sdl_convert_pixels(SDL_Surface *s, u32 format, void *data, i32 pitch);
void sdl_toggle_fullscreen(SDL_Window *w);

#endif // vid_h
