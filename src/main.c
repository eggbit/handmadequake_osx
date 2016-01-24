#include "quakedef.h"
#include <sdl.h>

// For naming consistency
#define sdl_pump_events() SDL_PumpEvents()

// Baseline initialization.
int32_t
sdl_init(const char *title, uint32_t width, uint32_t height, SDL_Window **win, SDL_Renderer **ren) {
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) return 0;

    *win = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0);
    if(!win) return 0;

    *ren = SDL_CreateRenderer(*win, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if(!ren) return 0;

    return 1;
}

bool
sdl_event_exists(SDL_Event *e, uint32_t event_type) {
    return (SDL_PeepEvents(e, 1, SDL_GETEVENT, event_type, event_type));
}

void
sdl_flush_events(void) {
    SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
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

    int32_t width = q_atoi(com_check_parm("-width", argc, argv));
    int32_t height = q_atoi(com_check_parm("-height", argc, argv));

    if(!width) width = 640;
    if(!height) height = 480;

    if(!sdl_init("Handmade Quake OSX", width, height, &window, &renderer)) goto error;

    SDL_Event event;

    // Main loop
    for(;;) {
        uint32_t ticks_start = SDL_GetTicks();  // For use in capping the framerate below.
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
        int32_t sleep_time = (1000 / 60) - (SDL_GetTicks() - ticks_start);

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
