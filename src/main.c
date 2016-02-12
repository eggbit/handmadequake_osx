#include "quakedef.h"
#include "host.h"

// Returns the game's total running time.
// time_counter is a variable that's been initialized outside of the game loop with
// SDL_GetPerformanceCounter.
double
sys_float_time(u64 *time_counter, double seconds_per_tick) {
    static double time_passed = 0;

    u64 counter = SDL_GetPerformanceCounter();
    u64 interval = counter - *time_counter;

    *time_counter = counter;

    return time_passed += (double)interval * seconds_per_tick;
}

void
sdl_toggle_fullscreen(SDL_Window *w) {
    bool is_fullscreen = SDL_GetWindowFlags(w) & SDL_WINDOW_FULLSCREEN;
    is_fullscreen ? SDL_SetWindowFullscreen(w, 0) : SDL_SetWindowFullscreen(w, SDL_WINDOW_FULLSCREEN);
}

int
main(int argc, const char *argv[]) {
    void *output_buffer = NULL;
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *output_texture = NULL; // NOTE: output_buffer will be copied to this texture.

    i32 width = q_atoi(com_check_parm("-width", argc, argv));
    i32 height = q_atoi(com_check_parm("-height", argc, argv));

    if(!width) width = 640;
    if(!height) height = 480;

    if(!sdl_init("Handmade Quake OSX", width, height, &window, &renderer)) goto error;

    // NOTE: Set up output_texture and output_buffer.
    output_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, width, height);
    output_buffer = malloc(width * height * sizeof(i32));

    host_init();

    // Set up timers and frame rate
    double seconds_per_tick = 1.0 / (double)SDL_GetPerformanceFrequency();
    double newtime = 0.0, oldtime = 0.0;
    u64 time_count = SDL_GetPerformanceCounter();

    // Main loop
    for(;;) {
        // Event processing
        SDL_Event event;
        sdl_pump_events();

        if(sdl_event_exists(&event, SDL_QUIT)) goto exit;
        if(sdl_event_exists(&event, SDL_KEYUP)) sdl_toggle_fullscreen(window);

        sdl_flush_events();

        // Update timers
        newtime = sys_float_time(&time_count, seconds_per_tick);
        host_frame(newtime - oldtime);
        oldtime = newtime;

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

        SDL_UpdateTexture(output_texture, NULL, output_buffer, width * sizeof(i32));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, output_texture, NULL, NULL);
        SDL_RenderPresent(renderer);
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
