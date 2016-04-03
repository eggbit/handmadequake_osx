#ifndef draw_h
#define draw_h

#include "quakedef.h"

void draw_rect(SDL_Surface *s, i32 x, i32 y, i32 w, i32 h, i32 color);
void draw_pic(SDL_Surface *s, i32 x, i32 y, u32 *data);
void draw_load_palette(void);
void draw_free_palette(void);

#endif // draw_h
