#include "assets.h"
#include "graphics.h"

#define ROW(p0,p1,p2,p3,p4,p5,p6,p7) \
    ((u8)(((p0)&0xF) | (((p1)&0xF)<<4))), \
    ((u8)(((p2)&0xF) | (((p3)&0xF)<<4))), \
    ((u8)(((p4)&0xF) | (((p5)&0xF)<<4))), \
    ((u8)(((p6)&0xF) | (((p7)&0xF)<<4)))

// =========================================================================
// 15-bit Color Palettes (BG & OBJ)
// =========================================================================

static const u16 s_bg_palettes[6][16] = {
    // 0: PAL_BG_STONE (Dungeon masonry, stone floor, carved brick)
    {
        RGB15(0, 0, 0),       // 0: Transparent
        RGB15(2, 2, 3),       // 1: Deep shadow / mortar
        RGB15(5, 5, 7),       // 2: Dark slate stone
        RGB15(9, 9, 11),      // 3: Mid slate
        RGB15(14, 14, 16),    // 4: Light slate paving
        RGB15(18, 18, 20),    // 5: Stone highlight
        RGB15(10, 6, 4),      // 6: Brick brown
        RGB15(16, 10, 6),     // 7: Light brick
        RGB15(4, 5, 8),       // 8: Damp blue-gray stone
        RGB15(22, 22, 24),    // 9: Bright rim
        RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0)
    },
    // 1: PAL_BG_NATURE (Overworld camp grass, dirt, trees, wood)
    {
        RGB15(0, 0, 0),       // 0: Transparent
        RGB15(1, 4, 1),       // 1: Deep forest shadow
        RGB15(3, 10, 3),      // 2: Dark pine green
        RGB15(6, 18, 6),      // 3: Grass green
        RGB15(12, 25, 8),     // 4: Sunny grass highlight
        RGB15(8, 5, 2),       // 5: Dirt dark brown
        RGB15(14, 9, 4),      // 6: Dirt path brown
        RGB15(18, 13, 7),     // 7: Sand / dry dirt
        RGB15(20, 14, 6),     // 8: Wood bridge plank
        RGB15(10, 7, 3),      // 9: Tree bark
        RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0)
    },
    // 2: PAL_BG_WATER (Water canals, crystal switches)
    {
        RGB15(0, 0, 0),       // 0: Transparent
        RGB15(1, 3, 8),       // 1: Deep abyss water
        RGB15(2, 6, 16),      // 2: Deep blue channel
        RGB15(4, 12, 24),     // 3: Shimmering water mid
        RGB15(8, 20, 28),     // 4: Water surface crest
        RGB15(16, 26, 31),    // 5: Foam white
        RGB15(8, 2, 14),      // 6: Crystal switch dark
        RGB15(16, 4, 24),     // 7: Crystal switch violet
        RGB15(26, 12, 31),    // 8: Crystal shine
        RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0)
    },
    // 3: PAL_BG_FIRE (Torches, campfire, gold chests, boss doors)
    {
        RGB15(0, 0, 0),       // 0: Transparent
        RGB15(4, 1, 0),       // 1: Dark ember
        RGB15(14, 8, 3),      // 2: Chest wood dark
        RGB15(20, 13, 5),     // 3: Chest wood light
        RGB15(28, 22, 4),     // 4: Chest gold trim
        RGB15(24, 4, 2),      // 5: Torch flame red
        RGB15(31, 14, 2),     // 6: Torch flame orange
        RGB15(31, 26, 6),     // 7: Torch flame yellow
        RGB15(31, 31, 20),    // 8: Flame core white
        RGB15(28, 6, 6),      // 9: Boss skull red
        RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0)
    },
    // 4: PAL_BG_UI (Heads-up display, dialog boxes, text font)
    {
        RGB15(0, 0, 0),       // 0: Transparent
        RGB15(2, 2, 3),       // 1: Dialog border dark
        RGB15(4, 5, 8),       // 2: Dialog slate fill
        RGB15(10, 12, 16),    // 3: Dialog bevel light
        RGB15(31, 31, 31),    // 4: Crisp white text
        RGB15(20, 20, 20),    // 5: Dim gray text
        RGB15(28, 2, 4),      // 6: Heart crimson red
        RGB15(31, 12, 14),    // 7: Heart highlight
        RGB15(30, 24, 4),     // 8: Gold coin yellow
        RGB15(4, 14, 28),     // 9: MP magic blue
        RGB15(12, 24, 31),    // 10: MP bright cyan
        RGB15(20, 14, 6),     // 11: Key bronze
        RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0)
    },
    // 5: PAL_BG_SHADOW (Atmospheric arches, dark crypt vignette)
    {
        RGB15(0, 0, 0),       // 0: Transparent
        RGB15(1, 1, 2),       // 1: Black crypt vignette
        RGB15(2, 2, 4),       // 2: Deep shadow
        RGB15(4, 4, 6),       // 3: Archway dark stone
        RGB15(8, 8, 10),      // 4: Archway light stone
        RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0),
        RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0)
    }
};

static const u16 s_obj_palettes[6][16] = {
    // 0: PAL_OBJ_KNIGHT (Player character: armor, tunic, sword, skin)
    {
        RGB15(0, 0, 0),       // 0: Transparent
        RGB15(2, 2, 3),       // 1: Dark outline
        RGB15(8, 9, 12),      // 2: Steel armor shadow
        RGB15(16, 17, 20),    // 3: Steel armor mid
        RGB15(25, 26, 28),    // 4: Steel armor highlight
        RGB15(4, 8, 20),      // 5: Blue tunic dark
        RGB15(6, 14, 28),     // 6: Blue tunic mid
        RGB15(12, 22, 31),    // 7: Blue tunic bright
        RGB15(22, 14, 8),     // 8: Skin shadow
        RGB15(30, 22, 16),    // 9: Skin light
        RGB15(12, 6, 2),      // 10: Belt / leather boot brown
        RGB15(28, 22, 4),     // 11: Gold buckle / hilt
        RGB15(31, 31, 31),    // 12: Blade shine white
        RGB15(31, 10, 10),    // 13: Hurt flash red
        RGB15(0, 0, 0), RGB15(0, 0, 0)
    },
    // 1: PAL_OBJ_MONSTER (Skeletons, Slimes, Bats, Blade Traps)
    {
        RGB15(0, 0, 0),       // 0: Transparent
        RGB15(2, 2, 2),       // 1: Dark outline
        RGB15(10, 10, 10),    // 2: Bone shadow gray
        RGB15(24, 24, 22),    // 3: Bone white
        RGB15(28, 2, 2),      // 4: Red glowing eye / blood
        RGB15(2, 10, 3),      // 5: Slime green dark
        RGB15(6, 22, 6),      // 6: Slime green mid
        RGB15(14, 30, 10),    // 7: Slime highlight
        RGB15(4, 2, 8),       // 8: Bat dark purple
        RGB15(10, 5, 14),     // 9: Bat wing mid
        RGB15(18, 10, 24),    // 10: Bat highlight
        RGB15(8, 8, 10),      // 11: Blade trap dark iron
        RGB15(18, 18, 20),    // 12: Blade trap sharp edge
        RGB15(31, 31, 31),    // 13: Metal shine
        RGB15(0, 0, 0), RGB15(0, 0, 0)
    },
    // 2: PAL_OBJ_FIRE_BOMB (Remote Bombs, Explosions, Fireballs, Boomerang)
    {
        RGB15(0, 0, 0),       // 0: Transparent
        RGB15(1, 1, 2),       // 1: Bomb iron black
        RGB15(6, 6, 8),       // 2: Bomb iron mid
        RGB15(14, 14, 16),    // 3: Bomb specular highlight
        RGB15(26, 4, 2),      // 4: Fuse red / flame edge
        RGB15(31, 12, 2),     // 5: Flame orange
        RGB15(31, 24, 4),     // 6: Flame yellow
        RGB15(31, 31, 18),    // 7: Explosion core bright
        RGB15(8, 8, 10),      // 8: Smoke grey dark
        RGB15(16, 16, 18),    // 9: Smoke grey light
        RGB15(6, 16, 28),     // 10: Boomerang blue
        RGB15(14, 26, 31),    // 11: Boomerang cyan shine
        RGB15(28, 18, 4),     // 12: Wood boomerang
        RGB15(31, 31, 31),    // 13: White flash
        RGB15(0, 0, 0), RGB15(0, 0, 0)
    },
    // 3: PAL_OBJ_GOLEM (Ancient Crypt Golem, Flying Fists, Core Eye, Stalactites)
    {
        RGB15(0, 0, 0),       // 0: Transparent
        RGB15(2, 2, 2),       // 1: Black outline
        RGB15(5, 7, 7),       // 2: Dark granite rock
        RGB15(10, 13, 13),    // 3: Mid granite
        RGB15(16, 19, 18),    // 4: Light rock highlight
        RGB15(4, 9, 5),       // 5: Mossy lichen dark
        RGB15(8, 15, 8),      // 6: Mossy lichen green
        RGB15(24, 2, 2),      // 7: Core eye ruby dark
        RGB15(31, 6, 6),      // 8: Core eye ruby bright
        RGB15(31, 24, 10),    // 9: Core laser charge yellow
        RGB15(31, 31, 24),    // 10: Core laser beam white-yellow
        RGB15(10, 7, 4),      // 11: Stalactite dirt brown
        RGB15(31, 31, 31),    // 12: Destruction flash
        RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0)
    },
    // 4: PAL_OBJ_ITEMS (Heart pickups, coins, small keys, boss key, heart container)
    {
        RGB15(0, 0, 0),       // 0: Transparent
        RGB15(1, 1, 1),       // 1: Item outline
        RGB15(26, 2, 4),      // 2: Heart ruby red
        RGB15(31, 14, 16),    // 3: Heart specular shine
        RGB15(24, 18, 2),     // 4: Gold coin / key dark
        RGB15(31, 26, 4),     // 5: Gold coin / key bright
        RGB15(31, 31, 16),    // 6: Gold shine
        RGB15(4, 14, 26),     // 7: Magic flask blue
        RGB15(10, 24, 31),    // 8: Magic flask bright
        RGB15(22, 10, 28),    // 9: Boss key amethyst gem
        RGB15(31, 18, 31),    // 10: Gem sparkle
        RGB15(31, 31, 31),    // 11: Sparkle white
        RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0)
    },
    // 5: PAL_OBJ_NPC (Lorekeeper & Camp Merchant)
    {
        RGB15(0, 0, 0),       // 0: Transparent
        RGB15(2, 2, 2),       // 1: Outline
        RGB15(8, 4, 2),       // 2: Cloak leather dark
        RGB15(16, 10, 4),     // 3: Cloak leather light
        RGB15(20, 20, 20),    // 4: Beard silver
        RGB15(28, 28, 28),    // 5: Beard white
        RGB15(28, 18, 12),    // 6: Skin
        RGB15(6, 16, 26),     // 7: Amulet cyan
        RGB15(31, 24, 4),     // 8: Staff gold cap
        RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0)
    }
};

void assets_load_palettes(void) {
    for (int p = 0; p < 6; p++) {
        for (int c = 0; c < 16; c++) {
            BG_PALETTE_RAM[p * 16 + c] = s_bg_palettes[p][c];
            OBJ_PALETTE_RAM[p * 16 + c] = s_obj_palettes[p][c];
        }
    }
}

// =========================================================================
// 8x8 Procedural Tile & Font Generators for CBB 0
// =========================================================================

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

static void upload_sprite_tile(u16 tile_index, const u8* data) {
    // Sprites start at VRAM 0x06010000 (Char Block 4)
    vu16* dest = (vu16*)(VRAM_BASE + 0x10000 + (tile_index * 32));
    const u16* src = (const u16*)data;
    for (int i = 0; i < 16; i++) {
        dest[i] = src[i];
    }
}

// Compact 5x7 ASCII font definitions
static const u8 s_font_glyphs[39][5] = {
    // Digits '0'..'9' (indices 0..9)
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
    // Letters 'A'..'Z' (indices 10..35)
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
    // Punctuation (indices 36..38)
    {0x00, 0x36, 0x36, 0x00, 0x00}, // : (36)
    {0x08, 0x08, 0x08, 0x08, 0x08}, // - (37)
    {0x00, 0x60, 0x60, 0x00, 0x00}  // . (38)
};

static void build_font_tile(u16 tile_id, const u8 glyph[5]) {
    u8 buf[32];
    for (int i = 0; i < 32; i++) buf[i] = 0;

    for (int col = 0; col < 5; col++) {
        u8 bits = glyph[col];
        for (int row = 0; row < 7; row++) {
            if (bits & (1 << row)) {
                // Drop shadow
                set_tile_pixel(buf, col + 2, row + 1, 1);
                // Crisp white text
                set_tile_pixel(buf, col + 1, row, 4);
            }
        }
    }
    upload_8x8_tile(tile_id, buf);
}

// Generate UI dialog box border tiles
static void generate_ui_box_tiles(void) {
    u8 buf[32];

    // TILE_BOX_TL
    for (int i = 0; i < 32; i++) buf[i] = 0;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            u8 c = 2; // Fill
            if (x == 0 || y == 0) c = 3; // Bevel light
            if (x == 1 || y == 1) c = 1; // Border dark
            set_tile_pixel(buf, x, y, c);
        }
    }
    upload_8x8_tile(TILE_BOX_TL, buf);

    // TILE_BOX_TOP
    for (int i = 0; i < 32; i++) buf[i] = 0;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            u8 c = (y == 0) ? 3 : ((y == 1) ? 1 : 2);
            set_tile_pixel(buf, x, y, c);
        }
    }
    upload_8x8_tile(TILE_BOX_TOP, buf);

    // TILE_BOX_TR
    for (int i = 0; i < 32; i++) buf[i] = 0;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            u8 c = 2;
            if (x == 7 || y == 0) c = 1;
            if (x == 6 || y == 1) c = 3;
            set_tile_pixel(buf, x, y, c);
        }
    }
    upload_8x8_tile(TILE_BOX_TR, buf);

    // TILE_BOX_LEFT
    for (int i = 0; i < 32; i++) buf[i] = 0;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            u8 c = (x == 0) ? 3 : ((x == 1) ? 1 : 2);
            set_tile_pixel(buf, x, y, c);
        }
    }
    upload_8x8_tile(TILE_BOX_LEFT, buf);

    // TILE_BOX_RIGHT
    for (int i = 0; i < 32; i++) buf[i] = 0;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            u8 c = (x == 7) ? 1 : ((x == 6) ? 3 : 2);
            set_tile_pixel(buf, x, y, c);
        }
    }
    upload_8x8_tile(TILE_BOX_RIGHT, buf);

    // TILE_BOX_BL
    for (int i = 0; i < 32; i++) buf[i] = 0;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            u8 c = 2;
            if (x == 0 || y == 7) c = 1;
            if (x == 1 || y == 6) c = 3;
            set_tile_pixel(buf, x, y, c);
        }
    }
    upload_8x8_tile(TILE_BOX_BL, buf);

    // TILE_BOX_BOTTOM
    for (int i = 0; i < 32; i++) buf[i] = 0;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            u8 c = (y == 7) ? 1 : ((y == 6) ? 3 : 2);
            set_tile_pixel(buf, x, y, c);
        }
    }
    upload_8x8_tile(TILE_BOX_BOTTOM, buf);

    // TILE_BOX_BR
    for (int i = 0; i < 32; i++) buf[i] = 0;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            u8 c = 2;
            if (x == 7 || y == 7) c = 1;
            if (x == 6 || y == 6) c = 3;
            set_tile_pixel(buf, x, y, c);
        }
    }
    upload_8x8_tile(TILE_BOX_BR, buf);

    // TILE_BOX_FILL
    for (int i = 0; i < 32; i++) buf[i] = 0x22;
    upload_8x8_tile(TILE_BOX_FILL, buf);
}

// Generate HUD Heart Containers (Full, Half, Empty)
static void generate_hud_tiles(void) {
    u8 full[32] = {
        ROW(0,6,6,0,0,6,6,0),
        ROW(6,7,7,6,6,7,7,6),
        ROW(6,7,7,7,7,7,7,6),
        ROW(6,7,7,7,7,7,7,6),
        ROW(0,6,7,7,7,7,6,0),
        ROW(0,0,6,7,7,6,0,0),
        ROW(0,0,0,6,6,0,0,0),
        ROW(0,0,0,0,0,0,0,0)
    };
    upload_8x8_tile(TILE_HEART_FULL, full);

    u8 half[32] = {
        ROW(0,6,6,0,0,1,1,0),
        ROW(6,7,7,6,1,2,2,1),
        ROW(6,7,7,6,1,2,2,1),
        ROW(6,7,7,6,1,2,2,1),
        ROW(0,6,7,6,1,2,1,0),
        ROW(0,0,6,6,1,1,0,0),
        ROW(0,0,0,6,1,0,0,0),
        ROW(0,0,0,0,0,0,0,0)
    };
    upload_8x8_tile(TILE_HEART_HALF, half);

    u8 empty[32] = {
        ROW(0,1,1,0,0,1,1,0),
        ROW(1,2,2,1,1,2,2,1),
        ROW(1,2,2,2,2,2,2,1),
        ROW(1,2,2,2,2,2,2,1),
        ROW(0,1,2,2,2,2,1,0),
        ROW(0,0,1,2,2,1,0,0),
        ROW(0,0,0,1,1,0,0,0),
        ROW(0,0,0,0,0,0,0,0)
    };
    upload_8x8_tile(TILE_HEART_EMPTY, empty);

    // Key Icon
    u8 key[32] = {
        ROW(0,0,11,11,11,0,0,0),
        ROW(0,11,0,0,0,11,0,0),
        ROW(0,11,0,0,0,11,0,0),
        ROW(0,0,11,11,11,0,0,0),
        ROW(0,0,0,11,0,0,0,0),
        ROW(0,0,0,11,11,0,0,0),
        ROW(0,0,0,11,0,0,0,0),
        ROW(0,0,0,11,11,0,0,0)
    };
    upload_8x8_tile(TILE_KEY_ICON, key);

    // Coin Icon
    u8 coin[32] = {
        ROW(0,0,8,8,8,8,0,0),
        ROW(0,8,8,4,4,8,8,0),
        ROW(8,8,4,8,8,4,8,8),
        ROW(8,4,8,8,8,8,4,8),
        ROW(8,4,8,8,8,8,4,8),
        ROW(8,8,4,8,8,4,8,8),
        ROW(0,8,8,4,4,8,8,0),
        ROW(0,0,8,8,8,8,0,0)
    };
    upload_8x8_tile(TILE_COIN_ICON, coin);

    // MP Flask Icon
    u8 mp[32] = {
        ROW(0,0,0,9,9,0,0,0),
        ROW(0,0,0,9,9,0,0,0),
        ROW(0,0,9,10,10,9,0,0),
        ROW(0,9,10,10,10,10,9,0),
        ROW(0,9,10,10,10,10,9,0),
        ROW(0,9,10,10,10,10,9,0),
        ROW(0,0,9,9,9,9,0,0),
        ROW(0,0,0,0,0,0,0,0)
    };
    upload_8x8_tile(TILE_MP_ICON, mp);
}

// Generate 16x16 metatiles (4 8x8 tiles each) for dungeon terrain
static void generate_terrain_metatiles(void) {
    u8 buf[32];

    // META_FLOOR_STONE: Smooth granite floor flagstones with subtle mortar
    for (int t = 0; t < 4; t++) {
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                u8 c = 4; // Mid slate
                if ((x == 0 && (t & 1) == 0) || (y == 0 && (t < 2))) c = 5; // Light rim
                if (x == 7 || y == 7) c = 2; // Dark mortar
                if ((x == 3 && y == 3) || (x == 5 && y == 2)) c = 3; // Texture speckle
                set_tile_pixel(buf, x, y, c);
            }
        }
        upload_8x8_tile(TILE_FLOOR_STONE_TL + t, buf);
    }

    // META_FLOOR_DIRT: Camp dirt with grass sprigs
    for (int t = 0; t < 4; t++) {
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                u8 c = 6; // Dirt brown
                if ((x ^ y) % 3 == 0) c = 5;
                if ((x + y) % 5 == 0) c = 7;
                if ((x * 3 + y * 7) % 11 == 0) c = 3; // Green grass tuft
                set_tile_pixel(buf, x, y, c);
            }
        }
        upload_8x8_tile(TILE_FLOOR_DIRT_TL + t, buf);
    }

    // META_WALL_TOP: Wall top crenelations
    for (int t = 0; t < 4; t++) {
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                u8 c = 3;
                if (y == 0 || y == 1) c = 5; // Top stone rim
                if (y == 7) c = 1; // Deep shadow underside
                if (x == 0 || x == 7) c = 2;
                set_tile_pixel(buf, x, y, c);
            }
        }
        upload_8x8_tile(TILE_WALL_TOP_TL + t, buf);
    }

    // META_WALL_FACE: Front brick wall face
    for (int t = 0; t < 4; t++) {
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                u8 c = 3;
                if (y == 3 || y == 7) c = 1; // Horizontal mortar lines
                if ((y < 3 && x == 3) || (y >= 4 && y < 7 && x == 6)) c = 1; // Vertical mortar
                if (y == 0) c = 2; // Under-crenelation shadow
                set_tile_pixel(buf, x, y, c);
            }
        }
        upload_8x8_tile(TILE_WALL_FACE_TL + t, buf);
    }

    // META_WATER: Subterranean water channels
    for (int t = 0; t < 4; t++) {
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                u8 c = 2; // Deep blue
                if ((x + y * 2) % 4 == 0) c = 3;
                if ((x + y) % 7 == 0) c = 4; // Crest
                set_tile_pixel(buf, x, y, c);
            }
        }
        upload_8x8_tile(TILE_WATER_TL + t, buf);
    }

    // META_BRIDGE: Wooden plank bridge
    for (int t = 0; t < 4; t++) {
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                u8 c = 8; // Wood bridge
                if (y % 4 == 0) c = 5; // Plank separation
                if (x == 0 || x == 7) c = 9; // Side rope rail
                set_tile_pixel(buf, x, y, c);
            }
        }
        upload_8x8_tile(TILE_BRIDGE_TL + t, buf);
    }

    // META_DOOR_OPEN: Open passage archway
    for (int t = 0; t < 4; t++) {
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                u8 c = 1; // Deep dark portal
                if (y == 0 && t < 2) c = 4; // Arch stone
                if (x == 0 && (t & 1) == 0) c = 3;
                if (x == 7 && (t & 1) == 1) c = 3;
                set_tile_pixel(buf, x, y, c);
            }
        }
        upload_8x8_tile(TILE_DOOR_OPEN_TL + t, buf);
    }

    // META_DOOR_LOCKED: Iron portcullis gate
    for (int t = 0; t < 4; t++) {
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                u8 c = 1; // Dark behind
                if (x == 2 || x == 5) c = 5; // Vertical iron bars
                if (y == 2 || y == 5) c = 4; // Horizontal cross-struts
                if (y == 7 && t >= 2 && (x == 2 || x == 5)) c = 9; // Spikes
                set_tile_pixel(buf, x, y, c);
            }
        }
        upload_8x8_tile(TILE_DOOR_LOCKED_TL + t, buf);
    }

    // META_DOOR_BOSS: Golden skull boss door
    for (int t = 0; t < 4; t++) {
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                u8 c = 3; // Chest wood
                if (x == 0 || x == 7 || y == 0 || y == 7) c = 4; // Gold frame
                if ((x >= 2 && x <= 5) && (y >= 2 && y <= 5)) c = 9; // Skull ruby emblem
                set_tile_pixel(buf, x, y, c);
            }
        }
        upload_8x8_tile(TILE_DOOR_BOSS_TL + t, buf);
    }

    // META_WALL_CRACKED: Cracked bombable wall
    for (int t = 0; t < 4; t++) {
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                u8 c = 3;
                if (x == y || x == 7 - y) c = 1; // Jagged crack fissure
                set_tile_pixel(buf, x, y, c);
            }
        }
        upload_8x8_tile(TILE_WALL_CRACKED_TL + t, buf);
    }

    // META_SWITCH_OFF & ON: Crystal switches
    for (int t = 0; t < 4; t++) {
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                u8 off_c = 2; // Floor
                if ((x + y >= 4) && (x + y <= 10)) off_c = 7; // Violet switch
                set_tile_pixel(buf, x, y, off_c);
            }
        }
        upload_8x8_tile(TILE_SWITCH_OFF_TL + t, buf);

        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                u8 on_c = 2;
                if ((x + y >= 4) && (x + y <= 10)) on_c = 6;  // Red active switch
                set_tile_pixel(buf, x, y, on_c);
            }
        }
        upload_8x8_tile(TILE_SWITCH_ON_TL + t, buf);
    }

    // META_PLATE_UP & DOWN: Pressure plates
    for (int t = 0; t < 4; t++) {
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                u8 c = 3;
                if (x >= 2 && x <= 5 && y >= 2 && y <= 5) c = 5; // Elevated plate
                set_tile_pixel(buf, x, y, c);
            }
        }
        upload_8x8_tile(TILE_PLATE_UP_TL + t, buf);
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                u8 c = 3;
                if (x >= 2 && x <= 5 && y >= 2 && y <= 5) c = 1; // Pressed plate
                set_tile_pixel(buf, x, y, c);
            }
        }
        upload_8x8_tile(TILE_PLATE_DOWN_TL + t, buf);
    }

    // META_TORCH_UNLIT & LIT
    for (int t = 0; t < 4; t++) {
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                u8 unlit = 3;
                if (x >= 2 && x <= 5 && y >= 3 && y <= 6) unlit = 1; // Cold iron sconce
                set_tile_pixel(buf, x, y, unlit);
            }
        }
        upload_8x8_tile(TILE_TORCH_UNLIT_TL + t, buf);

        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                u8 lit = 3;
                if (x >= 2 && x <= 5 && y >= 3 && y <= 6) {
                    lit = (x == 3 || x == 4) ? 7 : 6; // Flame
                }
                set_tile_pixel(buf, x, y, lit);
            }
        }
        upload_8x8_tile(TILE_TORCH_LIT_TL + t, buf);
    }

    // META_CHEST_CLOSED & OPEN
    for (int t = 0; t < 4; t++) {
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                u8 closed = 2; // Dark wood
                if (y == 2 || y == 6) closed = 4; // Gold trim
                if (x == 0 || x == 7) closed = 4;
                if (y == 4 && (x == 3 || x == 4)) closed = 4; // Keyhole
                set_tile_pixel(buf, x, y, closed);
            }
        }
        upload_8x8_tile(TILE_CHEST_CLOSED_TL + t, buf);
        upload_8x8_tile(TILE_CHEST_OPEN_TL + t, buf);
    }

    // META_POT: Clay jar
    for (int t = 0; t < 4; t++) {
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                u8 c = 6; // Terracotta clay
                if (x >= 2 && x <= 5 && y >= 2 && y <= 6) c = 7;
                if (x == 0 || x == 7 || y == 0 || y == 7) c = 4; // Floor
                set_tile_pixel(buf, x, y, c);
            }
        }
        upload_8x8_tile(TILE_POT_TL + t, buf);
    }

    // META_BLOCK: Pushable stone block
    for (int t = 0; t < 4; t++) {
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                u8 c = 3;
                if (x == 0 || y == 0) c = 5; // Highlight
                if (x == 7 || y == 7) c = 1; // Shadow
                if (x == y || x == 7 - y) c = 4; // Carved X
                set_tile_pixel(buf, x, y, c);
            }
        }
        upload_8x8_tile(TILE_BLOCK_TL + t, buf);
    }

    // META_CAMPFIRE
    for (int t = 0; t < 4; t++) {
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                u8 c = 1; // Ash
                if (x >= 2 && x <= 5 && y >= 2 && y <= 5) c = 6; // Fire
                if (x == 3 || x == 4) c = 7; // Core
                set_tile_pixel(buf, x, y, c);
            }
        }
        upload_8x8_tile(TILE_CAMPFIRE_TL + t, buf);
    }

    // META_TREE
    for (int t = 0; t < 4; t++) {
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                u8 c = 2; // Pine dark
                if (x >= 2 && x <= 5 && y <= 5) c = 3; // Pine green
                if (t >= 2 && y >= 5 && (x == 3 || x == 4)) c = 9; // Trunk
                set_tile_pixel(buf, x, y, c);
            }
        }
        upload_8x8_tile(TILE_TREE_TL + t, buf);
    }

    // BG1 Foreground Arches & Pillars
    for (int t = 0; t < 4; t++) {
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                u8 c = 3; // Arch dark stone
                if (x == 1 || y == 1) c = 4; // Arch light stone
                if (x == 0 || y == 0) c = 1; // Shadow
                set_tile_pixel(buf, x, y, c);
            }
        }
        upload_8x8_tile(TILE_ARCH_TL + t, buf);
    }

    // BG1 Vignette darkness mask
    for (int t = 0; t < 4; t++) {
        for (int i = 0; i < 32; i++) buf[i] = 0x11; // Solid dark
        upload_8x8_tile(TILE_VIGNETTE_TL + t, buf);
    }
}

// =========================================================================
// 16x16 Hardware Sprite Generators for OBJ VRAM
// =========================================================================

static void generate_sprite_16x16(u16 base_tile, u8 pal, const u8 pixels[16][16]) {
    u8 tile_buf[32];

    // Top-Left (0..7, 0..7)
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            set_tile_pixel(tile_buf, x, y, pixels[y][x]);
        }
    }
    upload_sprite_tile(base_tile + 0, tile_buf);

    // Top-Right (8..15, 0..7)
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            set_tile_pixel(tile_buf, x, y, pixels[y][x + 8]);
        }
    }
    upload_sprite_tile(base_tile + 1, tile_buf);

    // Bottom-Left (0..7, 8..15)
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            set_tile_pixel(tile_buf, x, y, pixels[y + 8][x]);
        }
    }
    upload_sprite_tile(base_tile + 2, tile_buf);

    // Bottom-Right (8..15, 8..15)
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            set_tile_pixel(tile_buf, x, y, pixels[y + 8][x + 8]);
        }
    }
    upload_sprite_tile(base_tile + 3, tile_buf);
    (void)pal;
}

static void generate_knight_sprites(void) {
    u8 p[16][16];

    // Knight Facing Down
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = 0;
            // Helmet (y: 1..5, x: 5..10)
            if (y >= 1 && y <= 5 && x >= 5 && x <= 10) c = 3;
            if (y == 3 && (x == 6 || x == 9)) c = 1; // Visor slit
            // Tunic & Breastplate (y: 6..11, x: 4..11)
            if (y >= 6 && y <= 11 && x >= 4 && x <= 11) c = 6; // Blue tunic
            if (y >= 7 && y <= 9 && x >= 6 && x <= 9) c = 4;   // Silver plate
            // Shield on left arm (x: 2..4, y: 7..11)
            if (y >= 7 && y <= 11 && x >= 2 && x <= 4) c = 3;
            // Sword pommel on right (x: 12..13, y: 8..12)
            if (y >= 8 && y <= 12 && x >= 12 && x <= 13) c = 11;
            // Boots (y: 12..14, x: 5..6 and 9..10)
            if (y >= 12 && y <= 14 && ((x >= 5 && x <= 6) || (x >= 9 && x <= 10))) c = 10;
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_KNIGHT_DOWN_0, PAL_OBJ_KNIGHT, p);

    // Walk frame (bob)
    p[14][5] = 0;
    p[14][10] = 10;
    generate_sprite_16x16(SPRITE_KNIGHT_DOWN_1, PAL_OBJ_KNIGHT, p);

    // Knight Facing Up
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = 0;
            if (y >= 1 && y <= 5 && x >= 5 && x <= 10) c = 3; // Helmet back
            if (y >= 6 && y <= 11 && x >= 4 && x <= 11) c = 5; // Blue cape
            if (y >= 12 && y <= 14 && ((x >= 5 && x <= 6) || (x >= 9 && x <= 10))) c = 10;
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_KNIGHT_UP_0, PAL_OBJ_KNIGHT, p);
    p[14][6] = 0;
    generate_sprite_16x16(SPRITE_KNIGHT_UP_1, PAL_OBJ_KNIGHT, p);

    // Knight Facing Side
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = 0;
            if (y >= 1 && y <= 5 && x >= 5 && x <= 9) c = 3;
            if (y == 3 && x == 8) c = 1;
            if (y >= 6 && y <= 11 && x >= 4 && x <= 10) c = 6;
            if (y >= 7 && y <= 10 && x >= 7 && x <= 9) c = 4;
            if (y >= 12 && y <= 14 && x >= 6 && x <= 9) c = 10;
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_KNIGHT_SIDE_0, PAL_OBJ_KNIGHT, p);
    p[14][8] = 0;
    generate_sprite_16x16(SPRITE_KNIGHT_SIDE_1, PAL_OBJ_KNIGHT, p);

    // Sword Slash Down Frame
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = p[y][x];
            if (y >= 11 && y <= 15 && x >= 10 && x <= 14) c = 12; // Blade thrust
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_KNIGHT_SLASH_D, PAL_OBJ_KNIGHT, p);

    // Sword Slash Side Frame
    generate_sprite_16x16(SPRITE_KNIGHT_SLASH_S, PAL_OBJ_KNIGHT, p);

    // Sword Slash Up Frame
    generate_sprite_16x16(SPRITE_KNIGHT_SLASH_U, PAL_OBJ_KNIGHT, p);

    // Dodge Roll Frames
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = 0;
            if ((x - 8) * (x - 8) + (y - 8) * (y - 8) <= 36) c = 6; // Blue ball
            if ((x - 8) * (x - 8) + (y - 8) * (y - 8) <= 16) c = 3; // Silver core
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_KNIGHT_ROLL_0, PAL_OBJ_KNIGHT, p);
    generate_sprite_16x16(SPRITE_KNIGHT_ROLL_1, PAL_OBJ_KNIGHT, p);

    // Sword Arc Effects
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = 0;
            if (y >= 10 && x >= 2 && x <= 14) c = 12; // Silver swoosh
            if (y >= 12 && x >= 4 && x <= 12) c = 7;  // Cyan trail
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_SWORD_ARC_D, PAL_OBJ_KNIGHT, p);
    generate_sprite_16x16(SPRITE_SWORD_ARC_U, PAL_OBJ_KNIGHT, p);
    generate_sprite_16x16(SPRITE_SWORD_ARC_S, PAL_OBJ_KNIGHT, p);
}

static void generate_tool_and_monster_sprites(void) {
    u8 p[16][16];

    // Boomerang
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = 0;
            if ((x == y || x == 15 - y) && (x >= 4 && x <= 11)) c = 10; // Blue boomerang
            if ((x == y) && (x >= 6 && x <= 9)) c = 11;
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_BOOMERANG_0, PAL_OBJ_FIRE_BOMB, p);
    generate_sprite_16x16(SPRITE_BOOMERANG_1, PAL_OBJ_FIRE_BOMB, p);

    // Remote Bomb
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = 0;
            if ((x - 8) * (x - 8) + (y - 9) * (y - 9) <= 25) c = 2; // Black bomb body
            if (x >= 6 && x <= 7 && y >= 6 && y <= 8) c = 3;        // Highlight
            if (x == 8 && y >= 3 && y <= 5) c = 4;                  // Fuse stem
            if (x == 8 && y == 2) c = 6;                            // Spark
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_BOMB, PAL_OBJ_FIRE_BOMB, p);

    // Bomb Explosion (16x16 Expanding blast)
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = 0;
            int r2 = (x - 8) * (x - 8) + (y - 8) * (y - 8);
            if (r2 <= 49) c = 5; // Orange outer
            if (r2 <= 25) c = 6; // Yellow mid
            if (r2 <= 9) c = 7;  // White core
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_EXPLOSION_0, PAL_OBJ_FIRE_BOMB, p);
    generate_sprite_16x16(SPRITE_EXPLOSION_1, PAL_OBJ_FIRE_BOMB, p);
    generate_sprite_16x16(SPRITE_EXPLOSION_2, PAL_OBJ_FIRE_BOMB, p);

    // Fireball
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = 0;
            if ((x - 8) * (x - 8) + (y - 8) * (y - 8) <= 16) c = 5;
            if ((x - 8) * (x - 8) + (y - 8) * (y - 8) <= 4) c = 7;
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_FIREBALL_0, PAL_OBJ_FIRE_BOMB, p);
    generate_sprite_16x16(SPRITE_FIREBALL_1, PAL_OBJ_FIRE_BOMB, p);

    // Crypt Slime
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = 0;
            if (y >= 6 && y <= 14 && x >= 3 && x <= 12) c = 6; // Lime slime
            if (y >= 8 && y <= 12 && x >= 5 && x <= 10) c = 7; // Highlight
            if (y == 8 && (x == 5 || x == 9)) c = 1;           // Slime eyes
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_SLIME_0, PAL_OBJ_MONSTER, p);
    p[14][3] = 0; p[14][12] = 0;
    generate_sprite_16x16(SPRITE_SLIME_1, PAL_OBJ_MONSTER, p);

    // Skeletal Warrior
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = 0;
            // Skull
            if (y >= 2 && y <= 6 && x >= 5 && x <= 10) c = 3; // White bone
            if (y == 4 && (x == 6 || x == 9)) c = 4;           // Glowing red eyes
            // Ribcage
            if (y >= 7 && y <= 10 && x >= 5 && x <= 10) c = (y % 2 == 1) ? 3 : 2;
            // Rusty broadsword
            if (y >= 6 && y <= 13 && (x == 12 || x == 13)) c = 12;
            // Legs
            if (y >= 11 && y <= 14 && (x == 6 || x == 9)) c = 3;
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_SKELETON_D_0, PAL_OBJ_MONSTER, p);
    p[14][6] = 0;
    generate_sprite_16x16(SPRITE_SKELETON_D_1, PAL_OBJ_MONSTER, p);
    generate_sprite_16x16(SPRITE_SKELETON_S_0, PAL_OBJ_MONSTER, p);
    generate_sprite_16x16(SPRITE_SKELETON_S_1, PAL_OBJ_MONSTER, p);

    // Crypt Bat
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = 0;
            if (y >= 6 && y <= 10 && x >= 6 && x <= 9) c = 8; // Body
            if (y == 7 && (x == 7 || x == 8)) c = 4;          // Red eyes
            if (y >= 5 && y <= 9 && (x <= 5 || x >= 10)) c = 9; // Purple wings
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_BAT_0, PAL_OBJ_MONSTER, p);
    p[5][2] = 0; p[5][13] = 0;
    generate_sprite_16x16(SPRITE_BAT_1, PAL_OBJ_MONSTER, p);

    // Spike Blade Trap
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = 11; // Dark iron block
            if (x >= 4 && x <= 11 && y >= 4 && y <= 11) c = 12;
            if (x == 0 || x == 15 || y == 0 || y == 15) {
                if ((x + y) % 3 == 0) c = 13; // Sharp spike tips
            }
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_BLADE_TRAP, PAL_OBJ_MONSTER, p);

    // Ancient Crypt Golem (32x32 multi-part: 4 16x16 quadrants)
    for (int q = 0; q < 4; q++) {
        for (int y = 0; y < 16; y++) {
            for (int x = 0; x < 16; x++) {
                u8 c = 3; // Granite
                if ((x + y) % 4 == 0) c = 6; // Moss
                if (x == 0 || x == 15 || y == 0 || y == 15) c = 2; // Shadow outline
                p[y][x] = c;
            }
        }
        generate_sprite_16x16(SPRITE_GOLEM_TL + q * 4, PAL_OBJ_GOLEM, p);
    }

    // Golem Stone Fist
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = 0;
            if ((x - 8) * (x - 8) + (y - 8) * (y - 8) <= 36) c = 3;
            if (x >= 6 && x <= 10 && y >= 6 && y <= 10) c = 4;
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_GOLEM_FIST, PAL_OBJ_GOLEM, p);

    // Golem Core Eye
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = 0;
            if ((x - 8) * (x - 8) + (y - 8) * (y - 8) <= 25) c = 7; // Ruby
            if ((x - 8) * (x - 8) + (y - 8) * (y - 8) <= 9) c = 8;  // Bright ruby
            if (x == 8 && y == 8) c = 10; // White beam
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_GOLEM_CORE_EYE, PAL_OBJ_GOLEM, p);

    // Falling Stalactite
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = 0;
            int width = (16 - y) / 2;
            if (x >= 8 - width && x <= 8 + width) c = 3;
            if (x == 8 && y == 15) c = 4; // Sharp tip
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_STALACTITE, PAL_OBJ_GOLEM, p);

    // Pickups (Hearts, Keys, Coins, Boss Key)
    // Small Heart Pickup
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = 0;
            if (y >= 4 && y <= 11 && x >= 4 && x <= 11) c = 2; // Red heart
            if (y >= 5 && y <= 7 && x >= 5 && x <= 7) c = 3;   // Shine
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_HEART_PICKUP, PAL_OBJ_ITEMS, p);

    // Gold Coin Pickup
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = 0;
            if ((x - 8) * (x - 8) + (y - 8) * (y - 8) <= 16) c = 5;
            if (x == 8 && y == 8) c = 6;
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_COIN_PICKUP, PAL_OBJ_ITEMS, p);

    // Small Dungeon Key
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = 0;
            if ((x - 8) * (x - 8) + (y - 5) * (y - 5) <= 6) c = 5;
            if (x == 8 && y >= 6 && y <= 13) c = 5;
            if (y == 10 && x == 10) c = 5;
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_KEY_PICKUP, PAL_OBJ_ITEMS, p);

    // Big Boss Key
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = 0;
            if ((x - 8) * (x - 8) + (y - 5) * (y - 5) <= 9) c = 9; // Amethyst gem
            if (x == 8 && y >= 6 && y <= 14) c = 5; // Gold shank
            if ((y == 11 || y == 13) && x >= 8 && x <= 11) c = 5; // Double teeth
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_BOSS_KEY_PICKUP, PAL_OBJ_ITEMS, p);

    // Heart Container (Victory Trophy)
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = 0;
            if (y >= 2 && y <= 13 && x >= 2 && x <= 13) c = 2; // Big red heart
            if (x >= 4 && x <= 7 && y >= 4 && y <= 7) c = 3;   // Glossy highlight
            if (x == 2 || x == 13 || y == 2 || y == 13) c = 5; // Golden filigree border
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_HEART_CONTAINER, PAL_OBJ_ITEMS, p);

    // NPC Lorekeeper
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = 0;
            if (y >= 2 && y <= 5 && x >= 5 && x <= 10) c = 6; // Face
            if (y >= 6 && y <= 9 && x >= 5 && x <= 10) c = 5; // White beard
            if (y >= 6 && y <= 14 && x >= 3 && x <= 12) c = 3; // Brown traveler cloak
            if (y >= 3 && y <= 14 && x == 13) c = 8;          // Gold capped staff
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_NPC_LOREKEEPER, PAL_OBJ_NPC, p);

    // Smoke Puff
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = 0;
            if ((x - 8) * (x - 8) + (y - 8) * (y - 8) <= 16) c = 9;
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_SMOKE_PUFF, PAL_OBJ_FIRE_BOMB, p);
}

void assets_load_tiles(void) {
    // 1. Clear tile 0 (transparent) and solid black/white
    u8 empty[32];
    for (int i = 0; i < 32; i++) empty[i] = 0;
    upload_8x8_tile(TILE_EMPTY, empty);

    u8 black[32];
    for (int i = 0; i < 32; i++) black[i] = 0x11;
    upload_8x8_tile(TILE_SOLID_BLACK, black);

    u8 white[32];
    for (int i = 0; i < 32; i++) white[i] = 0x44;
    upload_8x8_tile(TILE_SOLID_WHITE, white);

    // 2. Build 5x7 ASCII font glyphs
    for (int i = 0; i < 10; i++) {
        build_font_tile(TILE_DIGIT_0 + i, s_font_glyphs[i]);
    }
    for (int i = 0; i < 26; i++) {
        build_font_tile(TILE_LETTER_A + i, s_font_glyphs[10 + i]);
    }
    build_font_tile(TILE_COLON, s_font_glyphs[36]);
    build_font_tile(TILE_MINUS, s_font_glyphs[37]);
    build_font_tile(TILE_DOT,   s_font_glyphs[38]);

    // 3. Generate UI dialog boxes and HUD hearts
    generate_ui_box_tiles();
    generate_hud_tiles();

    // 4. Generate 16x16 terrain metatiles for BG2 and arches for BG1
    generate_terrain_metatiles();

    // 5. Generate 16x16 hardware sprites for OBJ VRAM
    generate_knight_sprites();
    generate_tool_and_monster_sprites();
}
