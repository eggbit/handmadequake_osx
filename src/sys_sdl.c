// TODO: SDL timing while windowed is very, very off.
// TODO: Proper error checking everywhere.

#include "sys.h"

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
#define VALID_HANDLE(h) (h >= 0 || h < MAX_HANDLES || s_fhandles[h])

static FILE *s_fhandles[MAX_HANDLES] = { 0 };

static i32
s_gethandle(void) {
    for(i32 i = 0; i < MAX_HANDLES; i++) {
        if(!s_fhandles[i]) return i;
    }

    return -1;
}

static i32
s_flength(FILE *f) {
    i32 length;

    fseek(f, 0, SEEK_END);
    length = (i32)ftell(f);
    rewind(f);

    return length;
}

static i32
s_fopen(const char *path, const char *mode, i32 *size) {
    i32 handle_index = s_gethandle();
    if(handle_index < 0) goto error;

    FILE *f = fopen(path, mode);
    if(!f) goto error;

    s_fhandles[handle_index] = f;

    if(size) *size = s_flength(f);
    goto escape;

error:
    handle_index = -1;

escape:
    return handle_index;
}

i32
sys_fopen_rb(const char *path, i32 *size) {
    return s_fopen(path, "rb", size);
}

i32
sys_fopen_wb(const char *path) {
    return s_fopen(path, "wb", NULL);
}

static i32
s_freadwrite(i32 handle, void *buffer, i32 count, bool read_mode) {
    if(!VALID_HANDLE(handle) || !buffer) return -1;

    return read_mode ? (i32)fread(buffer, 1, count, s_fhandles[handle]) : (i32)fwrite(buffer, 1, count, s_fhandles[handle]);
}

i32
sys_fread(i32 handle, void *dest, i32 count) {
    return s_freadwrite(handle, dest, count, true);
}

i32
sys_fwrite(i32 handle, void *source, i32 count) {
    return s_freadwrite(handle, source, count, false);
}

void
sys_fclose(i32 handle) {
    if(VALID_HANDLE(handle)) {
        fclose(s_fhandles[handle]);
        s_fhandles[handle] = NULL;
    }
}

void
sys_fseek(i32 handle, i32 position) {
    if(VALID_HANDLE(handle)) fseek(s_fhandles[handle], position, SEEK_SET);
}

void
sys_frewind(i32 handle) {
    if(VALID_HANDLE(handle)) rewind(s_fhandles[handle]);
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
    // com_add_game_directory("data");
    //
    // i32 palette_length;
    // void *bytes = com_find_file("gfx/pause.lmp", &palette_length);
    //
    // if(palette_length) {
    //     FILE *output = fopen("data/out_pause.lmp", "wb");
    //     fwrite(bytes, 1, palette_length, output);
    //     fclose(output);
    // }
    //
    // com_free(bytes);
    // com_free_directory();

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
