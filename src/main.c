// TODO: SDL timing while windowed is very, very off.

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
    if(bpp == 1) {
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
    else if(bpp == 4) {
        i32 *buffer_walker = (i32 *)buffer;

        for(i32 y = 0; y < rect_height; y++) {
            for(i32 x = 0; x < rect_width; x++) {
                *buffer_walker = color;
                buffer_walker++;
            }

            buffer += s->w * bpp;
            buffer_walker = (i32 *)buffer;
        }
    }
}

int
main(int argc, const char *argv[]) {
    void *output_buffer = NULL; // NOTE: DO NOT TOUCH.  Will be passed to output_texture when it unlocks.
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Surface *work_surface = NULL; // NOTE: Holds pixel data we'll directly modify.
    SDL_Surface *output_surface = NULL; // NOTE: work_surface will be blitted here then converted.
    SDL_Texture *output_texture = NULL; // NOTE: Holds the final pixel data that'll be displayed.
    u32 output_format = SDL_PIXELFORMAT_RGB888;

    i32 pitch;
    struct Timer timer;

    i32 width = q_atoi(com_check_parm("-width", argc, argv));
    i32 height = q_atoi(com_check_parm("-height", argc, argv));

    if(!width) width = 640;
    if(!height) height = 480;

    if(!sdl_init("Handmade Quake OSX", width, height, &window, &renderer)) goto error;

    // NOTE: Set up work_surface, output_surface, output_texture.
    work_surface = SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0);
    output_surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
    output_texture = SDL_CreateTexture(renderer, output_format, SDL_TEXTUREACCESS_STREAMING, width, height);
    u8 bpp = work_surface->format->BytesPerPixel;

    // NOTE: Create and set the palette if 8-bit.
    if(bpp == 1) {
        SDL_Color c[255];

        for(u8 i = 255; i--;) {
            c[i].r = rand() % 255;
            c[i].g = rand() % 255;
            c[i].b = rand() % 255;
        }

        SDL_SetPaletteColors(work_surface->format->palette, c, 0, 256);
    }

    host_init();
    timer_init(&timer);
    srand(time(NULL));

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
        if(bpp == 1) {
            u8 *memory_walker = work_surface->pixels;

            for(i32 h = 0; h < height; h++) {
                for(i32 w = 0; w < width; w++)
                    *memory_walker++ = rand() % 255;
            }
        }
        else if(bpp == 4) {
            i32 *memory_walker = (i32 *)work_surface->pixels;

            for(i32 h = 0; h < height; h++) {
                for(i32 w = 0; w < width; w++) {
                    u8 r = rand() % 256;
                    u8 g = rand() % 256;
                    u8 b = rand() % 256;

                    *memory_walker++ = ((r << 16) | (g << 8) | b);
                }
            }
        }

        draw_rect(work_surface, 100, 100, 200, 200, SDL_MapRGB(work_surface->format, 255, 0, 0));

        // NOTE: Method of updating 8-bit palette without calling SDL_CreateTextureFromSurface every frame.
        // NOTE: http://sandervanderburg.blogspot.ca/2014/05/rendering-8-bit-palettized-surfaces-in.html

        // NOTE: Copy work_surface to output_surface.
        SDL_BlitSurface(work_surface, NULL, output_surface, NULL);

        // NOTE: Lock the texture, convert output_surface to a more native format, then unlock the texture.
        // NOTE: After unlocking, output_texture will hold the final pixel data.
        SDL_LockTexture(output_texture, NULL, &output_buffer, &pitch);
        sdl_convert_pixels(output_surface, output_format, output_buffer, pitch);
        SDL_UnlockTexture(output_texture);

        // NOTE: Output the texture to the screen.
        SDL_RenderCopy(renderer, output_texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

error:
    printf("ERROR: %s", SDL_GetError());

exit:
    host_shutdown();
    SDL_FreeSurface(work_surface);
    SDL_FreeSurface(output_surface);
    SDL_DestroyTexture(output_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
