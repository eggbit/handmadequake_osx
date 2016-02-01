#include "host.h"

void
host_init() {
}

void
host_frame(double timestep) {
    static u8 frame = 0;
    if(frame >= 60) frame = 0;

    printf("Frame: %d\n", ++frame);
}

void
host_shutdown() {
}
