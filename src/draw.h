#ifndef draw_h
#define draw_h

#include "quakedef.h"

struct lmpdata_t {
    i32 width;
    i32 height;
    void *data;
};

void draw_rect(SDL_Surface *s, i32 x, i32 y, i32 w, i32 h, i32 color);
void draw_pic(SDL_Surface *s, i32 x, i32 y, struct lmpdata_t *lmp);
void draw_load_palette(void);
void draw_load_image(struct lmpdata_t *lmp, const char *path);
void draw_free_palette(void);

#endif // draw_h
