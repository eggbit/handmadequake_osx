#ifndef file_h
#define file_h

#include "quakedef.h"

i32 sys_fopen_rb(const char *path, i32 *size);
i32 sys_fopen_wb(const char *path);
void sys_fclose(i32 handle);
void sys_fseek(i32 handle, i32 position);
void sys_frewind(i32 handle);
i32 sys_fread(i32 handle, void *dest, i32 count);
i32 sys_fwrite(i32 handle, void *source, i32 count);

#endif // file_h
