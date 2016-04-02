#ifndef pak_h
#define pak_h

#include "quakedef.h"

void pak_load();
void *pak_get(const char *lumpname, i32 *length);
void pak_free();

#endif  // pak_h
