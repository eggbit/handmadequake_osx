#include "vid.h"

// NOTE: Asset drawing helpers
#define draw_lmp(x, y, lmp_struct) draw_raw(x, y, 0, 0, 0, lmp_struct)
#define draw_rect(x, y, w, h, color) draw_raw(x, y, w, h, color, NULL)

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

static struct lmpdata_t sl_disc_data = { 0 };
static struct lmpdata_t sl_pause_data = { 0 };

static SDL_Window *sl_window = NULL;
static SDL_Renderer *sl_renderer = NULL;
static SDL_Surface *sl_work_surface = NULL;   // NOTE: Holds pixel data we'll directly modify.
static SDL_Surface *sl_output_surface = NULL; // NOTE: Will hold 32-bit conversion of sl_work_surface.
static SDL_Texture *sl_output_texture = NULL; // NOTE: Holds the final pixel data that'll be displayed.

size_t
read_lmp(struct lmpdata_t *lmp, const char *file_path) {
    FILE *file = fopen(file_path, "rb");
    size_t bytes_read = 0;

    if(!file) goto escape;

    fread(&lmp->width, 1, sizeof(lmp->width), file);
    fread(&lmp->height, 1, sizeof(lmp->height), file);

    printf("width = %d, height = %d\n", lmp->width, lmp->height);

    // NOTE: If width or height are unreasonable, assume it's a 256-color palette.
    if(lmp->width > 1024 || lmp->height > 1024) {
        rewind(file);
        lmp->data = malloc(256 * 3);
        bytes_read = fread(lmp->data, 1, 256 * 3, file);
    }
    else {
        lmp->data = malloc(lmp->width * lmp->height);
        bytes_read = fread(lmp->data, 1, lmp->width * lmp->height, file);
    }

escape:
    if(file) fclose(file);
    return bytes_read;
}

size_t
load_palette(const char *palette_path) {
    struct lmpdata_t palette = {0};
    size_t bytes_read = read_lmp(&palette, "data/palette.lmp");

    if(!bytes_read) goto escape;

    SDL_Color c[255];
    u8 *p_data = palette.data;

    for(u8 i = 0; i < 255; i++) {
        c[i].r = *p_data++;
        c[i].g = *p_data++;
        c[i].b = *p_data++;
    }

    if(SDL_SetPaletteColors(sl_work_surface->format->palette, c, 0, 256) < 0) goto escape;

escape:
    free(palette.data);
    return bytes_read;
}

// NOTE: draw_lmp and draw_rect are based on this function
void
draw_raw(i32 x, i32 y, i32 width, i32 height, u32 color, struct lmpdata_t *lmp) {
    u8 bpp = sl_work_surface->format->BytesPerPixel;
    u8 *dest = sl_work_surface->pixels;
    u8 *source = lmp ? lmp->data : NULL;

    // NOTE: Bounds checking
    if(height && width) {
        if((x + width) > sl_work_surface->w) width = sl_work_surface->w - x;
        if((y + height) > sl_work_surface->h) height = sl_work_surface->h - y;
    }
    else {
        height = lmp->height;
        width = lmp->width;
    }

    // NOTE: First pixel position.
    dest += (sl_work_surface->w * bpp * y) + (x * bpp);

    u8 *buffer_walker = dest;

    for(i32 y = 0; y < height; y++) {
        for(i32 x = 0; x < width; x++) {
            if(color) *buffer_walker = color;
            if(source) {
                *buffer_walker = *source;
                source++;
            }

            buffer_walker++;
        }

        dest += sl_work_surface->w * bpp;
        buffer_walker = dest;
    }
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
    if(sl_window) vid_shutdown();

    i32 width = sl_mode_list[mode].width;
    i32 height = sl_mode_list[mode].height;
    i32 fullscreen_flag = (sl_mode_list[mode].fullscreen) ? SDL_WINDOW_FULLSCREEN : 0;

    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
    if(SDL_CreateWindowAndRenderer(width, height, fullscreen_flag, &sl_window, &sl_renderer) < 0) return false;

    // NOTE: Maintain 4:3 aspect ratio while fullscreen
    if(fullscreen_flag) SDL_RenderSetLogicalSize(sl_renderer, 320, 240);

    // NOTE: Create surfaces and output texture.
    sl_work_surface = SDL_CreateRGBSurface(0, 320, 240, 8, 0, 0, 0, 0);
    sl_output_texture = SDL_CreateTexture(sl_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, 320, 240);

    // TODO: Error checking.
    load_palette("data/palette.lmp");
    read_lmp(&sl_disc_data, "data/DISC.lmp");
    read_lmp(&sl_pause_data, "data/pause.lmp");

    return true;
}

bool
vid_draw(void) {
    draw_rect(0, 0, sl_work_surface->w, sl_work_surface->h, SDL_MapRGB(sl_work_surface->format, 100, 100, 0));
    draw_lmp(20, 20, &sl_pause_data);
    draw_lmp(20, 60, &sl_disc_data);

    return true;
}

bool
vid_update(void) {
    void *output_buffer = NULL;
    i32 pitch;

    // NOTE: Method of updating 8-bit palette without calling SDL_CreateTextureFromSurface every frame.
    // NOTE: http://sandervanderburg.blogspot.ca/2014/05/rendering-8-bit-palettized-surfaces-in.html

    // NOTE: sl_work_surface -> sl_output_surface = 8-bit -> 32-bit.
    sl_output_surface = SDL_ConvertSurfaceFormat(sl_work_surface, SDL_PIXELFORMAT_RGB888, 0);
    if(!sl_output_surface) return false;

    // NOTE: Lock the texture, copy sl_output_surface pixels to the locked buffer, then unlock the texture.
    // NOTE: After unlocking, sl_output_texture will hold the final pixel data.
    SDL_LockTexture(sl_output_texture, NULL, &output_buffer, &pitch);
    memcpy(output_buffer, sl_output_surface->pixels, sl_output_surface->pitch * sl_output_surface->h);
    SDL_UnlockTexture(sl_output_texture);

    // NOTE: Output the texture to the screen.  SDL_RenderCopy will scale the texture up.
    SDL_RenderCopy(sl_renderer, sl_output_texture, NULL, NULL);
    SDL_RenderPresent(sl_renderer);
    SDL_RenderClear(sl_renderer);

    return true;
}

void
vid_shutdown(void) {
    free(sl_disc_data.data);
    free(sl_pause_data.data);
    SDL_FreeSurface(sl_work_surface);
    SDL_FreeSurface(sl_output_surface);
    SDL_DestroyTexture(sl_output_texture);
    SDL_DestroyRenderer(sl_renderer);
    SDL_DestroyWindow(sl_window);
}

void
vid_toggle_fullscreen(void) {
    bool is_fullscreen = SDL_GetWindowFlags(sl_window) & SDL_WINDOW_FULLSCREEN;
    is_fullscreen ? SDL_SetWindowFullscreen(sl_window, 0) : SDL_SetWindowFullscreen(sl_window, SDL_WINDOW_FULLSCREEN);
}
