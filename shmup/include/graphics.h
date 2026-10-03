#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "gba.h"

#define BG0_SBB 28
#define BG1_SBB 29
#define BG2_SBB 30
#define BG_CBB_TILES 0
#define SPRITE_CBB 4

#define MAP_WIDTH_TILES  32
#define MAP_HEIGHT_TILES 32

#ifdef __cplusplus
extern "C" {
#endif

void graphics_init(void);
void graphics_wait_vblank(void);
void graphics_oam_hide_all(void);
void graphics_oam_copy(void);
void graphics_set_sprite(u8 index, s16 x, s16 y, u16 tile, u8 shape, u8 size, u8 pal, bool hflip, bool vflip);
void graphics_set_scroll(s16 bg1_vofs, s16 bg2_vofs);
void graphics_trigger_shake(u8 intensity, u8 frames);
void graphics_update_shake(void);

// Tilemap helpers
void graphics_set_bg0_tile(u8 x, u8 y, u16 tile, u8 pal);
void graphics_set_bg1_tile(u8 x, u8 y, u16 tile, u8 pal);
void graphics_set_bg2_tile(u8 x, u8 y, u16 tile, u8 pal);
void graphics_print_text(u8 x, u8 y, const char* str, u8 pal);
void graphics_print_num(u8 x, u8 y, u32 num, u8 digits, u8 pal);

#ifdef __cplusplus
}
#endif

#endif // GRAPHICS_H
