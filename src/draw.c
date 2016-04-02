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
draw_load_image(struct lmpdata_t *lmp, const char *path) {
    u32 bytes_read = 0;
    u32 *data = pak_get(path, &bytes_read);

    if(bytes_read) {
        lmp->width = data[0];
        lmp->height = data[1];
        lmp->data = malloc(bytes_read);

        memcpy(lmp->data, data + 2, bytes_read - 8);
        free(data);
    }
}

void
draw_pic(SDL_Surface *s, i32 x, i32 y, struct lmpdata_t *lmp) {
    u32 *dest = s->pixels;
    dest += (y * s->w + x); // NOTE: Starting pixel position.

    u8 *source = lmp->data;

    for(i32 y = 0; y < lmp->height; y++) {
        for(i32 x = 0; x < lmp->width; source++, x++) {
            if(*source != 0xff) {
                u32 color = (lk_palette->colors[*source].r << 16) | (lk_palette->colors[*source].g << 8) | lk_palette->colors[*source].b;
                dest[y * s->w + x] = color;
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
