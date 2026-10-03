#include "graphics.h"
#include "assets.h"

static OBJ_ATTR s_oam_buffer[MAX_SPRITES];

static vu16* const s_bg0_map = (vu16*)(VRAM_BASE + (BG0_SBB * 0x800));
static vu16* const s_bg1_map = (vu16*)(VRAM_BASE + (BG1_SBB * 0x800));
static vu16* const s_bg2_map = (vu16*)(VRAM_BASE + (BG2_SBB * 0x800));

static s16 s_cam_x = 0;
static s16 s_cam_y = 0;

static u8 s_shake_intensity = 0;
static u8 s_shake_frames = 0;
static s8 s_shake_x = 0;
static s8 s_shake_y = 0;

void graphics_init(void) {
    // Mode 0: 3 BG layers active (BG0, BG1, BG2) + 1D sprite mapping
    REG_DISPCNT = MODE_0 | BG0_ENABLE | BG1_ENABLE | BG2_ENABLE | OBJ_ENABLE | OBJ_1D_MAP;

    // BG0: Priority 0 (Topmost UI / Menus / Dialogs)
    REG_BG0CNT = BG_CBB(BG_CBB_TILES) | BG_SBB(BG0_SBB) | BG_PRIORITY(0) | BG_SIZE_32x32 | BG_COLOR_16;
    REG_BG0HOFS = 0;
    REG_BG0VOFS = 0;

    // BG1: Priority 1 (Movement & Attack Reach Grid Highlight)
    REG_BG1CNT = BG_CBB(BG_CBB_TILES) | BG_SBB(BG1_SBB) | BG_PRIORITY(1) | BG_SIZE_32x32 | BG_COLOR_16;
    REG_BG1HOFS = 0;
    REG_BG1VOFS = 0;

    // BG2: Priority 2 (Terrain Map Grid)
    REG_BG2CNT = BG_CBB(BG_CBB_TILES) | BG_SBB(BG2_SBB) | BG_PRIORITY(2) | BG_SIZE_32x32 | BG_COLOR_16;
    REG_BG2HOFS = 0;
    REG_BG2VOFS = 0;

    graphics_oam_hide_all();
    graphics_oam_copy();

    graphics_clear_bg0();
    graphics_clear_bg1();
}

void graphics_wait_vblank(void) {
    while (REG_VCOUNT >= 160);
    while (REG_VCOUNT < 160);
}

void graphics_oam_hide_all(void) {
    for (int i = 0; i < MAX_SPRITES; i++) {
        s_oam_buffer[i].attr0 = 0x0200; // Disable (y=offscreen, double-size disabled)
        s_oam_buffer[i].attr1 = 0x0000;
        s_oam_buffer[i].attr2 = 0x0000;
        s_oam_buffer[i].dummy = 0x0000;
    }
}

void graphics_oam_copy(void) {
    // DMA Channel 3 copy: 128 sprites * 8 bytes = 1024 bytes (256 words)
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

    s_oam_buffer[index].attr0 = a0;
    s_oam_buffer[index].attr1 = a1;
    s_oam_buffer[index].attr2 = a2;
}

void graphics_set_camera(s16 cam_x, s16 cam_y) {
    // 320x240 map in 240x160 viewport -> max scroll is 80x80
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
    for (int i = 0; i < 32 * 32; i++) {
        s_bg0_map[i] = TILE_EMPTY;
    }
}

void graphics_clear_bg1(void) {
    for (int i = 0; i < 32 * 32; i++) {
        s_bg1_map[i] = TILE_EMPTY;
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
        } else if (c == '<') {
            tile = TILE_ARROW_LEFT;
        } else if (c == '>') {
            tile = TILE_ARROW_RIGHT;
        } else if (c == '[') {
            tile = TILE_BRACKET_L;
        } else if (c == ']') {
            tile = TILE_BRACKET_R;
        } else if (c == '/') {
            tile = TILE_SLASH;
        } else if (c == '%') {
            tile = TILE_PERCENT;
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
