// TODO: SDL timing while windowed is very, very off.
// NOTE: Since SDL needs to convert an 8-bit surface to 32-bits to get anything to display anyway, all 32-bit pixel code is redundant.

#include "sys.h"

struct Timer {
    double seconds_per_tick;
    double oldtime;
    double delta;
    double time_passed;
    u64 time_count;
};

struct LmpData {
    i32 width;
    i32 height;
    void *data;
};

void
timer_init(struct Timer *t) {
    t->seconds_per_tick = 1.0 / (double)SDL_GetPerformanceFrequency();
    t->time_count = SDL_GetPerformanceCounter();
}

void
timer_update(struct Timer *t) {
    u64 counter = SDL_GetPerformanceCounter();
    u64 interval = counter - t->time_count;

    t->time_count = counter;
    t->time_passed += (double)interval * t->seconds_per_tick;
    t->delta = t->time_passed - t->oldtime;
    t->oldtime = t->time_passed;
}

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

int
main(int argc, const char *argv[]) {
    // IDEA: Move all these variables to a struct?
    // TODO: Move all functionality to vid_setmode function.
    void *output_buffer = NULL;         // NOTE: DO NOT TOUCH.  Will be passed to output_texture when it unlocks.
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Surface *work_surface = NULL;   // NOTE: Holds pixel data we'll directly modify.
    SDL_Surface *tmp_surface = NULL;    // NOTE: Surface to convert 8-bit work_surface to 32-bit.
    SDL_Surface *output_surface = NULL; // NOTE: Will scale up tmp_surface to the final resolution.
    SDL_Texture *output_texture = NULL; // NOTE: Holds the final pixel data that'll be displayed.
    u32 output_format = SDL_PIXELFORMAT_RGB888;
    i32 pitch;

    i32 internal_width = 320;
    i32 internal_height = 240;
    i32 output_width = q_atoi(com_check_parm("-width", argc, argv));
    i32 output_height = q_atoi(com_check_parm("-height", argc, argv));

    if(!output_width) output_width = 640;
    if(!output_height) output_height = 480;

    if(!sdl_init("Handmade Quake OSX", output_width, output_height, &window, &renderer)) goto error;
    // SDL_RenderSetLogicalSize(renderer, 640, 480);

    // NOTE: Set up work_surface, output_surface, output_texture.
    work_surface = SDL_CreateRGBSurface(0, internal_width, internal_height, 8, 0, 0, 0, 0);
    tmp_surface = SDL_CreateRGBSurface(0, internal_width, internal_height, 32, 0, 0, 0, 0);
    output_surface = SDL_CreateRGBSurface(0, output_width, output_height, 32, 0, 0, 0, 0);
    output_texture = SDL_CreateTexture(renderer, output_format, SDL_TEXTUREACCESS_STREAMING, output_width, output_height);

    struct LmpData disc_data = {0};
    struct LmpData pause_data = {0};

    // TODO: Error checking.
    load_palette(work_surface, "data/palette.lmp");
    read_lmp(&disc_data, "data/DISC.lmp");
    read_lmp(&pause_data, "data/pause.lmp");

    host_init();

    struct Timer timer;
    timer_init(&timer);

    // NOTE: Main loop
    for(;;) {
        // NOTE: Event processing; move to vid_sdl.c
        SDL_Event event;
        sdl_pump_events();

        if(sdl_event_exists(&event, SDL_QUIT)) goto exit;
        if(sdl_event_exists(&event, SDL_KEYUP)) sdl_toggle_fullscreen(window);

        sdl_flush_events();

        // NOTE: Update timers and call host_frame.
        timer_update(&timer);
        host_frame(timer.delta);

        // TODO: Move drawing to host_frame
        draw_rect(work_surface, 0, 0, work_surface->w, work_surface->h, SDL_MapRGB(work_surface->format, 100, 100, 0));
        draw_lmp(work_surface, 20, 20, &pause_data);
        draw_lmp(work_surface, 20, 60, &disc_data);

        // NOTE: Method of updating 8-bit palette without calling SDL_CreateTextureFromSurface every frame.
        // NOTE: http://sandervanderburg.blogspot.ca/2014/05/rendering-8-bit-palettized-surfaces-in.html

        // NOTE: work_surface -> tmp_surface = 8-bit -> 32-bit.
        // NOTE: tmp_surface -> output_surface = 320x240 -> output resolution.
        if(SDL_BlitSurface(work_surface, NULL, tmp_surface, NULL) < 0) goto error;
        if(SDL_BlitScaled(tmp_surface, NULL, output_surface, NULL) < 0) goto error;

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

error:
    printf("ERROR: %s\n", SDL_GetError());

exit:
    free(disc_data.data);
    free(pause_data.data);
    host_shutdown();

    // TODO: Move to vid_shutdown.
    SDL_FreeSurface(work_surface);
    SDL_FreeSurface(output_surface);
    SDL_DestroyTexture(output_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
