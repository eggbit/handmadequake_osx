#ifndef wad_h
#define wad_h

#include "quakedef.h"

void wad_load(const char *filename);
void *wad_get(char *lumpname);
void wad_free(void);

#endif // wad_h
