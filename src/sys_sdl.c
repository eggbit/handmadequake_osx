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
