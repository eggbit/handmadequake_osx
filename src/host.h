#ifndef host_h
#define host_h

#include "quakedef.h"

// IDEA: Create game structure to pass?
bool host_filter_time(double time);
bool host_init();
bool host_frame(double time);
void host_shutdown();

#endif // host_h
