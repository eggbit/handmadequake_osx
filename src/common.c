#include "common.h"

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

static struct searchpaths_t *s_search_paths = NULL;

// Custom version of strcmp from string.h.  Same functionality and return values.
i32
q_strcmp(const char *str1, const char *str2) {
    if(!str1 || !str2) return -2;

    while(*str1 == *str2) {
        if(*str1 == '\0') return 0;

        ++str1;
        ++str2;
    }

    return (*str1 < *str2) ? 1 : -1;
}

// Updated version of atoi from stdlib.h.  Can handle hexadecimal and integer strings.
i32
q_atoi(const char *numstr) {
    // Check for null string
    if(!numstr) return 0;

    i32 sign = 1;
    i32 num = 0;
    i32 base = 10;

    while(*numstr) {
        // Check for signed/unsigned
        if(*numstr == '-')
            sign = -1;
        // Hexadecimal check.
        else if(*numstr == '0' && (*(numstr + 1) == 'x' || *(numstr + 1) == 'X')) {
            // Can't have negative hexadecimals (eg. -0x64BCD) or muli-hex (eg 0x350xf)
            if(sign < 0 || base == 16) return 0;

            // Hex is base-16, decidmal is base-10.
            base = 16;

            // Skip over the '0x'.
            ++numstr;
        }
        else if(*numstr >= '0' && *numstr <= '9')
            num = num * base + (*numstr - 48);
        else if(*numstr >= 'a' && *numstr <= 'f')
            num = num * base + (*numstr - 97) + 10;
        else if(*numstr >= 'A' && *numstr <= 'F')
            num = num * base + (*numstr - 65) + 10;
        else
            return num * sign;

        ++numstr;
    }

    return num * sign;
}

// Get the lengthof a given string.
size_t
q_strlen(const char *str) {
    if(!str) return 0;

    int i = 0;

    while(*str) {
        i++;
        str++;
    }

    return i;
}

// Custom versions of strcpy and strncpy from string.h.
void
q_strcpy(char *dest, const char *src) {
    if(!src) return;

    while(*src) {
        *dest++ = *src++;
    }
}

void
q_strncpy(char *dest, const char *src, i32 count) {
    if(!src || count < 0) return;

    while(*src && count) {
        *dest++ = *src++;
        --count;
    }

    while(*dest && count) {
        *dest++ = 0;
        --count;
    }
}

// Check if the command line argument 'search_arg' exists.  If it does, return the associated value.
// If it doesn't exist, return null.
//
//      eg. quake -setalpha 50 -startwindowed
//
//      const char *alpha_val = com_check_parm("-setalpha", argc, argv);
//      const char *windowed_val = com_check_parm("-startwindowed", argc, argv);
//
//      /* alpha_val will contain '50' */
//      /* windowed_val will contain '1' for true */
const char *
com_check_parm(const char *search_arg, u32 num_args, const char *args[]) {
    char *last_arg = "";

    // Loop backwards num_args amount of times.
    for(u32 i = num_args; i--;) {

        // See if any arguments match the one we're searching for.
        if(q_strcmp(args[i], search_arg) == 0) {

            // If last_arg isn't empty or the start of a new argument, return it.
            // Otherwise we have a boolean flag so just return a "1".
            return (*last_arg && *last_arg != '-') ? last_arg : "1";
        }

        // Save the parameter we just looked at for the next iteration of the loop;
        last_arg = (char *)args[i];
    }
    // Parameter not found
    return NULL;
}

static struct pack_t *
s_load_pak(const char *path) {
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

void
com_add_game_directory(const char *dir) {
    char buffer[128];
    struct pack_t *pak;

    for(i32 i = 0;; i++) {
        sprintf(buffer, "%s/PAK%d.PAK", dir, i);
        pak = s_load_pak(buffer);

        if(!pak) break;

        struct searchpaths_t *new = malloc(sizeof(struct searchpaths_t));
        new->pak = pak;
        new->next = s_search_paths;
        s_search_paths = new;
    }
}

void
com_free_directory() {
    for(struct searchpaths_t *node = s_search_paths; node != NULL; node = node->next) {
        struct searchpaths_t *temp = node;

        sys_fclose(temp->pak->pack_handle);
        com_free(temp->pak->pak_files);
        com_free(temp->pak);
        com_free(temp);
    }

    s_search_paths = NULL;
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
    for(struct searchpaths_t *node = s_search_paths; node != NULL; node = node->next) {
        i32 index = s_find_file(path, node);
        if(index >= 0) return s_get_file(node, index, length);
    }

    return NULL;
}
