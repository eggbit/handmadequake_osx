#include "file.h"

#define MAX_HANDLES 10
#define VALID_HANDLE(h) (h >= 0 || h < MAX_HANDLES || sl_handles[h])

static FILE *sl_handles[MAX_HANDLES] = { 0 };

static i32
find_handle(void) {
    for(i32 i = 0; i < MAX_HANDLES; i++) {
        if(!sl_handles[i]) return i;
    }

    return -1;
}

static i32
file_length(FILE *f) {
    i32 length;

    fseek(f, 0, SEEK_END);
    length = (i32)ftell(f);
    rewind(f);

    return length;
}

static i32
file_open(const char *path, const char *mode, i32 *size) {
    i32 handle_index = find_handle();
    if(handle_index < 0) goto error;

    FILE *f = fopen(path, mode);
    if(!f) goto error;

    sl_handles[handle_index] = f;

    if(size) *size = file_length(f);
    goto escape;

error:
    handle_index = -1;

escape:
    return handle_index;
}

i32
sys_file_open_read(const char *path, i32 *size) {
    return file_open(path, "rb", size);
}

i32
sys_file_open_write(const char *path) {
    return file_open(path, "wb", NULL);
}

static i32
file_read_write(i32 handle, void *buffer, i32 count, bool read_mode) {
    if(!VALID_HANDLE(handle) || !buffer) return -1;

    return read_mode ? (i32)fread(buffer, 1, count, sl_handles[handle]) : (i32)fwrite(buffer, 1, count, sl_handles[handle]);
}

i32
sys_file_read(i32 handle, void *dest, i32 count) {
    return file_read_write(handle, dest, count, true);
}

i32
sys_file_write(i32 handle, void *source, i32 count) {
    return file_read_write(handle, source, count, false);
}

void
sys_file_close(i32 handle) {
    if(VALID_HANDLE(handle)) {
        fclose(sl_handles[handle]);
        sl_handles[handle] = NULL;
    }
}

void
sys_file_seek(i32 handle, i32 position) {
    if(VALID_HANDLE(handle)) fseek(sl_handles[handle], position, SEEK_SET);
}

void
sys_file_rewind(i32 handle) {
    if(VALID_HANDLE(handle)) rewind(sl_handles[handle]);
}
