#ifndef sdl_helper_h
#define sdl_helper_h

// Helper macros
#define sdl_pump_events() SDL_PumpEvents()
#define sdl_flush_events() SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT)
#define sdl_event_exists(sdl_event, e_type) SDL_PeepEvents(sdl_event, 1, SDL_GETEVENT, e_type, e_type)

i32 sdl_init(const char *title, u32 width, u32 height, SDL_Window **win, SDL_Renderer **ren);

#endif // sdl_helper_h
