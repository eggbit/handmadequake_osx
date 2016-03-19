#include "vid.h"

#define MAX_MODES 30

struct lmpdata_t {
    i32 width;
    i32 height;
    void *data;
};

struct vmode_t {
    i32 width;
    i32 height;
    bool fullscreen;
};

// NOTE: Fullscreen modes will be added to this array via vid_init_fullscreen_mode function.
static struct vmode_t lk_mode_list[MAX_MODES] = {
    { 320, 240, false },
    { 640, 480, false },
    { 800, 600, false },
    { 1024, 768, false }
};

static struct lmpdata_t lk_discdata = { 0 };
static struct lmpdata_t lk_pausedata = { 0 };

static SDL_Window *lk_window = NULL;
static SDL_Renderer *lk_renderer = NULL;
static SDL_Surface *lk_worksurface = NULL;   // NOTE: Holds pixel data we'll directly modify.
static SDL_Texture *lk_texture = NULL;    // NOTE: Holds the final pixel data that'll be displayed.
static SDL_Palette *lk_palette = NULL;

static void
lk_readlmp(struct lmpdata_t *lmp, const char *path, bool palette) {
    i32 bytes_read = 0, int_offset = 0;
    u32 *data = com_find_file(path, &bytes_read);

    if(bytes_read) {
        if(palette) goto load;

        lmp->width = data[0];
        lmp->height = data[1];
        int_offset = 2;
        bytes_read -= 8;
    }
    else return;

load:
    lmp->data = malloc(bytes_read);
    memcpy(lmp->data, data + int_offset, bytes_read);

    free(data);
}

static void
lk_loadimage(struct lmpdata_t *lmp, const char *path) {
    lk_readlmp(lmp, path, false);
}

static void
lk_loadpalette(const char *palette_path) {
    struct lmpdata_t palette = {0};
    lk_readlmp(&palette, palette_path, true);

    if(palette.data) {
        lk_palette = SDL_AllocPalette(256);

        SDL_Color c[255];
        u8 *p_data = palette.data;

        for(u8 i = 0; i < 255; i++) {
            c[i].r = *p_data++;
            c[i].g = *p_data++;
            c[i].b = *p_data++;
        }

        SDL_SetPaletteColors(lk_palette, c, 0, 256);
        com_free(palette.data);
    }
}

// NOTE: DRAW_LMP and DRAW_RECT are based on this function
static void
s_drawraw(i32 x, i32 y, i32 width, i32 height, u32 color, struct lmpdata_t *lmp) {
    u32 *dest = lk_worksurface->pixels;
    u8 *source = lmp ? lmp->data : NULL;

    // NOTE: Bounds checking
    if(height && width) {
        if((x + width) > lk_worksurface->w) width = lk_worksurface->w - x;
        if((y + height) > lk_worksurface->h) height = lk_worksurface->h - y;
    }
    else {
        height = lmp->height;
        width = lmp->width;
    }

    // NOTE: Starting pixel position.
    dest += (y * lk_worksurface->w + x);

    for(i32 y = 0; y < height; y++) {
        for(i32 x = 0; x < width; x++) {
            if(source) {
                color = (lk_palette->colors[*source].r << 16) | (lk_palette->colors[*source].g << 8) | lk_palette->colors[*source].b;
                source++;
            }
            dest[y * lk_worksurface->w + x] = color;
        }
    }
}

static void
lk_drawlmp(i32 x, i32 y, struct lmpdata_t *lmp) {
    s_drawraw(x, y, 0, 0, 0, lmp);
}

static void
lk_drawrect(i32 x, i32 y, i32 w, i32 h, i32 color) {
    s_drawraw(x, y, w, h, color, NULL);
}

void
vid_init_fullscreen_mode(void) {
    for(u32 i = 0; i < MAX_MODES; i++) {
        if(!lk_mode_list[i].width) { // NOTE: Start adding fullscreen modes when we hit first empty element in sl_mode_list.
            SDL_DisplayMode d;
            i32 num_modes = SDL_GetNumDisplayModes(0);

            // TODO: Multidisplay support.
            for(i32 x = i; x < num_modes; x++) {
                SDL_GetDisplayMode(0, x, &d);

                if(d.refresh_rate == 60) {
                    // printf("width: %d - height: %d - vsync: %d\n", d.w, d.h, d.refresh_rate);
                    lk_mode_list[x].fullscreen = true;
                    lk_mode_list[x].width = d.w;
                    lk_mode_list[x].height = d.h;
                }
            }
            break;
        }
    }
}

bool
vid_init(void) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
    vid_init_fullscreen_mode();

    // NOTE: Print out all modes
    for(u32 i = 0; i < MAX_MODES; i++)
        printf("width: %d - height: %d - fullscreen: %s\n", lk_mode_list[i].width, lk_mode_list[i].height, lk_mode_list[i].fullscreen ? "true" : "false");

    return vid_setmode("Handmade Quake OSX", 1);
}

bool
vid_setmode(const char *title, i32 mode) {
    if(lk_window) vid_shutdown();

    i32 width = lk_mode_list[mode].width;
    i32 height = lk_mode_list[mode].height;
    i32 fullscreen_flag = (lk_mode_list[mode].fullscreen) ? SDL_WINDOW_FULLSCREEN : 0;

    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
    if(SDL_CreateWindowAndRenderer(width, height, fullscreen_flag, &lk_window, &lk_renderer) < 0) return false;
    SDL_SetWindowTitle(lk_window, title);

    // NOTE: Maintain 4:3 aspect ratio while fullscreen
    if(fullscreen_flag) SDL_RenderSetLogicalSize(lk_renderer, 320, 240);

    // NOTE: Create surfaces and output texture.
    lk_worksurface = SDL_CreateRGBSurface(0, 320, 240, 32, 0, 0, 0, 0);
    lk_texture = SDL_CreateTexture(lk_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, 320, 240);

    // TODO: Error checking.
    lk_loadpalette("gfx/palette.lmp");
    lk_loadimage(&lk_discdata, "gfx/qplaque.lmp");
    lk_loadimage(&lk_pausedata, "gfx/pause.lmp");

    return true;
}

bool
vid_draw(void) {
    lk_drawrect(0, 0, lk_worksurface->w, lk_worksurface->h, SDL_MapRGB(lk_worksurface->format, 100, 100, 0));
    lk_drawlmp(20, 20, &lk_pausedata);
    lk_drawlmp(20, 60, &lk_discdata);

    return true;
}

bool
vid_update(void) {
    // NOTE: Method of updating 8-bit palette without calling SDL_CreateTextureFromSurface every frame.
    // NOTE: http://sandervanderburg.blogspot.ca/2014/05/rendering-8-bit-palettized-surfaces-in.html

    void *output_buffer = NULL;
    i32 pitch;

    // NOTE: Copy s_worksurface pixels to s_texture's pixel buffer.
    SDL_LockTexture(lk_texture, NULL, &output_buffer, &pitch);
    memcpy(output_buffer, lk_worksurface->pixels, lk_worksurface->pitch * lk_worksurface->h);
    SDL_UnlockTexture(lk_texture);

    // NOTE: Output the texture to the screen. SDL_RenderCopy will scale the texture up.
    SDL_RenderClear(lk_renderer);
    SDL_RenderCopy(lk_renderer, lk_texture, NULL, NULL);
    SDL_RenderPresent(lk_renderer);

    return true;
}

void
vid_shutdown(void) {
    com_free(lk_discdata.data);
    com_free(lk_pausedata.data);
    SDL_FreePalette(lk_palette);
    SDL_FreeSurface(lk_worksurface);
    SDL_DestroyTexture(lk_texture);
    SDL_DestroyRenderer(lk_renderer);
    SDL_DestroyWindow(lk_window);
}

void
vid_toggle_fullscreen(void) {
    bool is_fullscreen = SDL_GetWindowFlags(lk_window) & SDL_WINDOW_FULLSCREEN;
    is_fullscreen ? SDL_SetWindowFullscreen(lk_window, 0) : SDL_SetWindowFullscreen(lk_window, SDL_WINDOW_FULLSCREEN);
}
