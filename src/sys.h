#ifndef sys_h
#define sys_h

#include "quakedef.h"

bool sys_sendkeyevents(void);
const char *sys_va(const char *format, ...);

// Helper macros
#define SDL_EVENT_EXISTS(sdl_event, e_type) \
    SDL_PeepEvents(sdl_event, 1, SDL_GETEVENT, e_type, e_type)

#define COM_FREE(p)     \
    do {                \
        if(p) {         \
            free(p);    \
            p = NULL;   \
        }               \
    } while(0)

#define COM_FREE_PAK(p)         \
    do {                        \
        COM_FREE(p->pak_files); \
        COM_FREE(p);            \
    } while(0)

#endif // sys_h
