#ifndef vid_h
#define vid_h

#include "quakedef.h"

bool vid_setmode(const char *title, i32 mode);
bool vid_init(void);
bool vid_update(void);
void vid_shutdown(void);
void vid_toggle_fullscreen(void);

#endif // vid_h
