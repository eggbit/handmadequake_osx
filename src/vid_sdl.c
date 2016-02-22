#include "vid.h"

static void *output_buffer = NULL;         // NOTE: DO NOT TOUCH.  Will be passed to output_texture when it unlocks.
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Surface *work_surface = NULL;   // NOTE: Holds pixel data we'll directly modify.
static SDL_Surface *tmp_surface = NULL;    // NOTE: Surface to convert 8-bit work_surface to 32-bit.
static SDL_Surface *output_surface = NULL; // NOTE: Will scale up tmp_surface to the final resolution.
static SDL_Texture *output_texture = NULL; // NOTE: Holds the final pixel data that'll be displayed.
static u32 output_format = SDL_PIXELFORMAT_RGB888;
static i32 pitch;

static i32 internal_width = 320;
static i32 internal_height = 240;
static i32 output_width = 640; //q_atoi(com_check_parm("-width", argc, argv));
static i32 output_height = 480; //q_atoi(com_check_parm("-height", argc, argv));

// if(!output_width) output_width = 640;
// if(!output_height) output_height = 480;

// if(!sdl_init("Handmade Quake OSX", output_width, output_height, &window, &renderer)) goto error;
// SDL_RenderSetLogicalSize(renderer, 640, 480);

// // NOTE: Set up work_surface, output_surface, output_texture.
// work_surface = SDL_CreateRGBSurface(0, internal_width, internal_height, 8, 0, 0, 0, 0);
// tmp_surface = SDL_CreateRGBSurface(0, internal_width, internal_height, 32, 0, 0, 0, 0);
// output_surface = SDL_CreateRGBSurface(0, output_width, output_height, 32, 0, 0, 0, 0);
// output_texture = SDL_CreateTexture(renderer, output_format, SDL_TEXTUREACCESS_STREAMING, output_width, output_height);

struct LmpData {
    i32 width;
    i32 height;
    void *data;
};

struct LmpData disc_data = {0};
struct LmpData pause_data = {0};

size_t
read_lmp(struct LmpData *lmp, const char *file_path) {
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
load_palette(SDL_Surface *s, const char *palette_path) {
    struct LmpData palette = {0};
    size_t bytes_read = read_lmp(&palette, "data/palette.lmp");

    if(!bytes_read) goto escape;

    SDL_Color c[255];
    u8 *p_data = palette.data;

    for(u8 i = 0; i < 255; i++) {
        c[i].r = *p_data++;
        c[i].g = *p_data++;
        c[i].b = *p_data++;
    }

    if(SDL_SetPaletteColors(s->format->palette, c, 0, 256) < 0) goto escape;

escape:
    free(palette.data);
    return bytes_read;
}


#define draw_lmp(surface, x, y, lmp_struct) draw_raw(surface, x, y, 0, 0, 0, lmp_struct)
#define draw_rect(surface, x, y, w, h, color) draw_raw(surface, x, y, w, h, color, NULL)

void
draw_raw(SDL_Surface *s, i32 x, i32 y, i32 width, i32 height, u32 color, struct LmpData *lmp) {
    u8 bpp = s->format->BytesPerPixel;
    u8 *dest = s->pixels;
    u8 *source = lmp ? lmp->data : NULL;

    // NOTE: Bounds checking
    if(height && width) {
        if((x + width) > s->w) width = s->w - x;
        if((y + height) > s->h) height = s->h - y;
    }
    else {
        height = lmp->height;
        width = lmp->width;
    }

    // NOTE: First pixel position.
    dest += (s->w * bpp * y) + (x * bpp);

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

        dest += s->w * bpp;
        buffer_walker = dest;
    }
}

bool
vid_setmode(const char *title, i32 width, i32 height) {
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) return 0;

    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0);
    if(!window) return 0;

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(!renderer) return 0;

    // NOTE: Set up work_surface, output_surface, output_texture.
    work_surface = SDL_CreateRGBSurface(0, internal_width, internal_height, 8, 0, 0, 0, 0);
    tmp_surface = SDL_CreateRGBSurface(0, internal_width, internal_height, 32, 0, 0, 0, 0);
    output_surface = SDL_CreateRGBSurface(0, output_width, output_height, 32, 0, 0, 0, 0);
    output_texture = SDL_CreateTexture(renderer, output_format, SDL_TEXTUREACCESS_STREAMING, output_width, output_height);

    // TODO: Error checking.
    load_palette(work_surface, "data/palette.lmp");
    read_lmp(&disc_data, "data/DISC.lmp");
    read_lmp(&pause_data, "data/pause.lmp");

    return 1;
}

void
vid_update(void) {
    // TODO: Move drawing to host_frame
    draw_rect(work_surface, 0, 0, work_surface->w, work_surface->h, SDL_MapRGB(work_surface->format, 100, 100, 0));
    draw_lmp(work_surface, 20, 20, &pause_data);
    draw_lmp(work_surface, 20, 60, &disc_data);

    // NOTE: Method of updating 8-bit palette without calling SDL_CreateTextureFromSurface every frame.
    // NOTE: http://sandervanderburg.blogspot.ca/2014/05/rendering-8-bit-palettized-surfaces-in.html

    // NOTE: work_surface -> tmp_surface = 8-bit -> 32-bit.
    // NOTE: tmp_surface -> output_surface = 320x240 -> output resolution.
    if(SDL_BlitSurface(work_surface, NULL, tmp_surface, NULL) < 0) return;
    if(SDL_BlitScaled(tmp_surface, NULL, output_surface, NULL) < 0) return;

    // NOTE: Lock the texture, convert output_surface to a more native format, then unlock the texture.
    // NOTE: After unlocking, output_texture will hold the final pixel data.
    SDL_LockTexture(output_texture, NULL, &output_buffer, &pitch);
    sdl_convert_pixels(output_surface, output_format, output_buffer, pitch);
    SDL_UnlockTexture(output_texture);

    // NOTE: Output the texture to the screen.
    SDL_RenderCopy(renderer, output_texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    SDL_RenderClear(renderer);
}

bool
vid_init(void) {
    return vid_setmode("Handmade Quake OSX", 640, 480);
}

void
vid_shutdown(void) {
    free(disc_data.data);
    free(pause_data.data);
    SDL_FreeSurface(work_surface);
    SDL_FreeSurface(output_surface);
    SDL_DestroyTexture(output_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

i32
sdl_convert_pixels(SDL_Surface *s, u32 format, void *pixels, i32 pitch) {
    return SDL_ConvertPixels(s->w, s->h, s->format->format, s->pixels, s->pitch, format, pixels, pitch);
}

void
sdl_toggle_fullscreen(SDL_Window *w) {
    bool is_fullscreen = SDL_GetWindowFlags(w) & SDL_WINDOW_FULLSCREEN;
    is_fullscreen ? SDL_SetWindowFullscreen(w, 0) : SDL_SetWindowFullscreen(w, SDL_WINDOW_FULLSCREEN);
}
