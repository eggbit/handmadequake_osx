#include "host.h"

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
    vid_init();

    return true;
}

bool
host_frame(double timestep) {
    if(!host_filter_time(timestep)) return true;

    if(!sys_sendkeyevents()) return false;
    if(!vid_update()) return false;
    //
    // static u8 frame = 0;
    // if(frame >= 72) frame = 0;
    //
    // printf("Frame: %d\n", ++frame);
    return true;
}

void
host_shutdown() {
    vid_shutdown();
    SDL_Quit();
}
