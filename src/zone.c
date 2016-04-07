#include "zone.h"

#define HUNK_SENTINEL 0x1df001ed
#define ZONE_SIZE     0xc000    // 48KB

struct hunk_t {
    i32  sentinel;
    i32  size;
    char name[8];
};

// NOTE: Hunk - Main chunk of allocated memory
static i8* _hunk_base;
static i32 _hunk_size;
static i32 _hunk_low_offset;
static i32 _hunk_high_offset;

void
hunk_memory_init(void *buffer, i32 buffer_size) {
    _hunk_base = buffer;
    _hunk_size = buffer_size;
    _hunk_low_offset = 0;
    _hunk_high_offset = 0;

    // NOTE: Zone - Small section of the hunk for frequently allocated resources
    // void *zone = hunk_alloc_name(ZONE_SIZE, "zone");
}

void *
hunk_alloc(u32 size) {
    return hunk_alloc_name(size, "unknown");
}

void *
hunk_alloc_name(u32 size, __unused const char *name) {
    u32 total_size = sizeof(struct hunk_t) + ((size + 15) & ~15);   // NOTE: Align to 16 bytes.

    // NOTE: Get a new chunk of memory and zero it out.
    struct hunk_t *header = (struct hunk_t *)(_hunk_base + _hunk_low_offset);
    memset(header, 0, total_size);

    // NOTE: Update _hunk_low_offset for future allocation.
    _hunk_low_offset += total_size;

    // NOTE: Set the header.
    header->size = total_size;
    header->sentinel = HUNK_SENTINEL;
    strcpy(header->name, name);

    // NOTE: Move to the beginning of the allocated memory and return it.
    return (void *)(header + 1);
}
