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

static const u16 s_bg_palettes[5][16] = {
    // 0: PAL_BG_SCOREBOARD
    {
        RGB15(0, 0, 0),       // 0: Transparent
        RGB15(2, 3, 6),       // 1: Dark Navy Window Frame
        RGB15(4, 7, 14),      // 2: Window Fill
        RGB15(8, 14, 24),     // 3: Window Bevel
        RGB15(31, 31, 31),    // 4: Crisp White Text
        RGB15(16, 16, 18),    // 5: Dim Gray
        RGB15(31, 24, 4),     // 6: Gold Status / Radar
        RGB15(28, 4, 4),      // 7: Crimson Accent
        RGB15(4, 28, 8),      // 8: Green In Call
        RGB15(30, 24, 2),     // 9: Trophy Amber
        RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0)
    },
    // 1: PAL_BG_STADIUM (Crowd, stands, net posts, mesh)
    {
        RGB15(0, 0, 0),       // 0: Transparent
        RGB15(8, 8, 10),      // 1: Grandstand Concrete Shadow
        RGB15(14, 14, 16),    // 2: Concrete Mid
        RGB15(20, 20, 22),    // 3: Concrete Light
        RGB15(30, 22, 16),    // 4: Crowd Skin Tone
        RGB15(26, 4, 4),      // 5: Crowd Red Shirt
        RGB15(4, 12, 28),     // 6: Crowd Blue Shirt / Sponsor Navy
        RGB15(30, 26, 4),     // 7: Crowd Yellow Shirt
        RGB15(6, 18, 8),      // 8: Crowd Green Shirt
        RGB15(31, 31, 31),    // 9: Net Top Tape White
        RGB15(4, 4, 5),       // 10: Net Mesh Dark Cord
        RGB15(18, 18, 20),    // 11: Net Post Steel
        RGB15(10, 6, 2),      // 12: Umpire Chair Wood
        RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0)
    },
    // 2: PAL_BG_GRASS (Wimbledon emerald lawn)
    {
        RGB15(0, 0, 0),       // 0: Transparent
        RGB15(2, 10, 4),      // 1: Dark Grass Shadow
        RGB15(4, 18, 6),      // 2: Grass Mid Green
        RGB15(8, 24, 8),      // 3: Grass Light Stripe
        RGB15(31, 31, 31),    // 4: Crisp White Lines
        RGB15(12, 28, 12),    // 5: Grass Sun Highlight
        RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0)
    },
    // 3: PAL_BG_CLAY (Roland Garros terracotta red clay)
    {
        RGB15(0, 0, 0),       // 0: Transparent
        RGB15(14, 5, 2),      // 1: Dark Terracotta
        RGB15(24, 10, 4),     // 2: Red Clay Mid
        RGB15(28, 14, 6),     // 3: Light Brick Dust
        RGB15(31, 31, 31),    // 4: Crisp White Lines
        RGB15(31, 18, 8),     // 5: Clay Sun Highlight
        RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0)
    },
    // 4: PAL_BG_HARD (US Open electric cobalt blue court)
    {
        RGB15(0, 0, 0),       // 0: Transparent
        RGB15(2, 8, 20),      // 1: Deep Navy Court
        RGB15(4, 14, 28),     // 2: Cobalt Blue Surface
        RGB15(8, 20, 31),     // 3: Outer Cyan Runoff
        RGB15(31, 31, 31),    // 4: Crisp White Lines
        RGB15(14, 26, 31),    // 5: Surface Specular
        RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0)
    }
};

static const u16 s_obj_palettes[5][16] = {
    // 0: PAL_OBJ_PLAYER (Near player: white polo, navy shorts, gold racket)
    {
        RGB15(0, 0, 0),       // 0: Transparent
        RGB15(4, 5, 8),       // 1: Soft Dark Navy Outline
        RGB15(16, 17, 20),    // 2: Polo Shadow
        RGB15(31, 31, 31),    // 3: Polo Crisp White
        RGB15(3, 5, 16),      // 4: Navy Shorts
        RGB15(7, 14, 28),     // 5: Shorts Highlight
        RGB15(22, 14, 8),     // 6: Skin Shadow
        RGB15(30, 22, 16),    // 7: Skin Light
        RGB15(12, 7, 3),      // 8: Hair Brown
        RGB15(20, 12, 4),     // 9: Hair Highlight
        RGB15(28, 22, 4),     // 10: Racket Frame Gold
        RGB15(18, 18, 20),    // 11: Racket Strings
        RGB15(31, 31, 31),    // 12: Shoes White
        RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0)
    },
    // 1: PAL_OBJ_OPPONENT (Far opponent: crimson polo, slate shorts)
    {
        RGB15(0, 0, 0),       // 0: Transparent
        RGB15(5, 3, 3),       // 1: Soft Charcoal Outline
        RGB15(14, 2, 2),      // 2: Crimson Shadow
        RGB15(28, 4, 4),      // 3: Crimson Polo Bright
        RGB15(5, 5, 7),       // 4: Dark Slate Shorts
        RGB15(10, 10, 12),    // 5: Shorts Highlight
        RGB15(20, 12, 8),     // 6: Skin Shadow
        RGB15(28, 20, 14),    // 7: Skin Light
        RGB15(4, 4, 4),       // 8: Dark Hair
        RGB15(20, 20, 22),    // 9: Silver Racket Frame
        RGB15(14, 14, 16),    // 10: Racket Strings
        RGB15(26, 26, 26),    // 11: Shoes
        RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0)
    },
    // 2: PAL_OBJ_BALL (Optic tennis yellow ball, drop shadow, chalk puff)
    {
        RGB15(0, 0, 0),       // 0: Transparent
        RGB15(6, 10, 3),      // 1: Ball Outline / Olive Shadow Core
        RGB15(18, 24, 2),     // 2: Ball Shading Yellow-Green
        RGB15(28, 31, 4),     // 3: Optic Yellow Ball Core
        RGB15(31, 31, 18),    // 4: Ball Felt Specular
        RGB15(31, 31, 31),    // 5: Ball Seam / Chalk Puff White
        RGB15(20, 20, 22),    // 6: Chalk Dust Gray
        RGB15(10, 16, 6),     // 7: Soft Ambient Ground Shadow
        RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0)
    },
    // 3: PAL_OBJ_VFX (Swing trails: red topspin, blue slice, yellow smash/ace)
    {
        RGB15(0, 0, 0),       // 0: Transparent
        RGB15(24, 2, 2),      // 1: Topspin Red Dark
        RGB15(31, 6, 6),      // 2: Topspin Red Bright
        RGB15(31, 16, 16),    // 3: Topspin White Trail
        RGB15(2, 10, 26),     // 4: Slice Blue Dark
        RGB15(6, 18, 31),     // 5: Slice Blue Bright
        RGB15(16, 26, 31),    // 6: Slice Cyan Trail
        RGB15(28, 20, 2),     // 7: Smash Yellow Dark
        RGB15(31, 28, 4),     // 8: Smash Yellow Bright
        RGB15(31, 31, 18),    // 9: Smash Flash White
        RGB15(16, 4, 24),     // 10: Target Ring Purple
        RGB15(26, 10, 31),    // 11: Target Ring Bright
        RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0)
    },
    // 4: PAL_OBJ_TROPHY (Grand Slam Trophy)
    {
        RGB15(0, 0, 0),       // 0: Transparent
        RGB15(14, 10, 2),     // 1: Gold Shadow
        RGB15(24, 18, 3),     // 2: Gold Mid
        RGB15(31, 26, 4),     // 3: Gold Bright
        RGB15(31, 31, 16),    // 4: Gold Specular
        RGB15(31, 31, 31),    // 5: Sparkle White
        RGB15(14, 14, 16),    // 6: Silver Cup Mid
        RGB15(24, 24, 26),    // 7: Silver Cup Bright
        RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0)
    }
};

void assets_load_palettes(void) {
    for (int p = 0; p < 5; p++) {
        for (int c = 0; c < 16; c++) {
            BG_PALETTE_RAM[p * 16 + c] = s_bg_palettes[p][c];
            OBJ_PALETTE_RAM[p * 16 + c] = s_obj_palettes[p][c];
        }
    }
}

// =========================================================================
// 8x8 Tile Procedural Rasterizers
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
    for (int i = 0; i < 16; i++) dest[i] = src[i];
}

static void upload_sprite_tile(u16 tile_index, const u8* data) {
    vu16* dest = (vu16*)(VRAM_BASE + 0x10000 + (tile_index * 32));
    const u16* src = (const u16*)data;
    for (int i = 0; i < 16; i++) dest[i] = src[i];
}

// 5x7 ASCII font glyphs
static const u8 s_font_glyphs[38][5] = {
    // 0..9
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
    // A..Z
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
    // : -
    {0x00, 0x36, 0x36, 0x00, 0x00}, // :
    {0x08, 0x08, 0x08, 0x08, 0x08}  // -
};

static void build_font_tile(u16 tile_id, const u8 glyph[5]) {
    u8 buf[32];
    for (int i = 0; i < 32; i++) buf[i] = 0;

    for (int col = 0; col < 5; col++) {
        u8 bits = glyph[col];
        for (int row = 0; row < 7; row++) {
            if (bits & (1 << row)) {
                set_tile_pixel(buf, col + 1, row + 1, 1); // Crisp 1px drop shadow
                set_tile_pixel(buf, col + 1, row, 4);     // Crisp white
            }
        }
    }
    upload_8x8_tile(tile_id, buf);
}

static void generate_ui_and_stadium_tiles(void) {
    u8 buf[32];

    // Scoreboard Dialog Box Frames
    for (int i = 0; i < 32; i++) buf[i] = 0;
    // TILE_BOX_TL
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            u8 c = 2; // Fill
            if (x == 0 || y == 0) c = 3; // Light bevel
            if (x == 1 || y == 1) c = 1; // Dark border
            set_tile_pixel(buf, x, y, c);
        }
    }
    upload_8x8_tile(TILE_BOX_TL, buf);

    // TILE_BOX_TOP
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            u8 c = (y == 0) ? 3 : ((y == 1) ? 1 : 2);
            set_tile_pixel(buf, x, y, c);
        }
    }
    upload_8x8_tile(TILE_BOX_TOP, buf);

    // TILE_BOX_TR
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
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            u8 c = (x == 0) ? 3 : ((x == 1) ? 1 : 2);
            set_tile_pixel(buf, x, y, c);
        }
    }
    upload_8x8_tile(TILE_BOX_LEFT, buf);

    // TILE_BOX_RIGHT
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            u8 c = (x == 7) ? 1 : ((x == 6) ? 3 : 2);
            set_tile_pixel(buf, x, y, c);
        }
    }
    upload_8x8_tile(TILE_BOX_RIGHT, buf);

    // TILE_BOX_BL
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
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            u8 c = (y == 7) ? 1 : ((y == 6) ? 3 : 2);
            set_tile_pixel(buf, x, y, c);
        }
    }
    upload_8x8_tile(TILE_BOX_BOTTOM, buf);

    // TILE_BOX_BR
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

    // TILE_BALL_ICON
    for (int i = 0; i < 32; i++) buf[i] = 0;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if ((x - 4) * (x - 4) + (y - 4) * (y - 4) <= 9) set_tile_pixel(buf, x, y, 6);
        }
    }
    upload_8x8_tile(TILE_BALL_ICON, buf);

    // TILE_RADAR_ICON
    for (int i = 0; i < 32; i++) buf[i] = 0;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if (x == y || x == 7 - y) set_tile_pixel(buf, x, y, 6);
        }
    }
    upload_8x8_tile(TILE_RADAR_ICON, buf);

    // Stadium Crowd (Animated rows: TILE_CROWD_0 and TILE_CROWD_1)
    for (int f = 0; f < 2; f++) {
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                u8 c = 2; // Concrete
                if (y <= 2) {
                    // Spectator heads
                    if ((x + f) % 4 == 1 || (x + f) % 4 == 2) c = 4; // Face skin
                } else if (y <= 5) {
                    // Spectator shirts
                    if ((x + f) % 4 == 1) c = 5; // Red
                    if ((x + f) % 4 == 2) c = 6; // Blue
                }
                set_tile_pixel(buf, x, y, c);
            }
        }
        upload_8x8_tile(TILE_CROWD_0 + f, buf);
    }

    // Stadium Wall & Sponsor Banner
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            set_tile_pixel(buf, x, y, (y == 7) ? 1 : 2);
        }
    }
    upload_8x8_tile(TILE_STADIUM_WALL, buf);

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            u8 c = 6; // Sponsor navy
            if (y == 0 || y == 7) c = 3;
            if (y == 4 && (x == 2 || x == 5)) c = 9; // Text mark
            set_tile_pixel(buf, x, y, c);
        }
    }
    upload_8x8_tile(TILE_SPONSOR_BANNER, buf);

    // Net Posts (Left and Right)
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            u8 c = 0;
            if (x >= 3 && x <= 5) c = 11; // Steel post
            set_tile_pixel(buf, x, y, c);
        }
    }
    upload_8x8_tile(TILE_NET_POST_L, buf);
    upload_8x8_tile(TILE_NET_POST_R, buf);

    // Net Tape & Net Mesh (White tape on top row, criss-cross mesh below)
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            u8 c = 0;
            if (y == 0 || y == 1) {
                c = 9; // White tape
            } else {
                if ((x + y) % 2 == 0) c = 10; // Dark cord mesh
            }
            set_tile_pixel(buf, x, y, c);
        }
    }
    upload_8x8_tile(TILE_NET_MESH, buf);
    upload_8x8_tile(TILE_NET_TAPE, buf);

    // Umpire Chair Top & Bottom
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            u8 c = 0;
            if (x >= 2 && x <= 6 && y >= 2 && y <= 6) c = 12; // Chair seat
            set_tile_pixel(buf, x, y, c);
        }
    }
    upload_8x8_tile(TILE_UMPIRE_CHAIR_T, buf);

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            u8 c = 0;
            if (x == 2 || x == 6) c = 12; // Ladder legs
            if (y == 4 && x >= 2 && x <= 6) c = 12; // Ladder rung
            set_tile_pixel(buf, x, y, c);
        }
    }
    upload_8x8_tile(TILE_UMPIRE_CHAIR_B, buf);

    // Court Surface & Lines
    for (int i = 0; i < 32; i++) buf[i] = 0x22; // Plain court fill
    upload_8x8_tile(TILE_COURT_SURFACE, buf);

    // Horizontal White Line
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            u8 c = (y == 3 || y == 4) ? 4 : 2; // White line
            set_tile_pixel(buf, x, y, c);
        }
    }
    upload_8x8_tile(TILE_COURT_LINE_H, buf);

    // Vertical Center Line
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            u8 c = (x == 3 || x == 4) ? 4 : 2;
            set_tile_pixel(buf, x, y, c);
        }
    }
    upload_8x8_tile(TILE_COURT_LINE_V, buf);

    // Slanted Sideline Left
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            u8 c = (x == 7 - y) ? 4 : 2;
            set_tile_pixel(buf, x, y, c);
        }
    }
    upload_8x8_tile(TILE_COURT_LINE_SL_L, buf);

    // Slanted Sideline Right
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            u8 c = (x == y) ? 4 : 2;
            set_tile_pixel(buf, x, y, c);
        }
    }
    upload_8x8_tile(TILE_COURT_LINE_SL_R, buf);

    // Corners & T marks
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            u8 c = (x == 4 || y == 4) ? 4 : 2;
            set_tile_pixel(buf, x, y, c);
        }
    }
    upload_8x8_tile(TILE_COURT_T_MARK, buf);
    upload_8x8_tile(TILE_COURT_CORNER_TL, buf);
    upload_8x8_tile(TILE_COURT_CORNER_TR, buf);
    upload_8x8_tile(TILE_COURT_CORNER_BL, buf);
    upload_8x8_tile(TILE_COURT_CORNER_BR, buf);

    // =========================================================================
    // Continuous Perspective Sidelines (Clean 1.5-2px line segments)
    // =========================================================================
    // Left Sideline Top (Tile 80: connects x=6..7 at y=0 down to x=4..5 at y=7)
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            int lx = 6 - (y * 2) / 7;
            u8 c = (x >= lx && x <= lx + 1) ? 4 : 2;
            set_tile_pixel(buf, x, y, c);
        }
    }
    upload_8x8_tile(TILE_COURT_SL_L_TOP, buf);

    // Left Sideline Mid (Tile 81: connects x=4..5 at y=0 down to x=2..3 at y=7)
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            int lx = 4 - (y * 2) / 7;
            u8 c = (x >= lx && x <= lx + 1) ? 4 : 2;
            set_tile_pixel(buf, x, y, c);
        }
    }
    upload_8x8_tile(TILE_COURT_SL_L_MID, buf);

    // Left Sideline Bot (Tile 82: connects x=2..3 at y=0 down to x=0..1 at y=7)
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            int lx = 2 - (y * 2) / 7;
            u8 c = (x >= lx && x <= lx + 1) ? 4 : 2;
            set_tile_pixel(buf, x, y, c);
        }
    }
    upload_8x8_tile(TILE_COURT_SL_L_BOT, buf);

    // Right Sideline Top (Tile 83: connects x=0..1 at y=0 up to x=2..3 at y=7)
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            int rx = (y * 2) / 7;
            u8 c = (x >= rx && x <= rx + 1) ? 4 : 2;
            set_tile_pixel(buf, x, y, c);
        }
    }
    upload_8x8_tile(TILE_COURT_SL_R_TOP, buf);

    // Right Sideline Mid (Tile 84: connects x=2..3 at y=0 up to x=4..5 at y=7)
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            int rx = 2 + (y * 2) / 7;
            u8 c = (x >= rx && x <= rx + 1) ? 4 : 2;
            set_tile_pixel(buf, x, y, c);
        }
    }
    upload_8x8_tile(TILE_COURT_SL_R_MID, buf);

    // Right Sideline Bot (Tile 85: connects x=4..5 at y=0 up to x=6..7 at y=7)
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            int rx = 4 + (y * 2) / 7;
            u8 c = (x >= rx && x <= rx + 1) ? 4 : 2;
            set_tile_pixel(buf, x, y, c);
        }
    }
    upload_8x8_tile(TILE_COURT_SL_R_BOT, buf);

    // =========================================================================
    // Stadium Roof Canopy & Side Audience Stands (PAL_BG_STADIUM)
    // =========================================================================
    // TILE_STADIUM_ROOF (Tile 86)
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            u8 c = 1; // Deep concrete shadow
            if (y <= 1) c = 3; // Roof fascia edge
            else if (y == 2) c = 11; // Steel truss beam
            else if (x == y || x == 7 - y || x == 3 || x == 4) c = 11; // Truss girder
            set_tile_pixel(buf, x, y, c);
        }
    }
    upload_8x8_tile(TILE_STADIUM_ROOF, buf);

    // TILE_CROWD_SIDE_L0 (Tile 87): Left Side Grandstand - Frame 0
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            u8 c = 2; // Concrete bleacher
            if (y == 0) c = 1; // Shadow
            else if (y == 1) c = 3; // Step highlight
            else if (y == 2 || y == 3) {
                if (x == 1 || x == 2 || x == 5 || x == 6) c = 4; // Face skin
            } else if (y >= 4 && y <= 6) {
                if (x >= 1 && x <= 3) c = 5; // Red shirt
                else if (x >= 4 && x <= 7) c = 6; // Blue shirt
            } else if (y == 7) {
                c = 1;
            }
            set_tile_pixel(buf, x, y, c);
        }
    }
    upload_8x8_tile(TILE_CROWD_SIDE_L0, buf);

    // TILE_CROWD_SIDE_L1 (Tile 88): Left Side Grandstand - Frame 1
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            u8 c = 2;
            if (y == 0) c = 1;
            else if (y == 1) c = 3;
            else if (y == 2 || y == 3) {
                if (x == 0 || x == 1 || x == 4 || x == 5) c = 4;
            } else if (y >= 4 && y <= 6) {
                if (x <= 3) c = 7; // Yellow shirt
                else c = 8; // Green shirt
            } else if (y == 7) {
                c = 1;
            }
            set_tile_pixel(buf, x, y, c);
        }
    }
    upload_8x8_tile(TILE_CROWD_SIDE_L1, buf);

    // TILE_CROWD_SIDE_R0 (Tile 89): Right Side Grandstand - Frame 0
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            u8 c = 2;
            if (y == 0) c = 1;
            else if (y == 1) c = 3;
            else if (y == 2 || y == 3) {
                if (x == 1 || x == 2 || x == 5 || x == 6) c = 4;
            } else if (y >= 4 && y <= 6) {
                if (x >= 1 && x <= 3) c = 6; // Blue shirt
                else if (x >= 4 && x <= 7) c = 7; // Yellow shirt
            } else if (y == 7) {
                c = 1;
            }
            set_tile_pixel(buf, x, y, c);
        }
    }
    upload_8x8_tile(TILE_CROWD_SIDE_R0, buf);

    // TILE_CROWD_SIDE_R1 (Tile 90): Right Side Grandstand - Frame 1
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            u8 c = 2;
            if (y == 0) c = 1;
            else if (y == 1) c = 3;
            else if (y == 2 || y == 3) {
                if (x == 2 || x == 3 || x == 6 || x == 7) c = 4;
            } else if (y >= 4 && y <= 6) {
                if (x <= 3) c = 8; // Green shirt
                else c = 5; // Red shirt
            } else if (y == 7) {
                c = 1;
            }
            set_tile_pixel(buf, x, y, c);
        }
    }
    upload_8x8_tile(TILE_CROWD_SIDE_R1, buf);
}

// =========================================================================
// 16x16 Hardware Sprite Generators for OBJ VRAM
// =========================================================================

static void generate_sprite_16x16(u16 base_tile, const u8 pixels[16][16]) {
    u8 tile_buf[32];

    // Top-Left (0..7, 0..7)
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) set_tile_pixel(tile_buf, x, y, pixels[y][x]);
    }
    upload_sprite_tile(base_tile + 0, tile_buf);

    // Top-Right (8..15, 0..7)
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) set_tile_pixel(tile_buf, x, y, pixels[y][x + 8]);
    }
    upload_sprite_tile(base_tile + 1, tile_buf);

    // Bottom-Left (0..7, 8..15)
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) set_tile_pixel(tile_buf, x, y, pixels[y + 8][x]);
    }
    upload_sprite_tile(base_tile + 2, tile_buf);

    // Bottom-Right (8..15, 8..15)
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) set_tile_pixel(tile_buf, x, y, pixels[y + 8][x + 8]);
    }
    upload_sprite_tile(base_tile + 3, tile_buf);
}

static void make_sprite_from_art(u16 tile_index, const char* rows[16]) {
    u8 p[16][16];
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            char ch = rows[y][x];
            u8 val = 0;
            if (ch >= '0' && ch <= '9') val = ch - '0';
            else if (ch >= 'A' && ch <= 'F') val = 10 + (ch - 'A');
            else if (ch >= 'a' && ch <= 'f') val = 10 + (ch - 'a');
            p[y][x] = val;
        }
    }
    generate_sprite_16x16(tile_index, p);
}

static void generate_player_sprites(void) {
    // 1. Near Player: Ready Stance 0 (athletic back-3/4 angle, white polo, navy shorts, gold racket)
    static const char* s_p_ready_0[16] = {
        "......8888......",
        ".....889988.....",
        ".....877778.....",
        ".....167761.....",
        "....13333331....",
        "...1333333331...",
        "...1233333321.AA",
        "...1714444171A.A",
        "....14444441.A.A",
        "....14411441.AA.",
        "....167..761..1.",
        "....177..771....",
        "....1CC..CC1....",
        "....1C1..1C1....",
        "....111..111....",
        "................"
    };
    make_sprite_from_art(SPRITE_PLAYER_READY_0, s_p_ready_0);

    // Ready Stance 1 (subtle athletic knee bounce)
    static const char* s_p_ready_1[16] = {
        "................",
        "......8888......",
        ".....889988.....",
        ".....877778.....",
        "....13333331....",
        "...1333333331...",
        "...1233333321.AA",
        "...1714444171A.A",
        "....14444441.A.A",
        "....14411441.AA.",
        "....167..761..1.",
        "....177..771....",
        "....1CC..CC1....",
        "....1C1..1C1....",
        "....111..111....",
        "................"
    };
    make_sprite_from_art(SPRITE_PLAYER_READY_1, s_p_ready_1);

    // Run Left
    static const char* s_p_run_l[16] = {
        ".....8888.......",
        "....889988......",
        "....877778......",
        "...13333331.AA..",
        "..1333333331A..A",
        "..1233333321.AA.",
        "..1714444171.1..",
        "...14444441.....",
        "..1671..1671....",
        ".1771....1771...",
        ".1CC1.....1CC1..",
        ".111.......111..",
        "................",
        "................",
        "................",
        "................"
    };
    make_sprite_from_art(SPRITE_PLAYER_RUN_L, s_p_run_l);

    // Run Right
    static const char* s_p_run_r[16] = {
        ".......8888.....",
        "......889988....",
        "......877778....",
        "AA..13333331....",
        "A..A1333333331..",
        ".AA.1233333321..",
        "..1.1714444171..",
        ".....14444441...",
        "....1671..1671..",
        "...1771....1771.",
        "..1CC1.....1CC1.",
        "..111.......111.",
        "................",
        "................",
        "................",
        "................"
    };
    make_sprite_from_art(SPRITE_PLAYER_RUN_R, s_p_run_r);

    // Forehand Swing (Full follow-through across body)
    static const char* s_p_swing_fh[16] = {
        "..AA............",
        ".A..A...8888....",
        ".A..A..889988...",
        "..AA...877778...",
        "...1..13333331..",
        "...1.1333333331.",
        "....1733333321..",
        "....17144441....",
        ".....14444441...",
        "....1671..1671..",
        "...1771....1771.",
        "...1CC1.....1CC1",
        "...111.......111",
        "................",
        "................",
        "................"
    };
    make_sprite_from_art(SPRITE_PLAYER_SWING_FH_0, s_p_swing_fh);
    make_sprite_from_art(SPRITE_PLAYER_SWING_FH_1, s_p_swing_fh);

    // Backhand Swing
    static const char* s_p_swing_bh[16] = {
        "............AA..",
        "....8888...A..A.",
        "...889988..A..A.",
        "...877778...AA..",
        "..13333331..1...",
        ".1333333331.1...",
        "..1233333371....",
        "....14444171....",
        "...14444441.....",
        "..1671..1671....",
        ".1771....1771...",
        "1CC1.....1CC1...",
        "111.......111...",
        "................",
        "................",
        "................"
    };
    make_sprite_from_art(SPRITE_PLAYER_SWING_BH_0, s_p_swing_bh);
    make_sprite_from_art(SPRITE_PLAYER_SWING_BH_1, s_p_swing_bh);

    // Serve Toss (left arm high in air releasing ball, racket coiled)
    static const char* s_p_toss[16] = {
        "...7............",
        "...7....8888....",
        "...7...889988...",
        "...1...877778...",
        "....13333331....",
        "...1333333331...",
        "...1233333321...",
        "...1714444171.AA",
        "....14444441.A.A",
        "....14411441.A.A",
        "....167..761..AA",
        "....177..771..1.",
        "....1CC..CC1....",
        "....111..111....",
        "................",
        "................"
    };
    make_sprite_from_art(SPRITE_PLAYER_TOSS, s_p_toss);

    // Serve Hit (fully extended overhead smash)
    static const char* s_p_serve_hit[16] = {
        "......AAAA......",
        ".....A....A.....",
        ".....A.BB.A.....",
        "......AAAA......",
        ".......1........",
        ".......7........",
        "......8888......",
        ".....877778.....",
        "....13333331....",
        "...1333333331...",
        "....14444441....",
        "....14411441....",
        ".....17..71.....",
        ".....1C..C1.....",
        ".....11..11.....",
        "................"
    };
    make_sprite_from_art(SPRITE_PLAYER_SERVE_HIT, s_p_serve_hit);

    // Jump Smash (Airborne spike)
    static const char* s_p_smash[16] = {
        ".....AAAA.......",
        "....A....A......",
        "....A.BB.A......",
        ".....AAAA.......",
        "......17........",
        ".....8888.......",
        "....877778......",
        "...13333331.....",
        "..1333333331....",
        "...14444441.....",
        "....144441......",
        "....1671671.....",
        "...1771.1771....",
        "...1CC1..1CC1...",
        "...111....111...",
        "................"
    };
    make_sprite_from_art(SPRITE_PLAYER_SMASH, s_p_smash);

    // 2. Far Opponent Player (Front-facing, crimson polo, dark hair, black shorts)
    static const char* s_opp_ready_0[16] = {
        "......8888......",
        ".....888888.....",
        ".....777777.....",
        ".....177771.....",
        "....13333331....",
        "...1333333331.99",
        "...12333333219.9",
        "...17144441719.9",
        "....14444441..99",
        "....14411441..1.",
        "....167..761....",
        "....177..771....",
        "....1BB..BB1....",
        "....111..111....",
        "................",
        "................"
    };
    make_sprite_from_art(SPRITE_OPP_READY_0, s_opp_ready_0);

    static const char* s_opp_ready_1[16] = {
        "................",
        "......8888......",
        ".....888888.....",
        ".....777777.....",
        "....13333331....",
        "...1333333331.99",
        "...12333333219.9",
        "...17144441719.9",
        "....14444441..99",
        "....14411441..1.",
        "....167..761....",
        "....177..771....",
        "....1BB..BB1....",
        "....111..111....",
        "................",
        "................"
    };
    make_sprite_from_art(SPRITE_OPP_READY_1, s_opp_ready_1);

    static const char* s_opp_run_l[16] = {
        ".....8888.......",
        "....888888......",
        "....177771......",
        "...13333331.99..",
        "..13333333319..9",
        "..1233333321.99.",
        "..1714444171.1..",
        "...14444441.....",
        "..1671..1671....",
        ".1771....1771...",
        ".1BB1.....1BB1..",
        ".111.......111..",
        "................",
        "................",
        "................",
        "................"
    };
    make_sprite_from_art(SPRITE_OPP_RUN_L, s_opp_run_l);

    static const char* s_opp_run_r[16] = {
        ".......8888.....",
        "......888888....",
        "......177771....",
        "99..13333331....",
        "9..91333333331..",
        ".99.1233333321..",
        "..1.1714444171..",
        ".....14444441...",
        "....1671..1671..",
        "...1771....1771.",
        "..1BB1.....1BB1.",
        "..111.......111.",
        "................",
        "................",
        "................",
        "................"
    };
    make_sprite_from_art(SPRITE_OPP_RUN_R, s_opp_run_r);

    static const char* s_opp_swing_fh[16] = {
        "..99............",
        ".9..9...8888....",
        ".9..9..888888...",
        "..99...177771...",
        "...1..13333331..",
        "...1.1333333331.",
        "....1733333321..",
        "....17144441....",
        ".....14444441...",
        "....1671..1671..",
        "...1771....1771.",
        "...1BB1.....1BB1",
        "...111.......111",
        "................",
        "................",
        "................"
    };
    make_sprite_from_art(SPRITE_OPP_SWING_FH, s_opp_swing_fh);

    static const char* s_opp_swing_bh[16] = {
        "............99..",
        "....8888...9..9.",
        "...888888..9..9.",
        "...177771...99..",
        "..13333331..1...",
        ".1333333331.1...",
        "..1233333371....",
        "....14444171....",
        "...14444441.....",
        "..1671..1671....",
        ".1771....1771...",
        "1BB1.....1BB1...",
        "111.......111...",
        "................",
        "................",
        "................"
    };
    make_sprite_from_art(SPRITE_OPP_SWING_BH, s_opp_swing_bh);

    static const char* s_opp_serve[16] = {
        "......9999......",
        ".....9....9.....",
        ".....9....9.....",
        "......9999......",
        ".......1........",
        ".......7........",
        "......8888......",
        ".....177771.....",
        "....13333331....",
        "...1333333331...",
        "....14444441....",
        "....14411441....",
        ".....17..71.....",
        ".....1B..B1.....",
        ".....11..11.....",
        "................"
    };
    make_sprite_from_art(SPRITE_OPP_SERVE, s_opp_serve);

    // 3. Tennis Ball & Shadow
    static const char* s_ball_small[16] = {
        "................",
        "................",
        "................",
        "................",
        "................",
        ".......11.......",
        "......1341......",
        "......1231......",
        ".......11.......",
        "................",
        "................",
        "................",
        "................",
        "................",
        "................",
        "................"
    };
    make_sprite_from_art(SPRITE_BALL_SMALL, s_ball_small);

    static const char* s_ball_mid[16] = {
        "................",
        "................",
        "................",
        "................",
        "......1111......",
        ".....124421.....",
        "....12533521....",
        "....12355321....",
        ".....123321.....",
        "......1111......",
        "................",
        "................",
        "................",
        "................",
        "................",
        "................"
    };
    make_sprite_from_art(SPRITE_BALL_MID, s_ball_mid);

    static const char* s_ball_large[16] = {
        "................",
        "................",
        "................",
        ".....111111.....",
        "....12344321....",
        "...1253333521...",
        "...1235555321...",
        "...1235555321...",
        "...1253333521...",
        "....12333321....",
        ".....111111.....",
        "................",
        "................",
        "................",
        "................",
        "................"
    };
    make_sprite_from_art(SPRITE_BALL_LARGE, s_ball_large);

    static const char* s_ball_shadow[16] = {
        "................",
        "................",
        "................",
        "................",
        "................",
        "......7777......",
        "....77111177....",
        "...7111111117...",
        "...7111111117...",
        "....77111177....",
        "......7777......",
        "................",
        "................",
        "................",
        "................",
        "................"
    };
    make_sprite_from_art(SPRITE_BALL_SHADOW, s_ball_shadow);

    static const char* s_chalk_puff[16] = {
        "......5..5......",
        "....5.5555.5....",
        "...5556556555...",
        "..556655556655..",
        "....55555555....",
        "..5.56555565.5..",
        "....5.5555.5....",
        "......5..5......",
        "................",
        "................",
        "................",
        "................",
        "................",
        "................",
        "................",
        "................"
    };
    make_sprite_from_art(SPRITE_CHALK_PUFF, s_chalk_puff);

    // Dynamic Curved Swing Trails (Topspin Red, Slice Blue, Smash Yellow)
    static const char* s_swing_trail_red[16] = {
        "................",
        "...........33...",
        "........33222...",
        "......322211....",
        "....322211......",
        "..3222111.......",
        ".322111.........",
        ".2211...........",
        ".211............",
        ".11.............",
        "................",
        "................",
        "................",
        "................",
        "................",
        "................"
    };
    make_sprite_from_art(SPRITE_SWING_TRAIL_RED, s_swing_trail_red);

    static const char* s_swing_trail_blue[16] = {
        "................",
        "...........66...",
        "........66555...",
        "......655544....",
        "....655544......",
        "..6555444.......",
        ".655444.........",
        ".5544...........",
        ".544............",
        ".44.............",
        "................",
        "................",
        "................",
        "................",
        "................",
        "................"
    };
    make_sprite_from_art(SPRITE_SWING_TRAIL_BLUE, s_swing_trail_blue);

    static const char* s_swing_trail_yel[16] = {
        "................",
        "...........99...",
        "........99888...",
        "......988877....",
        "....988877......",
        "..9888777.......",
        ".988777.........",
        ".8877...........",
        ".877............",
        ".77.............",
        "................",
        "................",
        "................",
        "................",
        "................",
        "................"
    };
    make_sprite_from_art(SPRITE_SWING_TRAIL_YEL, s_swing_trail_yel);

    // Target Ring (Clean anti-aliased concentric ring)
    static const char* s_target_ring[16] = {
        "......aaaa......",
        "....aabbbbaa....",
        "...abb....bba...",
        "..ab........ba..",
        "..ab...bb...ba..",
        ".ab...bbbb...ba.",
        ".ab...bbbb...ba.",
        ".ab....bb....ba.",
        ".ab....bb....ba.",
        ".ab...bbbb...ba.",
        ".ab...bbbb...ba.",
        "..ab...bb...ba..",
        "..ab........ba..",
        "...abb....bba...",
        "....aabbbbaa....",
        "......aaaa......"
    };
    make_sprite_from_art(SPRITE_TARGET_RING, s_target_ring);

    // Championship Gold Trophy
    static const char* s_trophy_gold[16] = {
        "................",
        "....33333333....",
        "...3344444433...",
        "..33.454443.33..",
        "..33.444443.33..",
        "...33.4443.33...",
        "....33.43.33....",
        ".....332233.....",
        "......2222......",
        "......2222......",
        ".....222222.....",
        "....11111111....",
        "...1111111111...",
        "................",
        "................",
        "................"
    };
    make_sprite_from_art(SPRITE_TROPHY_GOLD, s_trophy_gold);
}

void assets_load_tiles(void) {
    u8 empty[32];
    for (int i = 0; i < 32; i++) empty[i] = 0;
    upload_8x8_tile(TILE_EMPTY, empty);

    u8 black[32];
    for (int i = 0; i < 32; i++) black[i] = 0x11;
    upload_8x8_tile(TILE_SOLID_BLACK, black);

    u8 white[32];
    for (int i = 0; i < 32; i++) white[i] = 0x44;
    upload_8x8_tile(TILE_SOLID_WHITE, white);

    // Font glyphs
    for (int i = 0; i < 10; i++) build_font_tile(TILE_DIGIT_0 + i, s_font_glyphs[i]);
    for (int i = 0; i < 26; i++) build_font_tile(TILE_LETTER_A + i, s_font_glyphs[10 + i]);
    build_font_tile(TILE_COLON, s_font_glyphs[36]);
    build_font_tile(TILE_MINUS, s_font_glyphs[37]);

    // UI, Stadium & Court Tiles
    generate_ui_and_stadium_tiles();

    // 16x16 Sprites
    generate_player_sprites();
}
