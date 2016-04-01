#include "pak.h"

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

static struct searchpaths_t *s_search_paths = NULL;

static struct pack_t *
lk_load_pak(const char *path) {
    struct dpackheader_t pak_header;

    // NOTE: Allocate the structre we'll be returning.
    struct pack_t *pak = malloc(sizeof(struct pack_t));
    q_strcpy(pak->pack_name, path);

    // NOTE: Open the .PAK and verify.
    pak->pack_handle = sys_fopen_rb(path, NULL);
    if(pak->pack_handle < 0) goto error;

    // NOTE: Read the header and verify.
    sys_fread(pak->pack_handle, &pak_header, sizeof(pak_header));
    if(strncmp(pak_header.magic, "PACK", 4) != 0) goto error;

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

static void
lk_add_game_directory(const char *dir) {
    char buffer[128];
    struct pack_t *pak;

    for(i32 i = 0;; i++) {
        sprintf(buffer, "%s/PAK%d.PAK", dir, i);
        pak = lk_load_pak(buffer);

        if(!pak) break;

        struct searchpaths_t *new = malloc(sizeof(struct searchpaths_t));
        new->pak = pak;
        new->next = s_search_paths;
        s_search_paths = new;
    }
}

static i32
lk_find_file(const char *path, struct searchpaths_t *node) {
    for(i32 i = 0; i < node->pak->num_files; i++) {
        if(!q_strcmp(path, node->pak->pak_files[i].file_name)) return i;
    }

    return -1;
}

static void *
lk_get_file(struct searchpaths_t *node, i32 index, i32 *length) {
    i32 pak_handle = node->pak->pack_handle;
    i32 file_length = node->pak->pak_files[index].file_length;

    void *data = malloc(file_length);

    sys_fseek(pak_handle, node->pak->pak_files[index].file_position);
    i32 bytes_read = sys_fread(pak_handle, data, file_length);

    if(length) *length = bytes_read;

    return data;
}

// NOTE: com_file_init
void
pak_load(void) {
    lk_add_game_directory("data");
}

// NOTE: com_file_shutdown
void
pak_free(void) {
    for(struct searchpaths_t *node = s_search_paths; node != NULL; node = node->next) {
        struct searchpaths_t *temp = node;

        sys_fclose(temp->pak->pack_handle);
        com_free(temp->pak->pak_files);
        com_free(temp->pak);
        com_free(temp);
    }

    s_search_paths = NULL;
}

// NOTE: com_find_file
void *
pak_get(const char *path, i32 *length) {
    for(struct searchpaths_t *node = s_search_paths; node != NULL; node = node->next) {
        i32 index = lk_find_file(path, node);
        if(index >= 0) return lk_get_file(node, index, length);
    }

    return NULL;
}
