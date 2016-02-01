#include "quakedef.h"
#include "host.h"

// IDEA: Timer structure?

static u64 g_time_count = 0;

double
sys_init_float_time(void) {
    g_time_count = SDL_GetPerformanceCounter();
    return 0;
}

double
sys_float_time(double seconds_per_tick) {
    static double time_passed = 0;
    u64 counter = SDL_GetPerformanceCounter();

    u64 interval = counter - g_time_count;
    g_time_count = counter;

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

    double seconds_per_tick = 1.0 / (double)SDL_GetPerformanceFrequency();
    double newtime = 0.0, time_accumulated = 0.0;
    double target_fps = 1.0 / 30.0;
    double oldtime = sys_init_float_time();

    SDL_Color rgb = {0, 0, 0, 255};
    host_init();

    // Main loop
    for(;;) {
        // Event processing
        SDL_Event event;
        sdl_pump_events();

        if(sdl_event_exists(&event, SDL_QUIT)) goto exit;

        if(sdl_event_exists(&event, SDL_MOUSEBUTTONDOWN))
            rgb = sdl_handle_mousebuttondown();

        sdl_flush_events();

        newtime = sys_float_time(seconds_per_tick);
        time_accumulated += newtime - oldtime;
        oldtime = newtime;

        if (time_accumulated > target_fps)
        {
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
