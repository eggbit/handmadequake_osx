// TODO: SDL timing while windowed is very, very off.
// TODO: Proper error checking everywhere.
// NOTE: Since SDL needs to convert an 8-bit surface to 32-bits to get anything to display anyway, all 32-bit pixel code is redundant.

#include "sys.h"

struct Timer {
    double seconds_per_tick;
    double oldtime;
    double delta;
    double time_passed;
    u64 time_count;
};

void
sys_timerinit(struct Timer *t) {
    t->seconds_per_tick = 1.0 / (double)SDL_GetPerformanceFrequency();
    t->time_count = SDL_GetPerformanceCounter();
}

void
sys_timerupdate(struct Timer *t) {
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

        if(key == SDLK_z) vid_setmode("Handmade Que?", 640, 480);
        if(key == SDLK_x) vid_setmode("Handmade Que?", 800, 600);
        if(key == SDLK_c) vid_setmode("Handmade Que?", 1024, 768);
        if(key == SDLK_t) vid_toggle_fullscreen();
    }

    if(sdl_event_exists(&event, SDL_QUIT)) return false;

    sdl_flush_events();
    return true;
}

int
main(int argc, const char *argv[]) {
    if(!host_init()) goto exit;

    struct Timer timer;
    sys_timerinit(&timer);

    for(;;) {
        sys_timerupdate(&timer);
        if(!host_frame(timer.delta)) goto exit;
    }

exit:;
    const char *error = SDL_GetError();
    if(error) printf("ERROR: %s\n", error);

    host_shutdown();
    return 0;
}
