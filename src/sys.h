#ifndef sys_h
#define sys_h

#include "quakedef.h"

bool sys_sendkeyevents(void);
const char *sys_va(const char *format, ...);

i32 sys_fopen_rb(const char *path, i32 *size);
i32 sys_fopen_wb(const char *path);
void sys_fclose(i32 handle);
void sys_fseek(i32 handle, i32 position);
void sys_frewind(i32 handle);
i32 sys_fread(i32 handle, void *dest, i32 count);
i32 sys_fwrite(i32 handle, void *source, i32 count);

// Helper macros
#define print_dec(d) \
    printf("%s: %d\n", #d, d)

#define sdl_event_exists(sdl_event, e_type) \
    SDL_PeepEvents(sdl_event, 1, SDL_GETEVENT, e_type, e_type)

#endif // sys_h
