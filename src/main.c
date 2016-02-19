// TODO: SDL timing while windowed is very, very off.
// TODO: draw_rect and draw_lmp are way too similar.
// NOTE: Since SDL needs to convert an 8-bit surface to 32-bits to get anything to display anyway, all 32-bit pixel code is redundant.

#include "quakedef.h"
#include "host.h"
#include <time.h>

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

void
draw_rect(SDL_Surface *s, i32 x, i32 y, i32 rect_width, i32 rect_height, u32 color) {
    u8 bpp = s->format->BytesPerPixel;
    u8 *buffer = s->pixels;

    // NOTE: Bounds checking
    if((x + rect_width) > s->w) rect_width = s->w - x;
    if((y + rect_height) > s->h) rect_height = s->h - y;

    // NOTE: First pixel position.
    buffer += (s->w * bpp * y) + (x * bpp);

    // TODO: Clean this up.
    u8 *buffer_walker = buffer;

    for(i32 y = 0; y < rect_height; y++) {
        for(i32 x = 0; x < rect_width; x++) {
            *buffer_walker = color;
            buffer_walker++;
        }

        buffer += s->w * bpp;
        buffer_walker = buffer;
    }
}

// NOTE: Read .lmp files found within Quake's .PAKs.  Returns 0 on error.
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

void
draw_lmp(SDL_Surface *s, i32 x, i32 y, struct LmpData *lmp) {
    u8 bpp = s->format->BytesPerPixel;
    u8 *dest = s->pixels;
    u8 *source = lmp->data;

    dest += (s->w * bpp * y) + (x * bpp);

    u8 *buffer_walker = dest;

    for(i32 y = 0; y < lmp->height; y++) {
        for(i32 x = 0; x < lmp->width; x++) {
            *buffer_walker = *source;
            buffer_walker++;
            source++;
        }

        dest += s->w * bpp;
        buffer_walker = dest;
    }
}

int
main(int argc, const char *argv[]) {
    void *output_buffer = NULL;         // NOTE: DO NOT TOUCH.  Will be passed to output_texture when it unlocks.
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Surface *work_surface = NULL;   // NOTE: Holds pixel data we'll directly modify.
    SDL_Surface *tmp_surface = NULL;    // NOTE: Surface to convert 8-bit work_surface to 32-bit.
    SDL_Surface *output_surface = NULL; // NOTE: Will scale up tmp_surface to the final resolution.
    SDL_Texture *output_texture = NULL; // NOTE: Holds the final pixel data that'll be displayed.
    u32 output_format = SDL_PIXELFORMAT_RGB888;

    i32 pitch;
    struct Timer timer;

    i32 output_width = q_atoi(com_check_parm("-width", argc, argv));
    i32 output_height = q_atoi(com_check_parm("-height", argc, argv));

    i32 internal_width = 320;
    i32 internal_height = 240;
    if(!output_width) output_width = 640;
    if(!output_height) output_height = 480;

    if(!sdl_init("Handmade Quake OSX", output_width, output_height, &window, &renderer)) goto error;
    // SDL_RenderSetLogicalSize(renderer, 640, 480);

    // NOTE: Set up work_surface, output_surface, output_texture.
    work_surface = SDL_CreateRGBSurface(0, internal_width, internal_height, 8, 0, 0, 0, 0);
    tmp_surface = SDL_CreateRGBSurface(0, internal_width, internal_height, 32, 0, 0, 0, 0);
    output_surface = SDL_CreateRGBSurface(0, output_width, output_height, 32, 0, 0, 0, 0);
    output_texture = SDL_CreateTexture(renderer, output_format, SDL_TEXTUREACCESS_STREAMING, output_width, output_height);

    // NOTE: Create and set the palette
    struct LmpData palette = {0};
    read_lmp(&palette, "data/palette.lmp");

    u8 *p_data = palette.data;

    SDL_Color c[255];

    for(u8 i = 0; i < 255; i++) {
        c[i].r = *p_data++;
        c[i].g = *p_data++;
        c[i].b = *p_data++;
    }

    if(SDL_SetPaletteColors(work_surface->format->palette, c, 0, 256) < 0) goto error;
    free(palette.data);

    struct LmpData disc_data = {0};
    struct LmpData pause_data = {0};

    read_lmp(&disc_data, "data/DISC.lmp");
    read_lmp(&pause_data, "data/pause.lmp");

    host_init();
    timer_init(&timer);
    srand((u32)time(NULL));

    // NOTE: Main loop
    for(;;) {
        // NOTE: Event processing
        SDL_Event event;
        sdl_pump_events();

        if(sdl_event_exists(&event, SDL_QUIT)) goto exit;
        if(sdl_event_exists(&event, SDL_KEYUP)) sdl_toggle_fullscreen(window);

        sdl_flush_events();

        // NOTE: Update timers and call host_frame.
        timer_update(&timer);
        host_frame(timer.delta);

        // TODO: Clean this up.
        // u8 *memory_walker = work_surface->pixels;
        //
        // for(i32 h = 0; h < height; h++) {
        //     for(i32 w = 0; w < width; w++)
        //         *memory_walker++ = rand() % 255;
        // }

        // draw_rect(work_surface, 0, 0, work_surface->w, work_surface->h, SDL_MapRGB(work_surface->format, 100, 100, 0));
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
    SDL_FreeSurface(work_surface);
    SDL_FreeSurface(output_surface);
    SDL_DestroyTexture(output_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
