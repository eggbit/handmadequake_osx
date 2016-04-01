#include "wad.h"

// NOTE: wadinfo_t
struct wadhead_t {
    char magic[4];
    u32 num_entries;
    u32 dir_offset;
};

// NOTE: lumpinfo_t
struct wadentry_t {
    u32 offset;
    u32 dsize;
    u32 size;
    u8 type;
    u8 compressed;
    u16 dummy;
    char name[16];
};

static u8 *lk_wad;
static u32 lk_num_lumps;
static struct wadentry_t *lk_lumps;

static void
lk_clean_name(const char *in, char *out) {
    for(i32 i = 0; i < 16; i++) {
        char c = in[i];

        if(!c) {
            for(i32 x = i; x < 16; x++)
                out[i] = 0;

            break;
        }

        // NOTE: Capital to lower case.
        if(c >= 'A' && c <= 'Z')
            c += 32;

        out[i] = c;
    }
}

static struct wadentry_t*
lk_get_lump_info(const char *lumpname) {
    char clean_name[16];
    lk_clean_name(lumpname, clean_name);

    printf("Lump Name: %s\n", clean_name);

    for(u32 i = 0; i < lk_num_lumps; i++) {
        if(!q_strcmp(clean_name, lk_lumps[i].name))
            return &lk_lumps[i];
    }

    return NULL;
}

void
wad_load(const char *filename) {
    lk_wad = pak_get(filename, NULL);

    struct wadhead_t *wad_header = (struct wadhead_t *)lk_wad;

    if(strncmp(wad_header->magic, "WAD2", 4) != 0)
        return;

    lk_num_lumps = wad_header->num_entries;
    lk_lumps = (struct wadentry_t *)(lk_wad + wad_header->dir_offset);

    struct wadentry_t *lump_start = lk_lumps;

    for(u32 i = 0; i < wad_header->num_entries; i++) {
        lk_clean_name(lk_lumps->name, lk_lumps->name);
        lk_lumps++;
    }

    lk_lumps = lump_start;
}

void*
wad_get(const char *lumpname) {
    struct wadentry_t *lump = lk_get_lump_info(lumpname);

    // NOTE: Debug
    if(!lump) return NULL;

    printf("Found lump: %s\n", lump->name);
    return (void*)(lk_wad + lump->offset);
}

void
wad_free(void) {
    com_free(lk_wad);
}
