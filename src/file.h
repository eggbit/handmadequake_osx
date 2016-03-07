#ifndef file_h
#define file_h

#include "quakedef.h"

i32 sys_file_open_read(const char *path, i32 *size);
i32 sys_file_open_write(const char *path);
void sys_file_close(i32 handle);
void sys_file_seek(i32 handle, i32 position);
void sys_file_rewind(i32 handle);
i32 sys_file_read(i32 handle, void *dest, i32 count);
i32 sys_file_write(i32 handle, void *source, i32 count);

#endif // file_h
