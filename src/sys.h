#ifndef sys_h
#define sys_h

#include "quakedef.h"

bool sys_sendkeyevents(void);
void com_free(void *mem);
const char *sys_va(const char *format, ...);

// Helper macros
#define SDL_EVENT_EXISTS(sdl_event, e_type) \
    SDL_PeepEvents(sdl_event, 1, SDL_GETEVENT, e_type, e_type)

#endif // sys_h
