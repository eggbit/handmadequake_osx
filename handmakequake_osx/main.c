#include <stdio.h>
#include <stdlib.h>
#include <sdl.h>

int main(int argc, const char * argv[]) {
    SDL_Window *window;
    SDL_Renderer *renderer;
    
    // SDL window and renderer Initialization
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) < 0) {
        goto error;
    }
    
    window = SDL_CreateWindow("handmadequake_osx", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, 0);
    
    if(window == NULL) {
        goto error;
    }
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    
    if(renderer == NULL) {
        goto error;
    }

    SDL_Event event;
    int r = 0, g = 0, b = 0, a = 255;
    
    // Main loop
    for(;;) {
        // Event processing
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) {
                goto exit;
            }
            
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
    }
    
error:
    printf("ERROR: %s", SDL_GetError());

exit:
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}
