#ifndef sys_h
#define sys_h

#include "quakedef.h"

bool sys_sendkeyevents(void);

// Helper macros
#define sdl_event_exists(sdl_event, e_type) \
    SDL_PeepEvents(sdl_event, 1, SDL_GETEVENT, e_type, e_type)

#endif // sys_h
