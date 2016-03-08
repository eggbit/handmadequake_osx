#include "file.h"

#define MAX_HANDLES 10
#define VALID_HANDLE(h) (h >= 0 || h < MAX_HANDLES || s_fhandles[h])

static FILE *s_fhandles[MAX_HANDLES] = { 0 };

static i32
s_gethandle(void) {
    for(i32 i = 0; i < MAX_HANDLES; i++) {
        if(!s_fhandles[i]) return i;
    }

    return -1;
}

static i32
s_flength(FILE *f) {
    i32 length;

    fseek(f, 0, SEEK_END);
    length = (i32)ftell(f);
    rewind(f);

    return length;
}

static i32
s_fopen(const char *path, const char *mode, i32 *size) {
    i32 handle_index = s_gethandle();
    if(handle_index < 0) goto error;

    FILE *f = fopen(path, mode);
    if(!f) goto error;

    s_fhandles[handle_index] = f;

    if(size) *size = s_flength(f);
    goto escape;

error:
    handle_index = -1;

escape:
    return handle_index;
}

i32
sys_fopen_rb(const char *path, i32 *size) {
    return s_fopen(path, "rb", size);
}

i32
sys_fopen_wb(const char *path) {
    return s_fopen(path, "wb", NULL);
}

static i32
s_freadwrite(i32 handle, void *buffer, i32 count, bool read_mode) {
    if(!VALID_HANDLE(handle) || !buffer) return -1;

    return read_mode ? (i32)fread(buffer, 1, count, s_fhandles[handle]) : (i32)fwrite(buffer, 1, count, s_fhandles[handle]);
}

i32
sys_fread(i32 handle, void *dest, i32 count) {
    return s_freadwrite(handle, dest, count, true);
}

i32
sys_fwrite(i32 handle, void *source, i32 count) {
    return s_freadwrite(handle, source, count, false);
}

void
sys_fclose(i32 handle) {
    if(VALID_HANDLE(handle)) {
        fclose(s_fhandles[handle]);
        s_fhandles[handle] = NULL;
    }
}

void
sys_fseek(i32 handle, i32 position) {
    if(VALID_HANDLE(handle)) fseek(s_fhandles[handle], position, SEEK_SET);
}

void
sys_frewind(i32 handle) {
    if(VALID_HANDLE(handle)) rewind(s_fhandles[handle]);
}
