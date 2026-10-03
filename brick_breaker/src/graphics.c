#include "graphics.h"
#include "gba.h"

static OBJ_ATTR s_shadow_oam[MAX_SPRITES];

void gfx_init(void) {
    // Mode 0: Tiled text mode, BG0 active, OBJ active with 1D tile mapping
    REG_DISPCNT = MODE_0 | BG0_ENABLE | OBJ_ENABLE | OBJ_1D_MAP;

    // BG0: CharBlock 0 (tiles), ScreenBlock 28 (map), 32x32 tiles, 16 colors (4bpp)
    REG_BG0CNT = BG_CBB(0) | BG_SBB(28) | BG_SIZE_32x32 | BG_COLOR_16;
    REG_BG0HOFS = 0;
    REG_BG0VOFS = 0;

    oam_clear();
    oam_commit();
}

void vsync(void) {
    // Wait until start of VBlank
    while (REG_VCOUNT >= 160);
    while (REG_VCOUNT < 160);
}

void oam_clear(void) {
    for (int i = 0; i < MAX_SPRITES; i++) {
        s_shadow_oam[i].attr0 = ATTR0_HIDE;
        s_shadow_oam[i].attr1 = 0;
        s_shadow_oam[i].attr2 = 0;
        s_shadow_oam[i].dummy = 0;
    }
}

void oam_set(u8 id, s16 x, s16 y, u16 shape, u16 size, u16 tile, u8 pal, bool hflip, bool vflip) {
    if (id >= MAX_SPRITES) return;

    u16 a0 = (y & 0x00FF) | shape | ATTR0_REGULAR;
    u16 a1 = (x & 0x01FF) | size;
    if (hflip) a1 |= ATTR1_HFLIP;
    if (vflip) a1 |= ATTR1_VFLIP;
    u16 a2 = (tile & 0x03FF) | ATTR2_PAL(pal);

    s_shadow_oam[id].attr0 = a0;
    s_shadow_oam[id].attr1 = a1;
    s_shadow_oam[id].attr2 = a2;
}

void oam_hide(u8 id) {
    if (id < MAX_SPRITES) {
        s_shadow_oam[id].attr0 = ATTR0_HIDE;
    }
}

void oam_commit(void) {
    // Commit shadow OAM buffer to hardware OAM via DMA channel 3
    REG_DMA3SAD = (u32)&s_shadow_oam[0];
    REG_DMA3DAD = (u32)OAM_BASE;
    REG_DMA3CNT = DMA_ENABLE | DMA_32 | (sizeof(s_shadow_oam) / 4);
}

void load_bg_palette(const u16* palette, u8 count) {
    for (u8 i = 0; i < count; i++) {
        BG_PALETTE_RAM[i] = palette[i];
    }
}

void load_obj_palette(const u16* palette, u8 count) {
    for (u8 i = 0; i < count; i++) {
        OBJ_PALETTE_RAM[i] = palette[i];
    }
}

void load_bg_tiles(const u32* tiles, u32 count_words, u8 char_block) {
    vu32* dest = (vu32*)(VRAM_BASE + (char_block * 0x4000));
    for (u32 i = 0; i < count_words; i++) {
        dest[i] = tiles[i];
    }
}

void load_obj_tiles(const u32* tiles, u32 count_words) {
    // OBJ tiles reside in CharBlock 4 (offset 0x10000 in VRAM)
    vu32* dest = (vu32*)(VRAM_BASE + 0x10000);
    for (u32 i = 0; i < count_words; i++) {
        dest[i] = tiles[i];
    }
}
