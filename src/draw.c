#include "draw.h"

static SDL_Palette *lk_palette = NULL;

void
draw_load_palette(void) {
    u8 *data = pak_get("gfx/palette.lmp", NULL);

    if(data) {
        lk_palette = SDL_AllocPalette(256);

        SDL_Color c[256];
        u8 *p_data = data;

        for(u8 i = 0; i < 255; i++) {
            c[i].r = *p_data++;
            c[i].g = *p_data++;
            c[i].b = *p_data++;
        }

        SDL_SetPaletteColors(lk_palette, c, 0, 256);
        com_free(data);
    }
}

void
draw_pic(SDL_Surface *s, i32 x, i32 y, u32 *data) {
    u32 *dest = s->pixels;
    u8 *source = (u8 *)data;

    i32 width = data[0];
    i32 height = data[1];

    dest += (y * s->w + x); // NOTE: Starting pixel position.
    source += 8;            // NOTE: Starting data position (skip width and height).

    for(i32 y = 0; y < height; y++) {
        for(i32 x = 0; x < width; source++, x++) {
            if(*source != 0xff) {
                SDL_Color c = lk_palette->colors[*source];
                dest[y * s->w + x] = (c.r << 16) | (c.g << 8) | c.b;
            }
        }
    }
}

void
draw_rect(SDL_Surface *s, i32 x, i32 y, i32 width, i32 height, i32 color) {
    u32 *dest = s->pixels;
    dest += (y * s->w + x); // NOTE: Starting pixel position.

    // NOTE: Bounds checking
    if((x + width) > s->w) width = s->w - x;
    if((y + height) > s->h) height = s->h - y;

    for(i32 y = 0; y < height; y++) {
        for(i32 x = 0; x < width; x++) {
            dest[y * s->w + x] = color;
        }
    }
}

void
draw_free_palette(void) {
    SDL_FreePalette(lk_palette);
}
