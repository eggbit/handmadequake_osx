#include "quakedef.h"

// Returns ticks as a double.
#define sys_init_time() ((double)SDL_GetPerformanceCounter())

// Returns the total amount of seconds the game has been running.
double
sys_get_time(double last_ticks) {
    static double total_time = 0;
    static double seconds_per_tick = 0;

    if(!seconds_per_tick) seconds_per_tick = 1.0 / (double)SDL_GetPerformanceFrequency();
    double ticks_elapsed = (double)SDL_GetPerformanceCounter() - last_ticks;

    return total_time += ticks_elapsed * seconds_per_tick;
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

    SDL_Event event;

    // Main loop
    for(;;) {
        double ticks_start = sys_init_time();
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

        double total_time = sys_get_time(ticks_start);

        printf("total time passed: %.9f\n", total_time);
    }

error:
    printf("ERROR: %s", SDL_GetError());

exit:
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
