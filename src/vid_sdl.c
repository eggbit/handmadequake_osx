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
static struct vmode_t mode_list[MAX_MODES] = {
    { false, 320, 240 },
    { false, 640, 480 },
    { false, 800, 600 },
    { false, 1024, 768 }
};

static struct lmpdata_t disc_data = { 0 };
static struct lmpdata_t pause_data = { 0 };

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Surface *work_surface = NULL;   // NOTE: Holds pixel data we'll directly modify.
static SDL_Surface *tmp_surface = NULL;    // NOTE: Surface to convert 8-bit work_surface to 32-bit.
static SDL_Surface *output_surface = NULL; // NOTE: Will scale up tmp_surface to the final resolution.
static SDL_Texture *output_texture = NULL; // NOTE: Holds the final pixel data that'll be displayed.

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

    if(SDL_SetPaletteColors(work_surface->format->palette, c, 0, 256) < 0) goto escape;

escape:
    free(palette.data);
    return bytes_read;
}

// NOTE: draw_lmp and draw_rect are based on this function
void
draw_raw(i32 x, i32 y, i32 width, i32 height, u32 color, struct lmpdata_t *lmp) {
    u8 bpp = work_surface->format->BytesPerPixel;
    u8 *dest = work_surface->pixels;
    u8 *source = lmp ? lmp->data : NULL;

    // NOTE: Bounds checking
    if(height && width) {
        if((x + width) > work_surface->w) width = work_surface->w - x;
        if((y + height) > work_surface->h) height = work_surface->h - y;
    }
    else {
        height = lmp->height;
        width = lmp->width;
    }

    // NOTE: First pixel position.
    dest += (work_surface->w * bpp * y) + (x * bpp);

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

        dest += work_surface->w * bpp;
        buffer_walker = dest;
    }
}

void
vid_init_fullscreen_mode(void) {
    for(u32 i = 0; i < MAX_MODES; i++) {
        if(!mode_list[i].width) { // NOTE: Start adding fullscreen modes when we hit first empty element in mode_list.
            SDL_DisplayMode d;
            i32 num_modes = SDL_GetNumDisplayModes(0);

            // TODO: Multidisplay support.
            for(i32 x = i; x < num_modes; x++) {
                SDL_GetDisplayMode(0, x, &d);

                if(d.refresh_rate == 60) {
                    // printf("width: %d - height: %d - vsync: %d\n", d.w, d.h, d.refresh_rate);
                    mode_list[x].fullscreen = true;
                    mode_list[x].width = d.w;
                    mode_list[x].height = d.h;
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
        printf("width: %d - height: %d - fullscreen: %s\n", mode_list[i].width, mode_list[i].height, mode_list[i].fullscreen ? "true" : "false");

    return vid_setmode("Handmade Quake OSX", 1);
}

bool
vid_setmode(const char *title, i32 mode) {
    if(window) vid_shutdown();

    i32 width = mode_list[mode].width;
    i32 height = mode_list[mode].height;
    i32 fullscreen_flag = (mode_list[mode].fullscreen) ? SDL_WINDOW_FULLSCREEN : 0;

    if(SDL_CreateWindowAndRenderer(width, height, fullscreen_flag, &window, &renderer) < 0) return false;

    // NOTE: Maintain 4:3 aspect ratio while fullscreen
    if(fullscreen_flag) SDL_RenderSetLogicalSize(renderer, 320, 240);

    // NOTE: Create surfaces and output texture.
    work_surface = SDL_CreateRGBSurface(0, 320, 240, 8, 0, 0, 0, 0);
    tmp_surface = SDL_CreateRGBSurface(0, 320, 240, 32, 0, 0, 0, 0);
    output_surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
    output_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, width, height);

    // TODO: Error checking.
    load_palette("data/palette.lmp");
    read_lmp(&disc_data, "data/DISC.lmp");
    read_lmp(&pause_data, "data/pause.lmp");

    return true;
}

bool
vid_update(void) {
    void *output_buffer = NULL;
    i32 pitch;

    draw_rect(0, 0, work_surface->w, work_surface->h, SDL_MapRGB(work_surface->format, 100, 100, 0));
    draw_lmp(20, 20, &pause_data);
    draw_lmp(20, 60, &disc_data);

    // NOTE: Method of updating 8-bit palette without calling SDL_CreateTextureFromSurface every frame.
    // NOTE: http://sandervanderburg.blogspot.ca/2014/05/rendering-8-bit-palettized-surfaces-in.html

    // NOTE: work_surface -> tmp_surface = 8-bit -> 32-bit.
    // NOTE: tmp_surface -> output_surface = 320x240 -> output resolution.
    if(SDL_BlitSurface(work_surface, NULL, tmp_surface, NULL) < 0) return false;
    if(SDL_BlitScaled(tmp_surface, NULL, output_surface, NULL) < 0) return false;

    // NOTE: Lock the texture, convert output_surface to a more native format, then unlock the texture.
    // NOTE: After unlocking, output_texture will hold the final pixel data.
    SDL_LockTexture(output_texture, NULL, &output_buffer, &pitch);

    SDL_ConvertPixels(output_surface->w, output_surface->h, output_surface->format->format,
        output_surface->pixels, output_surface->pitch, SDL_PIXELFORMAT_RGB888, output_buffer, pitch);

    SDL_UnlockTexture(output_texture);

    // NOTE: Output the texture to the screen.
    SDL_RenderCopy(renderer, output_texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    SDL_RenderClear(renderer);

    return true;
}

void
vid_shutdown(void) {
    free(disc_data.data);
    free(pause_data.data);
    SDL_FreeSurface(work_surface);
    SDL_FreeSurface(tmp_surface);
    SDL_FreeSurface(output_surface);
    SDL_DestroyTexture(output_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

void
vid_toggle_fullscreen(void) {
    bool is_fullscreen = SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN;
    is_fullscreen ? SDL_SetWindowFullscreen(window, 0) : SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
}
