#include "graphics.h"
#include "assets.h"

static OBJ_ATTR s_oam_buffer[MAX_SPRITES];

static vu16* const s_bg0_map = (vu16*)(VRAM_BASE + (BG0_SBB * 0x800));

// BG1 (64x32) uses SBB 28 (left block) and SBB 29 (right block)
static vu16* const s_bg1_map0 = (vu16*)(VRAM_BASE + (BG1_SBB * 0x800));
static vu16* const s_bg1_map1 = (vu16*)(VRAM_BASE + ((BG1_SBB + 1) * 0x800));

// BG2 (64x32) uses SBB 30 (left block) and SBB 31 (right block)
static vu16* const s_bg2_map0 = (vu16*)(VRAM_BASE + (BG2_SBB * 0x800));
static vu16* const s_bg2_map1 = (vu16*)(VRAM_BASE + ((BG2_SBB + 1) * 0x800));

static s16 s_cam_x = 0;
static s16 s_cam_y = 0;

static u8 s_shake_intensity = 0;
static u8 s_shake_frames = 0;
static s8 s_shake_x = 0;
static s8 s_shake_y = 0;

void graphics_init(void) {
    // Mode 0: BG0, BG1, BG2 active + 1D sprite mapping
    REG_DISPCNT = MODE_0 | BG0_ENABLE | BG1_ENABLE | BG2_ENABLE | OBJ_ENABLE | OBJ_1D_MAP;

    // BG0: Priority 0 (Topmost UI / Menus / Dialogs, 32x32)
    REG_BG0CNT = BG_CBB(BG_CBB_TILES) | BG_SBB(BG0_SBB) | BG_PRIORITY(0) | BG_SIZE_32x32 | BG_COLOR_16;
    REG_BG0HOFS = 0;
    REG_BG0VOFS = 0;

    // BG1: Priority 1 (Foreground Arches & Lighting Vignette, 64x32)
    REG_BG1CNT = BG_CBB(BG_CBB_TILES) | BG_SBB(BG1_SBB) | BG_PRIORITY(1) | BG_SIZE_64x32 | BG_COLOR_16;
    REG_BG1HOFS = 0;
    REG_BG1VOFS = 0;

    // BG2: Priority 2 (Dungeon Terrain Map, 64x32)
    REG_BG2CNT = BG_CBB(BG_CBB_TILES) | BG_SBB(BG2_SBB) | BG_PRIORITY(2) | BG_SIZE_64x32 | BG_COLOR_16;
    REG_BG2HOFS = 0;
    REG_BG2VOFS = 0;

    // Initialize alpha blending (disabled initially)
    REG_BLDCNT = 0;
    REG_BLDALPHA = 0;

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
    a2 |= (1 << 10); // Priority 1 (renders above BG2 terrain and below BG0 HUD)

    s_oam_buffer[index].attr0 = a0;
    s_oam_buffer[index].attr1 = a1;
    s_oam_buffer[index].attr2 = a2;
}

void graphics_set_camera(s16 cam_x, s16 cam_y) {
    // 320x240 map in 240x160 screen -> max scroll is 80x80
    if (cam_x < 0) cam_x = 0;
    if (cam_x > 80) cam_x = 80;
    if (cam_y < 0) cam_y = 0;
    if (cam_y > 80) cam_y = 80;

    s_cam_x = cam_x;
    s_cam_y = cam_y;

    REG_BG1HOFS = (s16)(s_cam_x + s_shake_x);
    REG_BG1VOFS = (s16)(s_cam_y + s_shake_y);
    REG_BG2HOFS = (s16)(s_cam_x + s_shake_x);
    REG_BG2VOFS = (s16)(s_cam_y + s_shake_y);
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

void graphics_set_blending(bool enable, u8 eva, u8 evb) {
    if (enable) {
        // 1st target: BG1 (bit 1)
        // Blend Mode: Alpha (bit 6)
        // 2nd target: BG2 (bit 10), BD (bit 13)
        REG_BLDCNT = (1 << 1) | (1 << 6) | (1 << 10) | (1 << 13);
        REG_BLDALPHA = (eva & 0x1F) | ((evb & 0x1F) << 8);
    } else {
        REG_BLDCNT = 0;
        REG_BLDALPHA = 0;
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
    if (mx >= ROOM_WIDTH || my >= ROOM_HEIGHT) return;

    u16 base_tile = TILE_FLOOR_STONE_TL;
    u8 pal = PAL_BG_STONE;

    switch (meta) {
        case META_FLOOR_STONE:
            base_tile = TILE_FLOOR_STONE_TL;
            pal = PAL_BG_STONE;
            break;
        case META_FLOOR_DIRT:
            base_tile = TILE_FLOOR_DIRT_TL;
            pal = PAL_BG_NATURE;
            break;
        case META_WALL_TOP:
            base_tile = TILE_WALL_TOP_TL;
            pal = PAL_BG_STONE;
            break;
        case META_WALL_FACE:
            base_tile = TILE_WALL_FACE_TL;
            pal = PAL_BG_STONE;
            break;
        case META_WATER:
            base_tile = TILE_WATER_TL;
            pal = PAL_BG_WATER;
            break;
        case META_BRIDGE:
            base_tile = TILE_BRIDGE_TL;
            pal = PAL_BG_NATURE;
            break;
        case META_DOOR_OPEN:
            base_tile = TILE_DOOR_OPEN_TL;
            pal = PAL_BG_STONE;
            break;
        case META_DOOR_LOCKED:
            base_tile = TILE_DOOR_LOCKED_TL;
            pal = PAL_BG_STONE;
            break;
        case META_DOOR_BOSS:
            base_tile = TILE_DOOR_BOSS_TL;
            pal = PAL_BG_FIRE;
            break;
        case META_WALL_CRACKED:
            base_tile = TILE_WALL_CRACKED_TL;
            pal = PAL_BG_STONE;
            break;
        case META_SWITCH_OFF:
            base_tile = TILE_SWITCH_OFF_TL;
            pal = PAL_BG_WATER;
            break;
        case META_SWITCH_ON:
            base_tile = TILE_SWITCH_ON_TL;
            pal = PAL_BG_FIRE;
            break;
        case META_PLATE_UP:
            base_tile = TILE_PLATE_UP_TL;
            pal = PAL_BG_STONE;
            break;
        case META_PLATE_DOWN:
            base_tile = TILE_PLATE_DOWN_TL;
            pal = PAL_BG_STONE;
            break;
        case META_TORCH_UNLIT:
            base_tile = TILE_TORCH_UNLIT_TL;
            pal = PAL_BG_STONE;
            break;
        case META_TORCH_LIT:
            base_tile = TILE_TORCH_LIT_TL;
            pal = PAL_BG_FIRE;
            break;
        case META_CHEST_CLOSED:
            base_tile = TILE_CHEST_CLOSED_TL;
            pal = PAL_BG_FIRE;
            break;
        case META_CHEST_OPEN:
            base_tile = TILE_CHEST_OPEN_TL;
            pal = PAL_BG_FIRE;
            break;
        case META_POT:
            base_tile = TILE_POT_TL;
            pal = PAL_BG_NATURE;
            break;
        case META_BLOCK:
            base_tile = TILE_BLOCK_TL;
            pal = PAL_BG_STONE;
            break;
        case META_CAMPFIRE:
            base_tile = TILE_CAMPFIRE_TL;
            pal = PAL_BG_FIRE;
            break;
        case META_TREE:
            base_tile = TILE_TREE_TL;
            pal = PAL_BG_NATURE;
            break;
        default:
            base_tile = TILE_FLOOR_STONE_TL;
            pal = PAL_BG_STONE;
            break;
    }

    u8 tx = mx * 2;
    u8 ty = my * 2;

    graphics_set_bg2_tile(tx,     ty,     base_tile,     pal);
    graphics_set_bg2_tile(tx + 1, ty,     base_tile + 1, pal);
    graphics_set_bg2_tile(tx,     ty + 1, base_tile + 2, pal);
    graphics_set_bg2_tile(tx + 1, ty + 1, base_tile + 3, pal);
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

    // Top row
    graphics_set_bg0_tile(x, y, TILE_BOX_TL, pal);
    for (u8 col = 1; col < w - 1; col++) {
        graphics_set_bg0_tile(x + col, y, TILE_BOX_TOP, pal);
    }
    graphics_set_bg0_tile(x + w - 1, y, TILE_BOX_TR, pal);

    // Middle rows
    for (u8 row = 1; row < h - 1; row++) {
        graphics_set_bg0_tile(x, y + row, TILE_BOX_LEFT, pal);
        for (u8 col = 1; col < w - 1; col++) {
            graphics_set_bg0_tile(x + col, y + row, TILE_BOX_FILL, pal);
        }
        graphics_set_bg0_tile(x + w - 1, y + row, TILE_BOX_RIGHT, pal);
    }

    // Bottom row
    graphics_set_bg0_tile(x, y + h - 1, TILE_BOX_BL, pal);
    for (u8 col = 1; col < w - 1; col++) {
        graphics_set_bg0_tile(x + col, y + h - 1, TILE_BOX_BOTTOM, pal);
    }
    graphics_set_bg0_tile(x + w - 1, y + h - 1, TILE_BOX_BR, pal);
}
