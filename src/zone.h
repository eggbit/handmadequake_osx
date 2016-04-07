#ifndef zone_h
#define zone_h

#include "quakedef.h"

void hunk_memory_init(void *buffer, i32 buffer_size);
void *hunk_alloc_name(u32 size, const char *name);
void *hunk_alloc(u32 size);

#endif  // zone_h
