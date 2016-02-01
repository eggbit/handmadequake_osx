#ifndef host_h
#define host_h

#include "quakedef.h"

// IDEA: Create game structure to pass?
void host_init();
void host_frame(double time);
void host_shutdown();

#endif // host_h
