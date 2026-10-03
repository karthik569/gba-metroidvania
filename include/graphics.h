#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "gba.h"

// Sprite shapes
#define ATTR0_SQUARE    0x0000
#define ATTR0_WIDE      0x4000
#define ATTR0_TALL      0x8000
#define ATTR0_REGULAR   0x0000
#define ATTR0_AFFINE    0x0100
#define ATTR0_HIDE      0x0200
#define ATTR0_4BPP      0x0000
#define ATTR0_8BPP      0x2000

// Sprite sizes
#define ATTR1_SIZE_8    0x0000
#define ATTR1_SIZE_16   0x4000
#define ATTR1_SIZE_32   0x8000
#define ATTR1_SIZE_64   0xC000
#define ATTR1_HFLIP     0x1000
#define ATTR1_VFLIP     0x2000

#define ATTR2_PAL(x)    ((x) << 12)
#define ATTR2_PRIO(x)   ((x) << 10)

#ifdef __cplusplus
extern "C" {
#endif

void gfx_init(void);
void vsync(void);
void oam_clear(void);
void oam_commit(void);
void oam_set(u8 id, s16 x, s16 y, u16 shape, u16 size, u16 tile, u8 pal, bool hflip, bool vflip);
void oam_hide(u8 id);

void load_bg_palette(const u16* palette, u8 count);
void load_obj_palette(const u16* palette, u8 count);
void load_bg_tiles(const u32* tiles, u32 count_words, u8 char_block);
void load_obj_tiles(const u32* tiles, u32 count_words);

#ifdef __cplusplus
}
#endif

#endif // GRAPHICS_H
