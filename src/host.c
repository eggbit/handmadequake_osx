#include "host.h"

i32
frame_count(i32 target_fps) {
    static u8 frame = 0;
    if(frame >= target_fps) frame = 0;
    return ++frame;
}

bool
host_filter_time(double time) {
    static double real_time, old_real_time, delta = 0.0;

    real_time += time;
    delta += real_time - old_real_time; // == host_frame_time
    old_real_time = real_time;

    // printf("rt: %.9f - ort: %.9f - delta: %.9f\n", real_time, old_real_time, delta);

    if(delta < 1.0 / 72.0) return false;
    else {
        delta = 0.0;
        return true;
    }
}

bool
host_init() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
    return vid_init();
}

bool
host_frame(double timestep) {
    if(host_filter_time(timestep)) {
        // printf("Frame: %d\n", frame_count(72));
        if(!sys_sendkeyevents() || !vid_draw() || !vid_update()) return false;
    }

    return true;
}

void
host_shutdown() {
    vid_shutdown();
    SDL_Quit();
}
