#include "vid.h"

#define MAX_MODES 30

struct vmode_t {
    i32 width;
    i32 height;
    bool fullscreen;
};

// NOTE: Fullscreen modes will be added to this array via vid_init_fullscreen_mode function.
static struct vmode_t lk_mode_list[MAX_MODES] = {
    { 320, 240, false },
    { 640, 480, false },
    { 800, 600, false },
    { 1024, 768, false }
};

static void *lk_discdata = NULL;
static void *lk_pausedata = NULL;
static void *lk_tiledata = NULL;

static SDL_Window *lk_window = NULL;
static SDL_Renderer *lk_renderer = NULL;
static SDL_Surface *lk_surface = NULL;   // NOTE: Holds pixel data we'll directly modify.
static SDL_Texture *lk_texture = NULL;    // NOTE: Holds the final pixel data that'll be displayed.

void
vid_init_fullscreen_mode(void) {
    for(u32 i = 0; i < MAX_MODES; i++) {
        if(!lk_mode_list[i].width) { // NOTE: Start adding fullscreen modes when we hit first empty element in sl_mode_list.
            SDL_DisplayMode d;
            i32 num_modes = SDL_GetNumDisplayModes(0);

            // TODO: Multidisplay support.
            for(i32 x = i; x < num_modes; x++) {
                SDL_GetDisplayMode(0, x, &d);

                if(d.refresh_rate == 60) {
                    // printf("width: %d - height: %d - vsync: %d\n", d.w, d.h, d.refresh_rate);
                    lk_mode_list[x].fullscreen = true;
                    lk_mode_list[x].width = d.w;
                    lk_mode_list[x].height = d.h;
                }
            }
            break;
        }
    }
}

bool
vid_init(void) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
    vid_init_fullscreen_mode();

    // NOTE: Print out all modes
    // for(u32 i = 0; i < MAX_MODES; i++)
    //     printf("width: %d - height: %d - fullscreen: %s\n", lk_mode_list[i].width, lk_mode_list[i].height, lk_mode_list[i].fullscreen ? "true" : "false");

    return vid_setmode("Handmade Quake OSX", 3);
}

bool
vid_setmode(const char *title, i32 mode) {
    if(lk_window) vid_shutdown();

    i32 width = lk_mode_list[mode].width;
    i32 height = lk_mode_list[mode].height;
    i32 fullscreen_flag = (lk_mode_list[mode].fullscreen) ? SDL_WINDOW_FULLSCREEN : 0;

    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
    if(SDL_CreateWindowAndRenderer(width, height, fullscreen_flag, &lk_window, &lk_renderer) < 0) return false;
    SDL_SetWindowTitle(lk_window, title);

    // NOTE: Maintain 4:3 aspect ratio while fullscreen
    if(fullscreen_flag) SDL_RenderSetLogicalSize(lk_renderer, 320, 240);

    // NOTE: Create surfaces and output texture.
    lk_surface = SDL_CreateRGBSurface(0, 320, 240, 32, 0, 0, 0, 0);
    lk_texture = SDL_CreateTexture(lk_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, 320, 240);

    // TODO: Error checking.
    draw_load_palette();

    lk_discdata = pak_get("gfx/qplaque.lmp", NULL);
    lk_pausedata = pak_get("gfx/menuplyr.lmp", NULL);
    lk_tiledata = wad_get("num_9", NULL);

    return true;
}

bool
vid_draw(void) {
    draw_rect(lk_surface, 0, 0, lk_surface->w, lk_surface->h, SDL_MapRGB(lk_surface->format, 100, 100, 50));
    draw_pic(lk_surface, 100, 20, lk_pausedata);
    draw_pic(lk_surface, 20, 60, lk_discdata);
    draw_pic(lk_surface, 100, 120, lk_tiledata);

    return true;
}

bool
vid_update(void) {
    // NOTE: Method of updating 8-bit palette without calling SDL_CreateTextureFromSurface every frame.
    // NOTE: http://sandervanderburg.blogspot.ca/2014/05/rendering-8-bit-palettized-surfaces-in.html

    void *output_buffer = NULL;
    i32 pitch;

    // NOTE: Copy s_worksurface pixels to s_texture's pixel buffer.
    SDL_LockTexture(lk_texture, NULL, &output_buffer, &pitch);
    memcpy(output_buffer, lk_surface->pixels, lk_surface->pitch * lk_surface->h);
    SDL_UnlockTexture(lk_texture);

    // NOTE: Output the texture to the screen. SDL_RenderCopy will scale the texture up.
    SDL_RenderClear(lk_renderer);
    SDL_RenderCopy(lk_renderer, lk_texture, NULL, NULL);
    SDL_RenderPresent(lk_renderer);

    return true;
}

void
vid_shutdown(void) {
    com_free(lk_discdata);
    com_free(lk_pausedata);
    draw_free_palette();
    SDL_FreeSurface(lk_surface);
    SDL_DestroyTexture(lk_texture);
    SDL_DestroyRenderer(lk_renderer);
    SDL_DestroyWindow(lk_window);
}

void
vid_toggle_fullscreen(void) {
    bool is_fullscreen = SDL_GetWindowFlags(lk_window) & SDL_WINDOW_FULLSCREEN;
    is_fullscreen ? SDL_SetWindowFullscreen(lk_window, 0) : SDL_SetWindowFullscreen(lk_window, SDL_WINDOW_FULLSCREEN);
}
