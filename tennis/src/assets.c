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
        RGB15(2, 2, 3),       // 1: Dark Outline
        RGB15(16, 17, 20),    // 2: Polo Shadow
        RGB15(31, 31, 31),    // 3: Polo Crisp White
        RGB15(2, 4, 14),      // 4: Navy Shorts
        RGB15(6, 12, 26),     // 5: Shorts Highlight
        RGB15(22, 14, 8),     // 6: Skin Shadow
        RGB15(30, 22, 16),    // 7: Skin Light
        RGB15(12, 7, 3),      // 8: Hair Brown
        RGB15(20, 12, 4),     // 9: Hair Highlight
        RGB15(28, 22, 4),     // 10: Racket Frame Gold
        RGB15(18, 18, 20),    // 11: Racket Strings
        RGB15(31, 31, 31),    // 12: Shoes White
        RGB15(0, 0, 0), RGB15(0, 0, 0), RGB15(0, 0, 0)
    },
    // 1: PAL_OBJ_OPPONENT (Far opponent: crimson polo, black shorts)
    {
        RGB15(0, 0, 0),       // 0: Transparent
        RGB15(2, 2, 2),       // 1: Outline
        RGB15(14, 2, 2),      // 2: Crimson Shadow
        RGB15(28, 4, 4),      // 3: Crimson Polo Bright
        RGB15(3, 3, 3),       // 4: Black Shorts
        RGB15(8, 8, 8),       // 5: Shorts Highlight
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
        RGB15(1, 2, 1),       // 1: Ball Outline / Court Shadow Dark
        RGB15(18, 24, 2),     // 2: Ball Shading Yellow-Green
        RGB15(28, 31, 4),     // 3: Optic Yellow Ball Core
        RGB15(31, 31, 20),    // 4: Ball Felt Specular
        RGB15(31, 31, 31),    // 5: Ball Seam / Chalk Puff White
        RGB15(20, 20, 22),    // 6: Chalk Dust Gray
        RGB15(2, 3, 2),       // 7: Translucent Ground Shadow
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
                set_tile_pixel(buf, col + 2, row + 1, 1); // Shadow
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

static void generate_player_sprites(void) {
    u8 p[16][16];

    // Player Ready Stance
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = 0;
            // Head & Hair
            if (y >= 1 && y <= 4 && x >= 6 && x <= 9) c = 8; // Brown hair
            if (y >= 3 && y <= 5 && x >= 6 && x <= 9) c = 7; // Face skin
            // White Polo Shirt
            if (y >= 6 && y <= 9 && x >= 5 && x <= 10) c = 3;
            // Navy Shorts
            if (y >= 10 && y <= 12 && x >= 5 && x <= 10) c = 4;
            // Legs & White Tennis Shoes
            if (y >= 13 && y <= 15 && (x == 6 || x == 9)) c = 12;
            // Racket held forward (gold hoop)
            if (y >= 6 && y <= 8 && (x == 11 || x == 12)) c = 10;
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_PLAYER_READY_0, p);
    p[15][6] = 0; // Stance bob
    generate_sprite_16x16(SPRITE_PLAYER_READY_1, p);

    // Run Left / Right
    p[14][5] = 12; p[14][10] = 0;
    generate_sprite_16x16(SPRITE_PLAYER_RUN_L, p);
    p[14][5] = 0; p[14][10] = 12;
    generate_sprite_16x16(SPRITE_PLAYER_RUN_R, p);

    // Forehand Swing
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = p[y][x];
            if (y >= 7 && y <= 11 && x >= 11 && x <= 15) c = 10; // Racket sweeping forward
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_PLAYER_SWING_FH_0, p);
    generate_sprite_16x16(SPRITE_PLAYER_SWING_FH_1, p);

    // Backhand Swing
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = p[y][x];
            if (y >= 7 && y <= 11 && x >= 0 && x <= 4) c = 10; // Sweeping to the left
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_PLAYER_SWING_BH_0, p);
    generate_sprite_16x16(SPRITE_PLAYER_SWING_BH_1, p);

    // Serve Toss & Hit
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = 0;
            if (y >= 3 && y <= 6 && x >= 7 && x <= 10) c = 7;
            if (y >= 7 && y <= 11 && x >= 6 && x <= 10) c = 3; // Reaching up
            if (y <= 2 && x >= 8 && x <= 10) c = 10; // Racket raised high overhead
            if (y >= 12 && y <= 15 && (x == 7 || x == 9)) c = 12;
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_PLAYER_TOSS, p);
    generate_sprite_16x16(SPRITE_PLAYER_SERVE_HIT, p);
    generate_sprite_16x16(SPRITE_PLAYER_SMASH, p);

    // Far Opponent Player (Scaled smaller for depth)
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = 0;
            if (y >= 5 && y <= 7 && x >= 7 && x <= 9) c = 7;   // Skin
            if (y >= 8 && y <= 10 && x >= 6 && x <= 10) c = 3; // Crimson shirt
            if (y >= 11 && y <= 12 && x >= 6 && x <= 10) c = 4; // Black shorts
            if (y >= 13 && y <= 14 && (x == 7 || x == 9)) c = 11;
            if (y >= 8 && y <= 10 && x == 11) c = 9; // Silver racket
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_OPP_READY_0, p);
    p[14][7] = 0;
    generate_sprite_16x16(SPRITE_OPP_READY_1, p);
    generate_sprite_16x16(SPRITE_OPP_RUN_L, p);
    generate_sprite_16x16(SPRITE_OPP_RUN_R, p);
    generate_sprite_16x16(SPRITE_OPP_SWING_FH, p);
    generate_sprite_16x16(SPRITE_OPP_SWING_BH, p);
    generate_sprite_16x16(SPRITE_OPP_SERVE, p);

    // Tennis Ball Sizes (Small, Mid, Large)
    // Small Ball (Far court / High altitude)
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = 0;
            if (x >= 7 && x <= 8 && y >= 7 && y <= 8) c = 3; // 2x2 dot
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_BALL_SMALL, p);

    // Mid Ball (Standard rally)
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = 0;
            if ((x - 8) * (x - 8) + (y - 8) * (y - 8) <= 4) c = 3;
            if (x == 8 && y == 8) c = 4; // Specular
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_BALL_MID, p);

    // Large Ball (Near baseline / Smash)
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = 0;
            if ((x - 8) * (x - 8) + (y - 8) * (y - 8) <= 9) c = 3;
            if ((x - 8) * (x - 8) + (y - 8) * (y - 8) <= 3) c = 4;
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_BALL_LARGE, p);

    // Ball Ground Drop Shadow (Translucent dark oval)
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = 0;
            if (y >= 7 && y <= 9 && x >= 5 && x <= 11) c = 7; // Shadow
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_BALL_SHADOW, p);

    // Chalk Puff Impact
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = 0;
            if ((x - 8) * (x - 8) + (y - 8) * (y - 8) <= 12) c = 5; // White chalk
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_CHALK_PUFF, p);

    // Swing Trails (Red Topspin, Blue Slice, Yellow Ace)
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = 0;
            if (y >= 6 && y <= 10 && x >= 2 && x <= 14) c = 2; // Red trail
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_SWING_TRAIL_RED, p);

    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = 0;
            if (y >= 6 && y <= 10 && x >= 2 && x <= 14) c = 5; // Blue trail
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_SWING_TRAIL_BLUE, p);

    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = 0;
            if (y >= 6 && y <= 10 && x >= 2 && x <= 14) c = 8; // Yellow trail
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_SWING_TRAIL_YEL, p);

    // Target Ring (Mini-Game Target)
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = 0;
            int r2 = (x - 8) * (x - 8) + (y - 8) * (y - 8);
            if (r2 <= 49 && r2 >= 25) c = 10; // Outer ring
            if (r2 <= 9) c = 11;             // Bullseye
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_TARGET_RING, p);

    // Grand Slam Gold Trophy
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            u8 c = 0;
            if (y >= 2 && y <= 7 && x >= 5 && x <= 10) c = 3; // Cup bowl
            if (y >= 4 && y <= 6 && (x == 3 || x == 12)) c = 3; // Handles
            if (y >= 8 && y <= 10 && (x == 7 || x == 8)) c = 2; // Stem
            if (y >= 11 && y <= 13 && x >= 4 && x <= 11) c = 1; // Base
            if (y == 3 && x == 6) c = 5; // Shine
            p[y][x] = c;
        }
    }
    generate_sprite_16x16(SPRITE_TROPHY_GOLD, p);
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
