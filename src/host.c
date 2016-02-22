#include "host.h"

bool
host_filter_time(double time) {
    static double real_time, old_real_time = 0.0;

    real_time += time;
    double delta = real_time - old_real_time; // == host_frame_time
    old_real_time = real_time;

    return (delta < 1.0 / 72.0) ? false : true;
}

void
host_init() {
    // vid_init();
}

void
host_frame(double timestep) {
    if(!host_filter_time(timestep)) return;

    // TODO: Functions
    // sys_sendkeyevents();
    // vid_update();

    // static u8 frame = 0;
    // if(frame >= 72) frame = 0;
    //
    // printf("Frame: %d\n", ++frame);
}

void
host_shutdown() {
    // vid_shutdown();
}
