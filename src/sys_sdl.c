// TODO: SDL timing while windowed is very, very off.
// TODO: Proper error checking everywhere.

#include "sys.h"

#define sdl_event_exists(sdl_event, e_type) \
    SDL_PeepEvents(sdl_event, 1, SDL_GETEVENT, e_type, e_type)

struct timer_t {
    double seconds_per_tick;
    double oldtime;
    double delta;
    double time_passed;
    u64 time_count;
};

void
sys_timerinit(struct timer_t *t) {
    t->seconds_per_tick = 1.0 / (double)SDL_GetPerformanceFrequency();
    t->time_count = SDL_GetPerformanceCounter();
}

void
sys_timerupdate(struct timer_t *t) {
    u64 counter = SDL_GetPerformanceCounter();
    u64 interval = counter - t->time_count;

    t->time_count = counter;
    t->time_passed += (double)interval * t->seconds_per_tick;
    t->delta = t->time_passed - t->oldtime;
    t->oldtime = t->time_passed;
}

bool
sys_sendkeyevents() {
    SDL_Event event;
    SDL_PumpEvents();

    if(sdl_event_exists(&event, SDL_KEYDOWN)) {
        SDL_Keycode key = event.key.keysym.sym;

        if(key == SDLK_z) vid_setmode("Handmade Que?", 1);
        if(key == SDLK_x) vid_setmode("Handmade Que?", 2);
        if(key == SDLK_c) vid_setmode("Handmade Que?", 3);
        if(key == SDLK_p) vid_setmode("Handmade Fullscreen", 6);
        if(key == SDLK_t) vid_toggle_fullscreen();
    }

    if(sdl_event_exists(&event, SDL_QUIT)) return false;

    SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
    return true;
}

#define MAX_HANDLES 10
#define VALID_HANDLE(h) (h >= 0 || h < MAX_HANDLES || lk_fhandles[h])

static FILE *lk_fhandles[MAX_HANDLES] = { 0 };

static i32
lk_gethandle(void) {
    for(i32 i = 0; i < MAX_HANDLES; i++) {
        if(!lk_fhandles[i]) return i;
    }

    return -1;  // NOTE: This is just to quiet the compiler.
}

static i32
lk_flength(FILE *f) {
    i32 length;

    fseek(f, 0, SEEK_END);
    length = (i32)ftell(f);
    rewind(f);

    return length;
}

static i32
lk_fopen(const char *path, const char *mode, i32 *size) {
    i32 handle_index = lk_gethandle();
    FILE *f = fopen(path, mode);

    if(!f) {
        handle_index = -1;
        goto escape;
    }

    lk_fhandles[handle_index] = f;
    if(size) *size = lk_flength(f);

escape:
    return handle_index;
}

i32
sys_fopen_rb(const char *path, i32 *size) {
    return lk_fopen(path, "rb", size);
}

i32
sys_fopen_wb(const char *path) {
    return lk_fopen(path, "wb", NULL);
}

static i32
lk_freadwrite(i32 handle, void *buffer, i32 count, bool read_mode) {
    if(!VALID_HANDLE(handle) || !buffer) return -1;

    return read_mode ? (i32)fread(buffer, 1, count, lk_fhandles[handle]) : (i32)fwrite(buffer, 1, count, lk_fhandles[handle]);
}

i32
sys_fread(i32 handle, void *dest, i32 count) {
    return lk_freadwrite(handle, dest, count, true);
}

i32
sys_fwrite(i32 handle, void *source, i32 count) {
    return lk_freadwrite(handle, source, count, false);
}

void
sys_fclose(i32 handle) {
    if(VALID_HANDLE(handle)) {
        fclose(lk_fhandles[handle]);
        lk_fhandles[handle] = NULL;
    }
}

void
sys_fseek(i32 handle, i32 position) {
    if(VALID_HANDLE(handle)) fseek(lk_fhandles[handle], position, SEEK_SET);
}

void
sys_frewind(i32 handle) {
    if(VALID_HANDLE(handle)) rewind(lk_fhandles[handle]);
}

const char*
sys_va(const char *format, ...) {
    static char buffer[1024];
    va_list v;

    va_start(v, format);
    vsprintf(buffer, format, v);
    va_end(v);

    return buffer;
}

int
main(__unused int argc, __unused const char *argv[]) {
    if(!host_init()) goto exit;

    struct timer_t timer;
    sys_timerinit(&timer);

    for(;;) {
        sys_timerupdate(&timer);
        if(!host_frame(timer.delta)) goto exit;
    }

exit:;
    const char *error = SDL_GetError();
    if(error != NULL && error[0] != '\0') printf("ERROR: %s\n", error);

    host_shutdown();

    return 0;
}
