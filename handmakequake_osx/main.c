#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <sdl.h>

// Custom version of strcmp from string.h.  Same functionality and return values.
int32_t
q_strcmp(const char *str1, const char *str2) {
    while(*str1 == *str2) {
        if(*str1 == '\0') return 0;
        
        ++str1;
        ++str2;
    }
    
    return (*str1 < *str2) ? 1 : -1;
}

// Updated version of atoi from stdlib.h.  Can handle hexadecimal and integer strings.
int32_t
q_atoi(const char *numstr) {
    // Check for null string
    if(!numstr) return 0;
    
    int32_t sign = 1;
    int32_t num = 0;
    int32_t base = 10;
    
    while(*numstr) {
        // Check for signed/unsigned
        if(*numstr == '-') {
            sign = -1;
            
            // Go directly to next iteration of the loop.
            ++numstr;
            continue;
        }
        
        // Hexadecimal check.
        if(*numstr == '0' && (*(numstr + 1) == 'x' || *(numstr + 1) == 'X')) {
            // Can't have negative hexadecimals (eg. -0x64BCD) or muli-hex (eg 0x350xf)
            if(sign < 0 || base == 16) return 0;

            // Hex is base-16, decidmal is base-10.
            base = 16;
            
            // Skip over the '0x' and go to the next iteration.
            numstr += 2;
            continue;
        }
        
        if(*numstr >= '0' && *numstr <= '9') num = num * base + (*numstr - 48);
        if(*numstr >= 'a' && *numstr <= 'f') num = num * base + (*numstr - 97) + 10;
        if(*numstr >= 'A' && *numstr <= 'F') num = num * base + (*numstr - 65) + 10;
        
        ++numstr;
    }
    
    return num * sign;
}

// Check if the command line argument 'search_arg' exists.  If it does, return the associated value.
// If it doesn't exist, return null.
//
//      eg. quake -setalpha 50 -startwindowed
//
//      const char *alpha_val = CheckParam("-setalpha", argc, argv);
//      const char *windowed_val = CheckParam("-startwindowed", argc, argv);
//
//      /* alpha_val will contain '50' */
//      /* windowed_val will contain '1' for true */
const char *
com_check_parm(const char *search_arg, uint32_t num_args, const char *args[]) {
    // Loop through all available parameters.
    for(uint32_t i = 0; i < num_args; i++) {
        
        // See if any match the parameter we're searching for.
        if(strcmp(args[i], search_arg) == 0) {
            
            // If we found the parameter, get it's value
            const char *arg_val = args[++i];
            
            // if the parameter value is empty or the start of a new parameter, return 1, else return the value
            return (arg_val[0] == '\0' || arg_val[0] == '-') ? "1" : arg_val;
        }
    }
    // Parameter not found
    return NULL;
}

int
main(int argc, const char *argv[]) {
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    
    // SDL window and renderer Initialization
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) < 0) goto error;
    
    // Let's test our argument parser....
    int32_t width = q_atoi(com_check_parm("-width", argc, argv));
    int32_t height = q_atoi(com_check_parm("-height", argc, argv));
    
    if(!width) width = 640;
    if(!height) height = 480;
    
    window = SDL_CreateWindow("handmadequake_osx", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0);
    if(!window) goto error;
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if(!renderer) goto error;

    SDL_Event event;
    uint8_t r = 0, g = 0, b = 0, a = 255;
    
    // Main loop
    for(;;) {
        // For use in capping the framerate below.
        uint32_t ticks_start = SDL_GetTicks();
        
        // Event processing
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) goto exit;
            
            if(event.type == SDL_MOUSEBUTTONDOWN) {
                r = rand() % 255;
                g = rand() % 255;
                b = rand() % 255;
                a = rand() % 255;
            }
        }
        
        // Rendering
        SDL_SetRenderDrawColor(renderer, r, g, b, a);
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
