// TODO: SDL timing while windowed is very, very off.
// TODO: Proper error checking everywhere.

#include "sys.h"
#include <stdarg.h>

#define VALID_PAK(p)  (p.magic[0] == 'P' && p.magic[1] == 'A' && p.magic[2] == 'C' && p.magic[3] == 'K')

struct timer_t {
    double seconds_per_tick;
    double oldtime;
    double delta;
    double time_passed;
    u64 time_count;
};

// NOTE: .PAK header
struct dpackheader_t {
    char magic[4];
    i32 directory_offset;
    i32 directory_length;
};

// NOTE: .PAK
struct pack_t {
    char pack_name[128];
    i32 pack_handle;
    i32 num_files;

    struct packfile_t {
        char file_name[56];
        i32 file_position;
        i32 file_length;
    } *pak_files;
};

struct searchpaths_t {
    struct pack_t *pak;
    struct searchpaths_t *next;
};

struct searchpaths_t *com_search_paths = NULL;

void
sys_timerinit(struct timer_t *t) {
    t->seconds_per_tick = 1.0 / (double)SDL_GetPerformanceFrequency();
    t->time_count = SDL_GetPerformanceCounter();
}

void
sys_timerupdate(struct timer_t *t) {
    u64 counter = SDL_GetPerformanceCounter();
    u64 interval = counter - t->time_count;

    t->time_count = counter;
    t->time_passed += (double)interval * t->seconds_per_tick;
    t->delta = t->time_passed - t->oldtime;
    t->oldtime = t->time_passed;
}

bool
sys_sendkeyevents() {
    SDL_Event event;
    SDL_PumpEvents();

    if(sdl_event_exists(&event, SDL_KEYDOWN)) {
        SDL_Keycode key = event.key.keysym.sym;

        if(key == SDLK_z) vid_setmode("Handmade Que?", 1);
        if(key == SDLK_x) vid_setmode("Handmade Que?", 2);
        if(key == SDLK_c) vid_setmode("Handmade Que?", 3);
        if(key == SDLK_p) vid_setmode("Handmade Fullscreen", 6);
        if(key == SDLK_t) vid_toggle_fullscreen();
    }

    if(sdl_event_exists(&event, SDL_QUIT)) return false;

    SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
    return true;
}

const char*
sys_va(const char *format, ...) {
    static char buffer[1024];
    va_list v;

    va_start(v, format);
    vsprintf(buffer, format, v);
    va_end(v);

    return buffer;
}

struct pack_t *
com_load_pak(const char *path) {
    struct dpackheader_t pak_header;

    // NOTE: Allocate the structre we'll be returning.
    struct pack_t *pak = malloc(sizeof(struct pack_t));
    q_strcpy(pak->pack_name, path);

    // NOTE: Open the .PAK and verify.
    pak->pack_handle = sys_fopen_rb(path, NULL);
    if(pak->pack_handle < 0) goto error;

    // NOTE: Read the header and verify.
    sys_fread(pak->pack_handle, &pak_header, sizeof(pak_header));
    if(!VALID_PAK(pak_header)) goto error;

    // NOTE: Get the nunber of files in the .PAK and allocate enough space for them.
    pak->num_files = pak_header.directory_length / sizeof(struct packfile_t);
    pak->pak_files = malloc(pak->num_files * sizeof(struct packfile_t));

    // NOTE: Move to the start of the files and read in the information.
    sys_fseek(pak->pack_handle, pak_header.directory_offset);
    sys_fread(pak->pack_handle, pak->pak_files, pak_header.directory_length);

    goto escape;

error:
    com_free(pak);
    printf("Error opening %s\n", path);

escape:
    return pak;
}

void
com_add_game_directory(const char *dir) {
    char buffer[128];
    struct pack_t *pak;

    for(i32 i = 0;; i++) {
        sprintf(buffer, "%s/PAK%d.PAK", dir, i);
        pak = com_load_pak(buffer);

        if(!pak) break;

        struct searchpaths_t *new = malloc(sizeof(struct searchpaths_t));
        new->pak = pak;
        new->next = com_search_paths;
        com_search_paths = new;
    }
}

void
com_free_directory() {
    struct searchpaths_t *node = com_search_paths;

    while(node != NULL) {
        struct searchpaths_t *temp = node;
        node = node->next;
        sys_fclose(temp->pak->pack_handle);
        com_free_pak(temp->pak);
        com_free(temp);
    }

    com_search_paths = NULL;
}

static i32
s_find_file(const char *path, struct searchpaths_t *node) {
    for(i32 i = 0; i < node->pak->num_files; i++) {
        if(!q_strcmp(path, node->pak->pak_files[i].file_name)) return i;
    }

    return -1;
}

static void *
s_get_file(struct searchpaths_t *node, i32 index, i32 *length) {
    i32 pak_handle = node->pak->pack_handle;
    i32 file_length = node->pak->pak_files[index].file_length;

    void *data = malloc(file_length);

    sys_fseek(pak_handle, node->pak->pak_files[index].file_position);
    i32 bytes_read = sys_fread(pak_handle, data, file_length);

    if(length) *length = bytes_read;

    return data;
}

void *
com_find_file(const char *path, i32 *length) {
    if(!path) goto escape;

    struct searchpaths_t *node = com_search_paths;

    while(node != NULL) {
        i32 index = s_find_file(path, node);
        if(index >= 0) return s_get_file(node, index, length);

        node = node->next;
    }

escape:
    return NULL;
}

int
main(__unused int argc, __unused const char *argv[]) {
    com_add_game_directory("data");

    i32 palette_length;
    void *bytes = com_find_file("gfx/pause.lmp", &palette_length);

    if(palette_length) {
        FILE *output = fopen("data/out_pause.lmp", "wb");
        fwrite(bytes, 1, palette_length, output);
        fclose(output);
    }

    com_free(bytes);
    com_free_directory();

//     if(!host_init()) goto exit;
//
//     struct timer_t timer;
//     sys_timerinit(&timer);
//
//     for(;;) {
//         sys_timerupdate(&timer);
//         if(!host_frame(timer.delta)) goto exit;
//     }
//
// exit:;
//     const char *error = SDL_GetError();
//     if(error != NULL && error[0] != '\0') printf("ERROR: %s\n", error);
//
//     host_shutdown();

    return 0;
}
