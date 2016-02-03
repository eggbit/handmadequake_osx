#include "host.h"

static double real_time, old_real_time, host_frame_time = 0.0;

bool
host_filter_time(double time) {
    real_time += time;

    if(real_time - old_real_time < 1.0 / 72.0) return false;

    host_frame_time = real_time - old_real_time;
    old_real_time = real_time;

    return true;
}

void
host_init() {
}

void
host_frame(double timestep) {
    if(!host_filter_time(timestep)) return;

    static u8 frame = 0;
    if(frame >= 72) frame = 0;

    printf("Frame: %d\n", ++frame);
}

void
host_shutdown() {
}
