#ifndef wad_h
#define wad_h

#include "quakedef.h"

void wad_load();
void *wad_get(const char *lumpname, u32 *length);
void wad_free(void);

#endif // wad_h
