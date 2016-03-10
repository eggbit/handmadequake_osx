#include "vid.h"

#define MAX_MODES 30

struct lmpdata_t {
    i32 width;
    i32 height;
    void *data;
};

struct vmode_t {
    bool fullscreen;
    i32 width;
    i32 height;
};

// NOTE: Fullscreen modes will be added to this array via vid_init_fullscreen_mode function.
static struct vmode_t sl_mode_list[MAX_MODES] = {
    { false, 320, 240 },
    { false, 640, 480 },
    { false, 800, 600 },
    { false, 1024, 768 }
};

static struct lmpdata_t s_discdata = { 0 };
static struct lmpdata_t s_pausedata = { 0 };

static SDL_Window *s_window = NULL;
static SDL_Renderer *s_renderer = NULL;
static SDL_Surface *s_worksurface = NULL;   // NOTE: Holds pixel data we'll directly modify.
static SDL_Texture *s_outtexture = NULL; // NOTE: Holds the final pixel data that'll be displayed.
static SDL_Palette *s_palette = NULL;

static size_t
s_readlmp(struct lmpdata_t *lmp, const char *file_path) {
    i32 file, bytes_read;

    file = sys_fopen_rb(file_path, &bytes_read);
    if(file < 0) goto escape;

    sys_fread(file, &lmp->width, sizeof(lmp->width));
    sys_fread(file, &lmp->height, sizeof(lmp->height));

    printf("width = %d, height = %d\n", lmp->width, lmp->height);

    // NOTE: If width or height are unreasonable, assume it's a 256-color palette.
    if(lmp->width > 1024 || lmp->height > 1024) {
        sys_frewind(file);
        lmp->data = malloc(256 * 3);
        bytes_read = sys_fread(file, lmp->data, 256 * 3);
    }
    else {
        lmp->data = malloc(lmp->width * lmp->height);
        bytes_read = sys_fread(file, lmp->data, lmp->width * lmp->height);
    }

escape:
    sys_fclose(file);
    return bytes_read;
}

static size_t
s_loadpalette(const char *palette_path) {
    struct lmpdata_t palette = {0};
    size_t bytes_read = s_readlmp(&palette, palette_path);

    if(!bytes_read) goto escape;

    s_palette = SDL_AllocPalette(256);

    SDL_Color c[255];
    u8 *p_data = palette.data;

    for(u8 i = 0; i < 255; i++) {
        c[i].r = *p_data++;
        c[i].g = *p_data++;
        c[i].b = *p_data++;
    }

    SDL_SetPaletteColors(s_palette, c, 0, 256);

escape:
    COM_FREE(palette.data);
    return bytes_read;
}

// NOTE: DRAW_LMP and DRAW_RECT are based on this function
static void
s_drawraw(i32 x, i32 y, i32 width, i32 height, u32 color, struct lmpdata_t *lmp) {
    u8 bpp = s_worksurface->format->BytesPerPixel;
    u8 *dest = s_worksurface->pixels;
    u8 *source = lmp ? lmp->data : NULL;

    // NOTE: Bounds checking
    if(height && width) {
        if((x + width) > s_worksurface->w) width = s_worksurface->w - x;
        if((y + height) > s_worksurface->h) height = s_worksurface->h - y;
    }
    else {
        height = lmp->height;
        width = lmp->width;
    }

    // NOTE: First pixel position.
    dest += (s_worksurface->w * bpp * y) + (x * bpp);

    u32 *buffer_walker = (u32 *)dest;

    for(i32 y = 0; y < height; y++) {
        for(i32 x = 0; x < width; x++) {
            if(color) *buffer_walker = color;
            if(source) {
                SDL_Color c = s_palette->colors[*source];
                *buffer_walker = (c.r << 16) | (c.g << 8) | c.b;
                source++;
            }

            buffer_walker++;
        }

        dest += s_worksurface->w * bpp;
        buffer_walker = (u32 *)dest;
    }
}

static void
s_drawlmp(i32 x, i32 y, struct lmpdata_t *lmp) {
    s_drawraw(x, y, 0, 0, 0, lmp);
}

static void
s_drawrect(i32 x, i32 y, i32 w, i32 h, i32 color) {
    s_drawraw(x, y, w, h, color, NULL);
}

void
vid_init_fullscreen_mode(void) {
    for(u32 i = 0; i < MAX_MODES; i++) {
        if(!sl_mode_list[i].width) { // NOTE: Start adding fullscreen modes when we hit first empty element in sl_mode_list.
            SDL_DisplayMode d;
            i32 num_modes = SDL_GetNumDisplayModes(0);

            // TODO: Multidisplay support.
            for(i32 x = i; x < num_modes; x++) {
                SDL_GetDisplayMode(0, x, &d);

                if(d.refresh_rate == 60) {
                    // printf("width: %d - height: %d - vsync: %d\n", d.w, d.h, d.refresh_rate);
                    sl_mode_list[x].fullscreen = true;
                    sl_mode_list[x].width = d.w;
                    sl_mode_list[x].height = d.h;
                }
            }
            break;
        }
    }
}

bool
vid_init(void) {
    vid_init_fullscreen_mode();

    // NOTE: Print out all modes
    for(u32 i = 0; i < MAX_MODES; i++)
        printf("width: %d - height: %d - fullscreen: %s\n", sl_mode_list[i].width, sl_mode_list[i].height, sl_mode_list[i].fullscreen ? "true" : "false");

    return vid_setmode("Handmade Quake OSX", 1);
}

bool
vid_setmode(const char *title, i32 mode) {
    if(s_window) vid_shutdown();

    i32 width = sl_mode_list[mode].width;
    i32 height = sl_mode_list[mode].height;
    i32 fullscreen_flag = (sl_mode_list[mode].fullscreen) ? SDL_WINDOW_FULLSCREEN : 0;

    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
    if(SDL_CreateWindowAndRenderer(width, height, fullscreen_flag, &s_window, &s_renderer) < 0) return false;
    SDL_SetWindowTitle(s_window, title);

    // NOTE: Maintain 4:3 aspect ratio while fullscreen
    if(fullscreen_flag) SDL_RenderSetLogicalSize(s_renderer, 320, 240);

    // NOTE: Create surfaces and output texture.
    s_worksurface = SDL_CreateRGBSurface(0, 320, 240, 32, 0, 0, 0, 0);
    s_outtexture = SDL_CreateTexture(s_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, 320, 240);

    // TODO: Error checking.
    s_loadpalette("data/palette.lmp");
    s_readlmp(&s_discdata, "data/DISC.lmp");
    s_readlmp(&s_pausedata, "data/pause.lmp");

    return true;
}

bool
vid_draw(void) {
    s_drawrect(0, 0, s_worksurface->w, s_worksurface->h, SDL_MapRGB(s_worksurface->format, 100, 100, 0));
    s_drawlmp(20, 20, &s_pausedata);
    s_drawlmp(20, 60, &s_discdata);

    return true;
}

bool
vid_update(void) {
    // NOTE: Method of updating 8-bit palette without calling SDL_CreateTextureFromSurface every frame.
    // NOTE: http://sandervanderburg.blogspot.ca/2014/05/rendering-8-bit-palettized-surfaces-in.html

    void *output_buffer = NULL;
    i32 pitch;

    // NOTE: Copy s_worksurface pixels to s_outtexture's pixel buffer.
    SDL_LockTexture(s_outtexture, NULL, &output_buffer, &pitch);
    memcpy(output_buffer, s_worksurface->pixels, s_worksurface->pitch * s_worksurface->h);
    SDL_UnlockTexture(s_outtexture);

    // NOTE: Output the texture to the screen. SDL_RenderCopy will scale the texture up.
    SDL_RenderClear(s_renderer);
    SDL_RenderCopy(s_renderer, s_outtexture, NULL, NULL);
    SDL_RenderPresent(s_renderer);

    return true;
}

void
vid_shutdown(void) {
    COM_FREE(s_discdata.data);
    COM_FREE(s_pausedata.data);
    SDL_FreePalette(s_palette);
    SDL_FreeSurface(s_worksurface);
    SDL_DestroyTexture(s_outtexture);
    SDL_DestroyRenderer(s_renderer);
    SDL_DestroyWindow(s_window);
}

void
vid_toggle_fullscreen(void) {
    bool is_fullscreen = SDL_GetWindowFlags(s_window) & SDL_WINDOW_FULLSCREEN;
    is_fullscreen ? SDL_SetWindowFullscreen(s_window, 0) : SDL_SetWindowFullscreen(s_window, SDL_WINDOW_FULLSCREEN);
}
