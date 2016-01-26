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

    // Main loop
    for(;;) {
        u32 ticks_start = SDL_GetTicks();  // For use in capping the framerate below.
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

        // Cap the framerate to avoid high CPU usage from SDL_PollEvent.
        //  1. Calculate our frame time: (1000 / frames_per_second_we_want)
        //  2. Subtract that from the current ticks subtracted by the ticks we got at the start of the frame.
        i32 sleep_time = (1000 / 60) - (SDL_GetTicks() - ticks_start);

        // If sleep_time is greater than zero, sleep for that amount of time.
        if(sleep_time > 0) SDL_Delay(sleep_time);
    }

error:
    printf("ERROR: %s", SDL_GetError());

exit:
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
