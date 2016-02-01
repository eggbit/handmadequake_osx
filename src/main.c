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

SDL_Color
sdl_handle_mousebuttondown() {
    SDL_Color c = {
        .r = rand() % 255,
        .g = rand() % 255,
        .b = rand() % 255,
        .a = rand() % 255
    };

    return c;
}

int
main(int argc, const char *argv[]) {
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    i32 width = q_atoi(com_check_parm("-width", argc, argv));
    i32 height = q_atoi(com_check_parm("-height", argc, argv));

    if(!width) width = 640;
    if(!height) height = 480;

    if(!sdl_init("Handmade Quake OSX", width, height, &window, &renderer)) goto error;

    host_init();
    SDL_Color rgb = {0, 0, 0, 255};

    // Set up timers and frame rate
    double seconds_per_tick = 1.0 / (double)SDL_GetPerformanceFrequency();
    double newtime = 0.0, oldtime = 0.0, time_accumulated = 0.0;
    double target_fps = 1.0 / 30.0;
    u64 time_count = SDL_GetPerformanceCounter();

    // Main loop
    for(;;) {
        // Event processing
        SDL_Event event;
        sdl_pump_events();

        if(sdl_event_exists(&event, SDL_QUIT)) goto exit;

        if(sdl_event_exists(&event, SDL_MOUSEBUTTONDOWN))
            rgb = sdl_handle_mousebuttondown();

        sdl_flush_events();

        // Update timers
        newtime = sys_float_time(&time_count, seconds_per_tick);
        time_accumulated += newtime - oldtime;
        oldtime = newtime;

        if(time_accumulated > target_fps)
        {
            // Rendering
            host_frame(target_fps);

            SDL_SetRenderDrawColor(renderer, rgb.r, rgb.g, rgb.b, rgb.a);
            SDL_RenderClear(renderer);
            SDL_RenderPresent(renderer);

            time_accumulated -= target_fps;
        }
    }

error:
    printf("ERROR: %s", SDL_GetError());

exit:
    host_shutdown();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
