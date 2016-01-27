#include "quakedef.h"

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

    SDL_Event event;
    double seconds_per_tick = 1.0 / (double)SDL_GetPerformanceFrequency();

    // Main loop
    for(;;) {
        u64 ticks_start = SDL_GetPerformanceCounter();

        static SDL_Color rgb = {0, 0, 0, 255};

        // Event processing
        sdl_pump_events();

        if(sdl_event_exists(&event, SDL_QUIT)) goto exit;

        if(sdl_event_exists(&event, SDL_MOUSEBUTTONDOWN))
            rgb = sdl_handle_mousebuttondown();

        sdl_flush_events();

        // Rendering
        SDL_SetRenderDrawColor(renderer, rgb.r, rgb.g, rgb.b, rgb.a);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);

        u64 ticks_elapsed = SDL_GetPerformanceCounter() - ticks_start;
        double secs_passed = (double)ticks_elapsed * seconds_per_tick;
        printf("interval: %llu\nseconds passed: %.9f\n\n", ticks_elapsed, secs_passed);
    }

error:
    printf("ERROR: %s", SDL_GetError());

exit:
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
