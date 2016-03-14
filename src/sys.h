#ifndef sys_h
#define sys_h

#include "quakedef.h"

bool sys_sendkeyevents(void);
const char *sys_va(const char *format, ...);

// Helper macros
#define print_dec(d) \
    printf("%s: %d\n", #d, d)

#define sdl_event_exists(sdl_event, e_type) \
    SDL_PeepEvents(sdl_event, 1, SDL_GETEVENT, e_type, e_type)

#define com_free(p)     \
    do {                \
        if(p) {         \
            free(p);    \
            p = NULL;   \
        }               \
    } while(0)

#endif // sys_h
