#include "graphics.h"
#include "assets.h"

static OBJ_ATTR s_oam_buffer[MAX_SPRITES];

static vu16* const s_bg0_map = (vu16*)(VRAM_BASE + (BG0_SBB * 0x800));
static vu16* const s_bg1_map = (vu16*)(VRAM_BASE + (BG1_SBB * 0x800));
static vu16* const s_bg2_map = (vu16*)(VRAM_BASE + (BG2_SBB * 0x800));

static u8 s_shake_intensity = 0;
static u8 s_shake_frames = 0;
static s8 s_shake_x = 0;
static s8 s_shake_y = 0;

void graphics_init(void) {
    // Mode 0: BG0, BG1, BG2 active + 1D sprite mapping
    REG_DISPCNT = MODE_0 | BG0_ENABLE | BG1_ENABLE | BG2_ENABLE | OBJ_ENABLE | OBJ_1D_MAP;

    // BG0: Priority 0 (Scoreboard HUD & Announcements, 32x32)
    REG_BG0CNT = BG_CBB(BG_CBB_TILES) | BG_SBB(BG0_SBB) | BG_PRIORITY(0) | BG_SIZE_32x32 | BG_COLOR_16;
    REG_BG0HOFS = 0;
    REG_BG0VOFS = 0;

    // BG1: Priority 1 (Stadium Grandstands, Crowd & Net Mesh, 32x32)
    REG_BG1CNT = BG_CBB(BG_CBB_TILES) | BG_SBB(BG1_SBB) | BG_PRIORITY(1) | BG_SIZE_32x32 | BG_COLOR_16;
    REG_BG1HOFS = 0;
    REG_BG1VOFS = 0;

    // BG2: Priority 2 (Perspective Tennis Court, 32x32)
    REG_BG2CNT = BG_CBB(BG_CBB_TILES) | BG_SBB(BG2_SBB) | BG_PRIORITY(2) | BG_SIZE_32x32 | BG_COLOR_16;
    REG_BG2HOFS = 0;
    REG_BG2VOFS = 0;

    graphics_oam_hide_all();
    graphics_oam_copy();

    graphics_clear_bg0();
    graphics_clear_bg1();
    graphics_clear_bg2();
}

void graphics_wait_vblank(void) {
    while (REG_VCOUNT >= 160);
    while (REG_VCOUNT < 160);
}

void graphics_oam_hide_all(void) {
    for (int i = 0; i < MAX_SPRITES; i++) {
        s_oam_buffer[i].attr0 = 0x0200; // Offscreen disable
        s_oam_buffer[i].attr1 = 0x0000;
        s_oam_buffer[i].attr2 = 0x0000;
        s_oam_buffer[i].dummy = 0x0000;
    }
}

void graphics_oam_copy(void) {
    // DMA Channel 3: 128 sprites * 8 bytes = 1024 bytes (256 32-bit words)
    REG_DMA3SAD = (u32)s_oam_buffer;
    REG_DMA3DAD = (u32)OAM_BASE;
    REG_DMA3CNT = 256 | DMA_32 | DMA_ENABLE | DMA_IMMEDIATE;
}

void graphics_set_sprite(u8 index, s16 x, s16 y, u16 tile, u8 shape, u8 size, u8 pal, bool hflip, bool vflip) {
    if (index >= MAX_SPRITES) return;

    if (x <= -32 || x >= SCREEN_WIDTH || y <= -32 || y >= SCREEN_HEIGHT) {
        s_oam_buffer[index].attr0 = 0x0200; // Offscreen
        return;
    }

    u16 a0 = (y & 0x00FF);
    a0 |= ((shape & 0x03) << 14); // 0=square, 1=wide, 2=tall

    u16 a1 = (x & 0x01FF);
    if (hflip) a1 |= (1 << 12);
    if (vflip) a1 |= (1 << 13);
    a1 |= ((size & 0x03) << 14);

    u16 a2 = (tile & 0x03FF);
    a2 |= ((pal & 0x0F) << 12);
    a2 |= (1 << 10); // Priority 1 (above BG2 court, below BG0 HUD)

    s_oam_buffer[index].attr0 = a0;
    s_oam_buffer[index].attr1 = a1;
    s_oam_buffer[index].attr2 = a2;
}

void graphics_trigger_shake(u8 intensity, u8 frames) {
    s_shake_intensity = intensity;
    s_shake_frames = frames;
}

void graphics_update_shake(void) {
    if (s_shake_frames > 0) {
        s_shake_frames--;
        s8 mag = (s8)s_shake_intensity;
        s_shake_x = (s_shake_frames & 1) ? mag : -mag;
        s_shake_y = (s_shake_frames & 2) ? (mag >> 1) : -(mag >> 1);
    } else {
        s_shake_x = 0;
        s_shake_y = 0;
    }
    REG_BG1HOFS = s_shake_x;
    REG_BG1VOFS = s_shake_y;
    REG_BG2HOFS = s_shake_x;
    REG_BG2VOFS = s_shake_y;
}

void graphics_set_bg0_tile(u8 x, u8 y, u16 tile, u8 pal) {
    u16 entry = (tile & 0x03FF) | ((pal & 0x0F) << 12);
    s_bg0_map[(y % 32) * 32 + (x % 32)] = entry;
}

void graphics_set_bg1_tile(u8 x, u8 y, u16 tile, u8 pal) {
    u16 entry = (tile & 0x03FF) | ((pal & 0x0F) << 12);
    s_bg1_map[(y % 32) * 32 + (x % 32)] = entry;
}

void graphics_set_bg2_tile(u8 x, u8 y, u16 tile, u8 pal) {
    u16 entry = (tile & 0x03FF) | ((pal & 0x0F) << 12);
    s_bg2_map[(y % 32) * 32 + (x % 32)] = entry;
}

void graphics_clear_bg0(void) {
    for (int i = 0; i < 32 * 32; i++) s_bg0_map[i] = TILE_EMPTY;
}

void graphics_clear_bg1(void) {
    for (int i = 0; i < 32 * 32; i++) s_bg1_map[i] = TILE_EMPTY;
}

void graphics_clear_bg2(void) {
    for (int i = 0; i < 32 * 32; i++) s_bg2_map[i] = TILE_EMPTY;
}

void graphics_render_court(CourtSurface surface) {
    u8 court_pal = PAL_BG_GRASS;
    if (surface == SURFACE_CLAY) court_pal = PAL_BG_CLAY;
    if (surface == SURFACE_HARD) court_pal = PAL_BG_HARD;

    graphics_clear_bg1();
    graphics_clear_bg2();

    // 1. BG1: Stadium Grandstands & Audience
    // Row 0: Stadium Roof Canopy (cols 0..4 and 25..29) and Center Upper Bleachers
    for (u8 x = 0; x < 30; x++) {
        u16 roof = (x <= 4 || x >= 25) ? TILE_STADIUM_ROOF : TILE_CROWD_0;
        graphics_set_bg1_tile(x, 0, roof, PAL_BG_STADIUM);
    }

    // Rows 1..2: Grandstand Crowd Spectators across all 30 columns
    for (u8 y = 1; y < 3; y++) {
        for (u8 x = 0; x < 30; x++) {
            u16 crowd_tile = ((x + y) & 1) ? TILE_CROWD_0 : TILE_CROWD_1;
            graphics_set_bg1_tile(x, y, crowd_tile, PAL_BG_STADIUM);
        }
    }

    // Row 3: Stadium Concrete Wall & Sponsor Banners
    for (u8 x = 0; x < 30; x++) {
        u16 banner = (x >= 8 && x <= 21) ? TILE_SPONSOR_BANNER : TILE_STADIUM_WALL;
        graphics_set_bg1_tile(x, 3, banner, PAL_BG_STADIUM);
    }

    // Rows 4..19 on BG1: Side Audience Stands flanking the court
    for (u8 y = 4; y < 20; y++) {
        // Left Grandstand: cols 0..2 crowd, col 3 perimeter barrier
        for (u8 x = 0; x < 3; x++) {
            u16 l_crowd = ((x + y) & 1) ? TILE_CROWD_SIDE_L0 : TILE_CROWD_SIDE_L1;
            graphics_set_bg1_tile(x, y, l_crowd, PAL_BG_STADIUM);
        }
        graphics_set_bg1_tile(3, y, TILE_STADIUM_WALL, PAL_BG_STADIUM);

        // Right Grandstand: col 26 perimeter barrier, cols 27..29 crowd
        graphics_set_bg1_tile(26, y, TILE_STADIUM_WALL, PAL_BG_STADIUM);
        for (u8 x = 27; x < 30; x++) {
            u16 r_crowd = ((x + y) & 1) ? TILE_CROWD_SIDE_R0 : TILE_CROWD_SIDE_R1;
            graphics_set_bg1_tile(x, y, r_crowd, PAL_BG_STADIUM);
        }
    }

    // Net on Row 9 (Y=72..79) on BG1
    // Net posts at col 6 and 23
    graphics_set_bg1_tile(6, 9, TILE_NET_POST_L, PAL_BG_STADIUM);
    graphics_set_bg1_tile(23, 9, TILE_NET_POST_R, PAL_BG_STADIUM);
    // Net tape & mesh spanning cols 7..22
    for (u8 x = 7; x <= 22; x++) {
        graphics_set_bg1_tile(x, 9, TILE_NET_MESH, PAL_BG_STADIUM);
    }
    // Umpire chair on right at col 24, 25
    graphics_set_bg1_tile(24, 8, TILE_UMPIRE_CHAIR_T, PAL_BG_STADIUM);
    graphics_set_bg1_tile(24, 9, TILE_UMPIRE_CHAIR_B, PAL_BG_STADIUM);

    // 2. BG2: Court Surface & Lines in Rows 4..19 (cols 4..25 only, sides left empty for crowd)
    for (u8 y = 4; y < 20; y++) {
        for (u8 x = 4; x <= 25; x++) {
            graphics_set_bg2_tile(x, y, TILE_COURT_SURFACE, court_pal);
        }
    }

    // Far Baseline (Row 4): cols 9..20
    for (u8 x = 10; x <= 19; x++) {
        graphics_set_bg2_tile(x, 4, TILE_COURT_LINE_H, court_pal);
    }
    graphics_set_bg2_tile(9, 4, TILE_COURT_CORNER_TL, court_pal);
    graphics_set_bg2_tile(20, 4, TILE_COURT_CORNER_TR, court_pal);

    // Far Service Line (Row 7): cols 9..20
    for (u8 x = 9; x <= 20; x++) {
        graphics_set_bg2_tile(x, 7, TILE_COURT_LINE_H, court_pal);
    }

    // Near Service Line (Row 13): cols 7..22
    for (u8 x = 7; x <= 22; x++) {
        graphics_set_bg2_tile(x, 13, TILE_COURT_LINE_H, court_pal);
    }

    // Near Baseline (Row 17): cols 5..24
    for (u8 x = 6; x <= 23; x++) {
        graphics_set_bg2_tile(x, 17, TILE_COURT_LINE_H, court_pal);
    }
    graphics_set_bg2_tile(5, 17, TILE_COURT_CORNER_BL, court_pal);
    graphics_set_bg2_tile(24, 17, TILE_COURT_CORNER_BR, court_pal);

    // Center Service Line (Col 14, between rows 7 and 13)
    for (u8 y = 8; y <= 12; y++) {
        graphics_set_bg2_tile(14, y, TILE_COURT_LINE_V, court_pal);
    }
    graphics_set_bg2_tile(14, 7, TILE_COURT_T_MARK, court_pal);
    graphics_set_bg2_tile(14, 13, TILE_COURT_T_MARK, court_pal);

    // =========================================================================
    // Continuous Perspective Sidelines (Mathematically aligned, zero sawteeth)
    // =========================================================================
    // Left Sideline:
    graphics_set_bg2_tile(8, 5,  TILE_COURT_SL_L_TOP, court_pal);
    graphics_set_bg2_tile(8, 6,  TILE_COURT_SL_L_MID, court_pal);
    graphics_set_bg2_tile(8, 7,  TILE_COURT_SL_L_BOT, court_pal);
    graphics_set_bg2_tile(7, 8,  TILE_COURT_SL_L_TOP, court_pal);
    graphics_set_bg2_tile(7, 9,  TILE_COURT_SL_L_MID, court_pal);
    graphics_set_bg2_tile(7, 10, TILE_COURT_SL_L_BOT, court_pal);
    graphics_set_bg2_tile(6, 11, TILE_COURT_SL_L_TOP, court_pal);
    graphics_set_bg2_tile(6, 12, TILE_COURT_SL_L_MID, court_pal);
    graphics_set_bg2_tile(6, 13, TILE_COURT_SL_L_BOT, court_pal);
    graphics_set_bg2_tile(5, 14, TILE_COURT_SL_L_TOP, court_pal);
    graphics_set_bg2_tile(5, 15, TILE_COURT_SL_L_MID, court_pal);
    graphics_set_bg2_tile(5, 16, TILE_COURT_SL_L_BOT, court_pal);

    // Right Sideline:
    graphics_set_bg2_tile(21, 5,  TILE_COURT_SL_R_TOP, court_pal);
    graphics_set_bg2_tile(21, 6,  TILE_COURT_SL_R_MID, court_pal);
    graphics_set_bg2_tile(21, 7,  TILE_COURT_SL_R_BOT, court_pal);
    graphics_set_bg2_tile(22, 8,  TILE_COURT_SL_R_TOP, court_pal);
    graphics_set_bg2_tile(22, 9,  TILE_COURT_SL_R_MID, court_pal);
    graphics_set_bg2_tile(22, 10, TILE_COURT_SL_R_BOT, court_pal);
    graphics_set_bg2_tile(23, 11, TILE_COURT_SL_R_TOP, court_pal);
    graphics_set_bg2_tile(23, 12, TILE_COURT_SL_R_MID, court_pal);
    graphics_set_bg2_tile(23, 13, TILE_COURT_SL_R_BOT, court_pal);
    graphics_set_bg2_tile(24, 14, TILE_COURT_SL_R_TOP, court_pal);
    graphics_set_bg2_tile(24, 15, TILE_COURT_SL_R_MID, court_pal);
    graphics_set_bg2_tile(24, 16, TILE_COURT_SL_R_BOT, court_pal);
}

void graphics_animate_crowd(u32 frame, bool cheering) {
    u32 rate = cheering ? 12 : 24;
    if ((frame % rate) != 0) return;

    u8 phase = (u8)((frame / rate) & 1);

    // Animate top crowd in rows 1..2
    u16 tc0 = phase ? TILE_CROWD_1 : TILE_CROWD_0;
    u16 tc1 = phase ? TILE_CROWD_0 : TILE_CROWD_1;
    for (u8 y = 1; y < 3; y++) {
        for (u8 x = 0; x < 30; x++) {
            u16 tile = ((x + y) & 1) ? tc0 : tc1;
            graphics_set_bg1_tile(x, y, tile, PAL_BG_STADIUM);
        }
    }

    // Animate side crowd in rows 4..19
    u16 lc = phase ? TILE_CROWD_SIDE_L1 : TILE_CROWD_SIDE_L0;
    u16 rc = phase ? TILE_CROWD_SIDE_R1 : TILE_CROWD_SIDE_R0;
    for (u8 y = 4; y < 20; y++) {
        for (u8 x = 0; x < 3; x++) {
            graphics_set_bg1_tile(x, y, lc, PAL_BG_STADIUM);
        }
        for (u8 x = 27; x < 30; x++) {
            graphics_set_bg1_tile(x, y, rc, PAL_BG_STADIUM);
        }
    }
}

void graphics_print_text(u8 x, u8 y, const char* str, u8 pal) {
    while (*str) {
        char c = *str++;
        u16 tile = TILE_EMPTY;
        if (c >= '0' && c <= '9') {
            tile = TILE_DIGIT_0 + (c - '0');
        } else if (c >= 'A' && c <= 'Z') {
            tile = TILE_LETTER_A + (c - 'A');
        } else if (c >= 'a' && c <= 'z') {
            tile = TILE_LETTER_A + (c - 'a');
        } else if (c == ':') {
            tile = TILE_COLON;
        } else if (c == '-') {
            tile = TILE_MINUS;
        } else if (c == '.') {
            tile = TILE_DOT;
        } else if (c == '!') {
            tile = TILE_EXCLAMATION;
        } else if (c == '/') {
            tile = TILE_SLASH;
        } else if (c == '<') {
            tile = TILE_ARROW_L;
        } else if (c == '>') {
            tile = TILE_ARROW_R;
        } else if (c == '[') {
            tile = TILE_BRACKET_L;
        } else if (c == ']') {
            tile = TILE_BRACKET_R;
        } else {
            tile = TILE_EMPTY;
        }
        graphics_set_bg0_tile(x++, y, tile, pal);
    }
}

void graphics_print_num(u8 x, u8 y, u32 num, u8 digits, u8 pal) {
    char buf[12];
    for (int i = digits - 1; i >= 0; i--) {
        buf[i] = '0' + (num % 10);
        num /= 10;
    }
    buf[digits] = '\0';
    graphics_print_text(x, y, buf, pal);
}

void graphics_draw_box(u8 x, u8 y, u8 w, u8 h, u8 pal) {
    if (w < 2 || h < 2) return;

    graphics_set_bg0_tile(x, y, TILE_BOX_TL, pal);
    for (u8 c = 1; c < w - 1; c++) graphics_set_bg0_tile(x + c, y, TILE_BOX_TOP, pal);
    graphics_set_bg0_tile(x + w - 1, y, TILE_BOX_TR, pal);

    for (u8 r = 1; r < h - 1; r++) {
        graphics_set_bg0_tile(x, y + r, TILE_BOX_LEFT, pal);
        for (u8 c = 1; c < w - 1; c++) graphics_set_bg0_tile(x + c, y + r, TILE_BOX_FILL, pal);
        graphics_set_bg0_tile(x + w - 1, y + r, TILE_BOX_RIGHT, pal);
    }

    graphics_set_bg0_tile(x, y + h - 1, TILE_BOX_BL, pal);
    for (u8 c = 1; c < w - 1; c++) graphics_set_bg0_tile(x + c, y + h - 1, TILE_BOX_BOTTOM, pal);
    graphics_set_bg0_tile(x + w - 1, y + h - 1, TILE_BOX_BR, pal);
}
