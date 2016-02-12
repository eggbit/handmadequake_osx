#include "quakedef.h"
#include "host.h"

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

int
main(int argc, const char *argv[]) {
    void *output_buffer = NULL;
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *output_texture = NULL; // NOTE: output_buffer will be copied to this texture.

    struct Timer timer;

    i32 width = q_atoi(com_check_parm("-width", argc, argv));
    i32 height = q_atoi(com_check_parm("-height", argc, argv));

    if(!width) width = 640;
    if(!height) height = 480;

    if(!sdl_init("Handmade Quake OSX", width, height, &window, &renderer)) goto error;

    // NOTE: Set up output_texture and output_buffer.
    output_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, width, height);
    output_buffer = malloc(width * height * sizeof(i32));

    host_init();
    timer_init(&timer);

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

        i32 *memory_walker = (i32 *)output_buffer;

        // NOTE: Let there be color.
        for(i32 h = 0; h < height; h++) {
            for(i32 w = 0; w < width; w++) {
                u8 red = rand() % 256;
                u8 green = rand() % 256;
                u8 blue = rand() % 256;

                *memory_walker++ = ((red << 16) | (green << 8) | blue);
            }
        }

        sdl_display_buffer(renderer, output_buffer, output_texture, width * sizeof(i32));
    }

error:
    printf("ERROR: %s", SDL_GetError());

exit:
    host_shutdown();
    SDL_DestroyTexture(output_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
