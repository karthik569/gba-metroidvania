#include "assets.h"
#include "graphics.h"
#include "asset_data.h"

// 5x7 ASCII font glyphs
static const u8 s_font_glyphs[39][5] = {
    // Digits '0'..'9' (0..9)
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    // Letters 'A'..'Z' (10..35)
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
    // Punctuation (36..38)
    {0x00, 0x36, 0x36, 0x00, 0x00}, // : (36)
    {0x08, 0x08, 0x08, 0x08, 0x08}, // - (37)
    {0x00, 0x60, 0x60, 0x00, 0x00}  // . (38)
};

static void set_tile_pixel(u8* tile_buf, u8 x, u8 y, u8 color) {
    u16 offset = (y * 4) + (x / 2);
    if (x & 1) {
        tile_buf[offset] = (tile_buf[offset] & 0x0F) | (color << 4);
    } else {
        tile_buf[offset] = (tile_buf[offset] & 0xF0) | (color & 0x0F);
    }
}

static void upload_8x8_tile(u16 tile_index, const u8* data) {
    vu16* dest = (vu16*)(VRAM_BASE + (BG_CBB_TILES * 0x4000) + (tile_index * 32));
    const u16* src = (const u16*)data;
    for (int i = 0; i < 16; i++) {
        dest[i] = src[i];
    }
}

static void build_font_tile(u16 tile_id, const u8 glyph[5]) {
    u8 buf[32];
    for (int i = 0; i < 32; i++) buf[i] = 0x22; // Solid Color 2 matching TILE_BOX_FILL

    for (int col = 0; col < 5; col++) {
        u8 bits = glyph[col];
        for (int row = 0; row < 7; row++) {
            if (bits & (1 << row)) {
                set_tile_pixel(buf, col + 2, row + 1, 10); // shadow
                set_tile_pixel(buf, col + 1, row, 4);      // white text
            }
        }
    }
    upload_8x8_tile(tile_id, buf);
}

// DMA helper for bulk transfers
static void dma_copy32(void* dest, const void* src, u32 words) {
    REG_DMA3SAD = (u32)src;
    REG_DMA3DAD = (u32)dest;
    REG_DMA3CNT = words | DMA_32 | DMA_ENABLE | DMA_IMMEDIATE;
}

void assets_init(void) {
    // 1. Upload Special background tiles (Empty, Black, White)
    u8 empty[32];
    for (int i = 0; i < 32; i++) empty[i] = 0;
    upload_8x8_tile(TILE_EMPTY, empty);

    u8 black[32];
    for (int i = 0; i < 32; i++) black[i] = 0x11;
    upload_8x8_tile(TILE_SOLID_BLACK, black);

    u8 white[32];
    for (int i = 0; i < 32; i++) white[i] = 0x44;
    upload_8x8_tile(TILE_SOLID_WHITE, white);

    // 2. Build ASCII Font tiles (3..48)
    for (int i = 0; i < 10; i++) {
        build_font_tile(TILE_DIGIT_0 + i, s_font_glyphs[i]);
    }
    for (int i = 0; i < 26; i++) {
        build_font_tile(TILE_LETTER_A + i, s_font_glyphs[10 + i]);
    }
    build_font_tile(TILE_COLON, s_font_glyphs[36]);
    build_font_tile(TILE_MINUS, s_font_glyphs[37]);
    build_font_tile(TILE_DOT,   s_font_glyphs[38]);

    // 3. Upload UI & HUD tiles (49..61)
    dma_copy32((void*)(VRAM_BASE + (BG_CBB_TILES * 0x4000) + (TILE_DIAMOND_ICON * 32)),
               g_tiles_ui,
               sizeof(g_tiles_ui) / 4);

    // 4. Upload Title Logo banner tiles (80 tiles at TILE_TITLE_LOGO_BASE = 256)
    dma_copy32((void*)(VRAM_BASE + (BG_CBB_TILES * 0x4000) + (TILE_TITLE_LOGO_BASE * 32)),
               g_tiles_title_logo,
               sizeof(g_tiles_title_logo) / 4);

    // 5. Upload Sprites to OBJ VRAM (0x06010000)
    // Explorer sprites (0..63)
    dma_copy32((void*)(VRAM_BASE + 0x10000 + (SPRITE_EXPLORER_D0 * 32)),
               g_sprites_explorer,
               sizeof(g_sprites_explorer) / 4);

    // Enemies (64..95)
    dma_copy32((void*)(VRAM_BASE + 0x10000 + (SPRITE_SNAKE_0 * 32)),
               g_sprites_enemies,
               sizeof(g_sprites_enemies) / 4);

    // Objects & FX (96..159)
    dma_copy32((void*)(VRAM_BASE + 0x10000 + (SPRITE_BOULDER_0 * 32)),
               g_sprites_objects,
               sizeof(g_sprites_objects) / 4);

    // 6. Load UI and OBJ Palettes
    for (int c = 0; c < 16; c++) {
        BG_PALETTE_RAM[PAL_BG_UI * 16 + c]       = g_pal_bg_ui[c];
        OBJ_PALETTE_RAM[PAL_OBJ_EXPLORER * 16 + c] = g_pal_obj_explorer[c];
        OBJ_PALETTE_RAM[PAL_OBJ_ENEMIES * 16 + c]  = g_pal_obj_enemies[c];
        OBJ_PALETTE_RAM[PAL_OBJ_BOULDER * 16 + c]  = g_pal_obj_objects[c];
        OBJ_PALETTE_RAM[3 * 16 + c]                = g_pal_obj_objects[c];
    }

    // 7. Load default theme (Angkor Wat)
    assets_load_theme(THEME_ANGKOR);
}

void assets_load_theme(LevelTheme theme) {
    const u16* theme_pal = (theme == THEME_BAVARIA) ? g_pal_bg_bavaria : g_pal_bg_angkor;
    const u8*  theme_tiles = (theme == THEME_BAVARIA) ? g_tiles_bavaria : g_tiles_angkor;

    // Upload 128 theme metatiles (4096 bytes) starting at TILE_FLOOR_TL = 64
    dma_copy32((void*)(VRAM_BASE + (BG_CBB_TILES * 0x4000) + (TILE_FLOOR_TL * 32)),
               theme_tiles,
               4096 / 4);

    // Assign theme palette to background palettes 1..4
    for (int c = 0; c < 16; c++) {
        BG_PALETTE_RAM[PAL_BG_STONE * 16 + c]   = theme_pal[c];
        BG_PALETTE_RAM[PAL_BG_DIRT * 16 + c]    = theme_pal[c];
        BG_PALETTE_RAM[PAL_BG_GEMS * 16 + c]    = theme_pal[c];
        BG_PALETTE_RAM[PAL_BG_OBJECTS * 16 + c] = theme_pal[c];
    }
}

void assets_update_vblank_effects(u32 frame_count) {
    // 1. Pulsating Mystic Exit Portal (Color 15 in palettes 1..4)
    static const u16 s_cyan_pulse[8] = {
        RGB15(4, 20, 26),
        RGB15(8, 24, 28),
        RGB15(14, 27, 30),
        RGB15(22, 30, 31),
        RGB15(31, 31, 31),
        RGB15(22, 30, 31),
        RGB15(14, 27, 30),
        RGB15(8, 24, 28)
    };
    u16 portal_col = s_cyan_pulse[(frame_count >> 2) & 7];
    BG_PALETTE_RAM[PAL_BG_STONE * 16 + 15]   = portal_col;
    BG_PALETTE_RAM[PAL_BG_OBJECTS * 16 + 15] = portal_col;

    // 2. Sparkling Gem Specular Facet (Color 14 in PAL_BG_GEMS)
    // Twinkles with bright flare every ~16 frames
    u8 phase = (frame_count >> 3) & 3;
    u16 gem_glint = (phase == 0) ? RGB15(31, 31, 31) : ((phase == 2) ? RGB15(28, 28, 30) : RGB15(31, 30, 26));
    BG_PALETTE_RAM[PAL_BG_GEMS * 16 + 14] = gem_glint;
}
