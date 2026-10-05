#include "graphics.h"
#include "assets.h"

static OBJ_ATTR s_oam_buffer[MAX_SPRITES];

static vu16* const s_bg0_map = (vu16*)(VRAM_BASE + (BG0_SBB * 0x800));

// BG1 (64x32) uses SBB 28 (left 32x32 block) and SBB 29 (right 32x32 block)
static vu16* const s_bg1_map0 = (vu16*)(VRAM_BASE + (BG1_SBB * 0x800));
static vu16* const s_bg1_map1 = (vu16*)(VRAM_BASE + ((BG1_SBB + 1) * 0x800));

// BG2 (64x32) uses SBB 30 (left 32x32 block) and SBB 31 (right 32x32 block)
static vu16* const s_bg2_map0 = (vu16*)(VRAM_BASE + (BG2_SBB * 0x800));
static vu16* const s_bg2_map1 = (vu16*)(VRAM_BASE + ((BG2_SBB + 1) * 0x800));

static s16 s_cam_x = 0;
static s16 s_cam_y = 0;

static u8 s_shake_intensity = 0;
static u8 s_shake_frames = 0;
static s8 s_shake_x = 0;
static s8 s_shake_y = 0;

static u32 s_vblank_frame_count = 0;

void graphics_init(void) {
    // Mode 0: BG0, BG1, BG2 active + 1D sprite mapping
    REG_DISPCNT = MODE_0 | BG0_ENABLE | BG1_ENABLE | BG2_ENABLE | OBJ_ENABLE | OBJ_1D_MAP;

    // BG0: Priority 0 (Topmost UI / Menus / HUD, 32x32)
    REG_BG0CNT = BG_CBB(BG_CBB_TILES) | BG_SBB(BG0_SBB) | BG_PRIORITY(0) | BG_SIZE_32x32 | BG_COLOR_16;
    REG_BG0HOFS = 0;
    REG_BG0VOFS = 0;

    // BG1: Priority 1 (Interactive Metatiles: Walls, Dirt, Boulders, Gems, 64x32)
    REG_BG1CNT = BG_CBB(BG_CBB_TILES) | BG_SBB(BG1_SBB) | BG_PRIORITY(1) | BG_SIZE_64x32 | BG_COLOR_16;
    REG_BG1HOFS = 0;
    REG_BG1VOFS = 0;

    // BG2: Priority 2 (Ancient Temple Foundation Bedrock with Parallax, 64x32)
    REG_BG2CNT = BG_CBB(BG_CBB_TILES) | BG_SBB(BG2_SBB) | BG_PRIORITY(2) | BG_SIZE_64x32 | BG_COLOR_16;
    REG_BG2HOFS = 0;
    REG_BG2VOFS = 0;

    REG_BLDCNT = 0;
    REG_BLDALPHA = 0;

    graphics_oam_hide_all();
    graphics_oam_copy();

    graphics_clear_bg0();
    graphics_clear_bg1();
    graphics_clear_bg2();

    // Initialize Gameloft graphics and default Angkor Wat theme
    assets_init();
}

void graphics_wait_vblank(void) {
    while (REG_VCOUNT >= 160);
    while (REG_VCOUNT < 160);

    // Copy shadow OAM buffer during VBlank to prevent mid-frame tearing & flicker
    graphics_oam_copy();

    // Dynamic VBlank palette shimmer & animations
    assets_update_vblank_effects(s_vblank_frame_count++);
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
    REG_DMA3SAD = (u32)s_oam_buffer;
    REG_DMA3DAD = (u32)OAM_BASE;
    REG_DMA3CNT = 256 | DMA_32 | DMA_ENABLE | DMA_IMMEDIATE;
}

void graphics_set_sprite(u8 index, s16 x, s16 y, u16 tile, u8 shape, u8 size, u8 pal, bool hflip, bool vflip) {
    if (index >= MAX_SPRITES) return;

    if (x <= -32 || x >= SCREEN_WIDTH || y <= -32 || y >= SCREEN_HEIGHT) {
        s_oam_buffer[index].attr0 = 0x0200;
        return;
    }

    u16 a0 = (y & 0x00FF);
    a0 |= ((shape & 0x03) << 14); // 0=square (16x16 with size=1)

    u16 a1 = (x & 0x01FF);
    if (hflip) a1 |= (1 << 12);
    if (vflip) a1 |= (1 << 13);
    a1 |= ((size & 0x03) << 14);

    u16 a2 = (tile & 0x03FF);
    a2 |= ((pal & 0x0F) << 12);
    a2 |= (1 << 10); // Priority 1 (above BG1/BG2, below BG0 HUD)

    s_oam_buffer[index].attr0 = a0;
    s_oam_buffer[index].attr1 = a1;
    s_oam_buffer[index].attr2 = a2;
}

void graphics_set_camera(s16 cam_x, s16 cam_y) {
    // 384x256 map in 240x160 screen -> max scroll is 144x96
    if (cam_x < 0) cam_x = 0;
    if (cam_x > (LEVEL_WIDTH * TILE_SIZE - SCREEN_WIDTH)) {
        cam_x = LEVEL_WIDTH * TILE_SIZE - SCREEN_WIDTH;
    }
    if (cam_y < 0) cam_y = 0;
    if (cam_y > (LEVEL_HEIGHT * TILE_SIZE - SCREEN_HEIGHT)) {
        cam_y = LEVEL_HEIGHT * TILE_SIZE - SCREEN_HEIGHT;
    }

    s_cam_x = cam_x;
    s_cam_y = cam_y;

    // BG1: Primary interactive puzzle playfield
    REG_BG1HOFS = (s16)(s_cam_x + s_shake_x);
    REG_BG1VOFS = (s16)(s_cam_y + s_shake_y);

    // BG2: Subterranean bedrock (scrolling at half-speed for genuine ruin depth!)
    REG_BG2HOFS = (s16)((s_cam_x >> 1) + (s_shake_x >> 1));
    REG_BG2VOFS = (s16)((s_cam_y >> 1) + (s_shake_y >> 1));
}

s16 graphics_get_camera_x(void) {
    return s_cam_x;
}

s16 graphics_get_camera_y(void) {
    return s_cam_y;
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
}

void graphics_set_bg0_tile(u8 x, u8 y, u16 tile, u8 pal) {
    u16 entry = (tile & 0x03FF) | ((pal & 0x0F) << 12);
    s_bg0_map[(y % 32) * 32 + (x % 32)] = entry;
}

void graphics_set_bg1_tile(u8 x, u8 y, u16 tile, u8 pal) {
    u16 entry = (tile & 0x03FF) | ((pal & 0x0F) << 12);
    if (x < 32) {
        s_bg1_map0[(y % 32) * 32 + x] = entry;
    } else {
        s_bg1_map1[(y % 32) * 32 + (x - 32)] = entry;
    }
}

void graphics_set_bg2_tile(u8 x, u8 y, u16 tile, u8 pal) {
    u16 entry = (tile & 0x03FF) | ((pal & 0x0F) << 12);
    if (x < 32) {
        s_bg2_map0[(y % 32) * 32 + x] = entry;
    } else {
        s_bg2_map1[(y % 32) * 32 + (x - 32)] = entry;
    }
}

void graphics_set_metatile(u8 mx, u8 my, MetatileId meta) {
    if (mx >= LEVEL_WIDTH || my >= LEVEL_HEIGHT) return;

    u16 base_tile = TILE_FLOOR_TL;
    u8 pal = PAL_BG_STONE;

    switch (meta) {
        case META_EMPTY:
            base_tile = TILE_FLOOR_TL;
            pal = PAL_BG_STONE;
            break;
        case META_WALL:
            base_tile = TILE_WALL_TL;
            pal = PAL_BG_STONE;
            break;
        case META_DIRT:
            base_tile = TILE_DIRT_TL;
            pal = PAL_BG_DIRT;
            break;
        case META_BOULDER:
        case META_BOULDER_ROLLING:
            base_tile = TILE_BOULDER_TL;
            pal = PAL_BG_STONE;
            break;
        case META_DIAMOND_RED:
            base_tile = TILE_DIAMOND_RED_TL;
            pal = PAL_BG_GEMS;
            break;
        case META_DIAMOND_PURPLE:
            base_tile = TILE_DIAMOND_PURPLE_TL;
            pal = PAL_BG_GEMS;
            break;
        case META_DIAMOND_GREEN:
            base_tile = TILE_DIAMOND_GREEN_TL;
            pal = PAL_BG_GEMS;
            break;
        case META_KEY:
            base_tile = TILE_KEY_TL;
            pal = PAL_BG_OBJECTS;
            break;
        case META_DOOR_LOCKED:
            base_tile = TILE_DOOR_LOCKED_TL;
            pal = PAL_BG_OBJECTS;
            break;
        case META_DOOR_OPEN:
            base_tile = TILE_DOOR_OPEN_TL;
            pal = PAL_BG_OBJECTS;
            break;
        case META_CHEST:
            base_tile = TILE_CHEST_CLOSED_TL;
            pal = PAL_BG_OBJECTS;
            break;
        case META_CHEST_OPEN:
            base_tile = TILE_CHEST_OPEN_TL;
            pal = PAL_BG_OBJECTS;
            break;
        case META_SPIKES:
            base_tile = TILE_SPIKES_TL;
            pal = PAL_BG_OBJECTS;
            break;
        case META_PRESSURE_PLATE:
            base_tile = TILE_PRESSURE_PLATE_TL;
            pal = PAL_BG_OBJECTS;
            break;
        case META_BARRIER:
            base_tile = TILE_BARRIER_TL;
            pal = PAL_BG_STONE;
            break;
        case META_EXIT_GATE:
            base_tile = TILE_EXIT_GATE_TL;
            pal = PAL_BG_OBJECTS;
            break;
        default:
            base_tile = TILE_FLOOR_TL;
            pal = PAL_BG_STONE;
            break;
    }

    u8 tx = mx * 2;
    u8 ty = my * 2;

    graphics_set_bg1_tile(tx,     ty,     base_tile,     pal);
    graphics_set_bg1_tile(tx + 1, ty,     base_tile + 1, pal);
    graphics_set_bg1_tile(tx,     ty + 1, base_tile + 2, pal);
    graphics_set_bg1_tile(tx + 1, ty + 1, base_tile + 3, pal);
}

void graphics_clear_bg0(void) {
    for (int i = 0; i < 32 * 32; i++) {
        s_bg0_map[i] = TILE_EMPTY;
    }
}

void graphics_clear_bg1(void) {
    for (int i = 0; i < 32 * 32; i++) {
        s_bg1_map0[i] = TILE_EMPTY;
        s_bg1_map1[i] = TILE_EMPTY;
    }
}

void graphics_clear_bg2(void) {
    for (int i = 0; i < 32 * 32; i++) {
        s_bg2_map0[i] = TILE_EMPTY;
        s_bg2_map1[i] = TILE_EMPTY;
    }
}

// Populates BG2 with deep subterranean parallax bedrock
void graphics_populate_bg2_bedrock(LevelTheme theme) {
    (void)theme;
    graphics_clear_bg2();

    // Populate a 48x32 area of metatiles on BG2
    for (u8 my = 0; my < 16; my++) {
        for (u8 mx = 0; mx < 24; mx++) {
            u16 base_tile = TILE_BEDROCK_TL;

            // Architectural rhythm: Pillars every 4 metatiles, reliefs between them
            if ((mx % 4) == 0) {
                base_tile = TILE_PILLAR_L_TL;
            } else if ((mx % 4) == 1) {
                base_tile = TILE_PILLAR_R_TL;
            } else if ((mx % 4) == 2 && (my % 2) == 1) {
                base_tile = TILE_FACE_TL;
            } else {
                base_tile = TILE_BEDROCK_TL;
            }

            u8 tx = mx * 2;
            u8 ty = my * 2;
            graphics_set_bg2_tile(tx,     ty,     base_tile,     PAL_BG_STONE);
            graphics_set_bg2_tile(tx + 1, ty,     base_tile + 1, PAL_BG_STONE);
            graphics_set_bg2_tile(tx,     ty + 1, base_tile + 2, PAL_BG_STONE);
            graphics_set_bg2_tile(tx + 1, ty + 1, base_tile + 3, PAL_BG_STONE);
        }
    }
}

void graphics_draw_title_logo(u8 start_x, u8 start_y) {
    // 20 tiles wide x 4 tiles high
    u16 tile_idx = TILE_TITLE_LOGO_BASE;
    for (u8 ty = 0; ty < 4; ty++) {
        for (u8 tx = 0; tx < 20; tx++) {
            graphics_set_bg0_tile(start_x + tx, start_y + ty, tile_idx++, PAL_BG_UI);
        }
    }
}

static u8 s_fade_level = 0;

void graphics_set_fade(u8 level) {
    if (level > 16) level = 16;
    s_fade_level = level;
    if (level == 0) {
        REG_BLDCNT = 0;
        REG_BLDY = 0;
    } else {
        // Hardware Brightness Fade to Black on all layers
        REG_BLDCNT = (2 << 6) | 0x3F;
        REG_BLDY = level;
    }
}

u8 graphics_get_fade(void) {
    return s_fade_level;
}

void graphics_set_torchlight(bool enable, s16 screen_x, s16 screen_y) {
    if (!enable) {
        REG_DISPCNT &= ~WIN0_ENABLE;
        return;
    }

    // Lantern circle radius ~44px around player center (x+8, y+8)
    s16 cx = screen_x + 8;
    s16 cy = screen_y + 8;
    s16 left = cx - 44;
    s16 right = cx + 44;
    s16 top = cy - 44;
    s16 bottom = cy + 44;

    if (left < 0) left = 0;
    if (right > SCREEN_WIDTH) right = SCREEN_WIDTH;
    if (top < 0) top = 0;
    if (bottom > SCREEN_HEIGHT) bottom = SCREEN_HEIGHT;

    REG_WIN0H = ((u8)left << 8) | ((u8)right);
    REG_WIN0V = ((u8)top << 8) | ((u8)bottom);

    // Inside Win0: Full display (BG0, BG1, BG2, OBJ active)
    REG_WININ = 0x001F;

    // Outside Win0: Dim ambient darkness
    REG_WINOUT = 0x001F | 0x0020;
    REG_BLDCNT = (2 << 6) | 0x001F;
    REG_BLDY = 10;

    REG_DISPCNT |= WIN0_ENABLE;
}

void graphics_set_shadow(u8 index, s16 x, s16 y) {
    if (index >= MAX_SPRITES) return;
    // Ground shadow: 16x16 sprite under character feet
    graphics_set_sprite(index, x, y + 4, SPRITE_SHADOW, 0, 1, PAL_OBJ_EFFECTS, false, false);
}

void graphics_enable_alpha_blending(bool enable) {
    (void)enable;
    REG_BLDCNT = 0;
    REG_BLDALPHA = 0;
}

void graphics_print_text(u8 x, u8 y, const char* str, u8 pal) {
    while (*str && x < 30) {
        char c = *str++;
        u16 tile = TILE_BOX_FILL;

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
        } else if (c == '?') {
            tile = TILE_QUESTION;
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
        } else if (c == ' ') {
            tile = TILE_BOX_FILL;
        }

        graphics_set_bg0_tile(x, y, tile, pal);
        x++;
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
    graphics_set_bg0_tile(x + w - 1, y, TILE_BOX_TR, pal);
    graphics_set_bg0_tile(x, y + h - 1, TILE_BOX_BL, pal);
    graphics_set_bg0_tile(x + w - 1, y + h - 1, TILE_BOX_BR, pal);

    for (u8 col = x + 1; col < x + w - 1; col++) {
        graphics_set_bg0_tile(col, y, TILE_BOX_TOP, pal);
        graphics_set_bg0_tile(col, y + h - 1, TILE_BOX_BOTTOM, pal);
    }

    for (u8 row = y + 1; row < y + h - 1; row++) {
        graphics_set_bg0_tile(x, row, TILE_BOX_LEFT, pal);
        graphics_set_bg0_tile(x + w - 1, row, TILE_BOX_RIGHT, pal);
        for (u8 col = x + 1; col < x + w - 1; col++) {
            graphics_set_bg0_tile(col, row, TILE_BOX_FILL, pal);
        }
    }
}

void graphics_draw_hud(u8 diamonds, u8 quota, u8 lives, bool has_key, u32 score, u8 level) {
    // Top engraved stone HUD bar across columns 0..29
    for (u8 c = 0; c < 30; c++) {
        graphics_set_bg0_tile(c, 0, TILE_BOX_FILL, PAL_BG_UI);
        graphics_set_bg0_tile(c, 1, TILE_BOX_BOTTOM, PAL_BG_UI);
    }

    // 1. Diamond HUD icon + count / quota (gleams with star shimmer when quota reached)
    u16 gem_icon = TILE_DIAMOND_ICON;
    if (diamonds >= quota) {
        if ((s_vblank_frame_count >> 3) & 1) {
            gem_icon = TILE_STAR_ICON;
        }
    }
    graphics_set_bg0_tile(1, 0, gem_icon, PAL_BG_UI);
    graphics_print_num(2, 0, diamonds, 2, PAL_BG_UI);
    graphics_print_text(4, 0, "/", PAL_BG_UI);
    graphics_print_num(5, 0, quota, 2, PAL_BG_UI);

    // 2. Beveled Hearts: Lives (♥♥♡) — heartbeat pulse on 1 life remaining
    for (u8 i = 0; i < 3; i++) {
        u16 heart_tile = TILE_HEART_EMPTY;
        if (i < lives) {
            heart_tile = TILE_HEART_FULL;
            if (lives == 1 && i == 0) {
                if ((s_vblank_frame_count >> 3) & 1) {
                    heart_tile = TILE_HEART_EMPTY; // rhythmic heartbeat flicker
                }
            }
        }
        graphics_set_bg0_tile(9 + i, 0, heart_tile, PAL_BG_UI);
    }

    // 3. Golden Key indicator
    if (has_key) {
        graphics_set_bg0_tile(13, 0, TILE_KEY_ICON, PAL_BG_UI);
    } else {
        graphics_set_bg0_tile(13, 0, TILE_BOX_FILL, PAL_BG_UI);
    }

    // 4. Level indicator: "LV 1"
    graphics_print_text(16, 0, "LV", PAL_BG_UI);
    graphics_print_num(18, 0, level, 1, PAL_BG_UI);

    // 5. Score with 6 digits
    graphics_print_num(22, 0, score, 6, PAL_BG_UI);
}
