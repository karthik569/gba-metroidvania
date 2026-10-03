#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "gba.h"
#include "assets.h"

#ifdef __cplusplus
extern "C" {
#endif

void graphics_init(void);
void graphics_wait_vblank(void);

// Sprite management (128 hardware sprites via OAM DMA)
void graphics_oam_hide_all(void);
void graphics_oam_copy(void);
void graphics_set_sprite(u8 index, s16 x, s16 y, u16 tile, u8 shape, u8 size, u8 pal, bool hflip, bool vflip);

// Screen Shake
void graphics_trigger_shake(u8 intensity, u8 frames);
void graphics_update_shake(void);

// Tilemap operations
void graphics_set_bg0_tile(u8 x, u8 y, u16 tile, u8 pal);
void graphics_set_bg1_tile(u8 x, u8 y, u16 tile, u8 pal);
void graphics_set_bg2_tile(u8 x, u8 y, u16 tile, u8 pal);

void graphics_clear_bg0(void);
void graphics_clear_bg1(void);
void graphics_clear_bg2(void);

// Court & Stadium Renderers
void graphics_render_court(CourtSurface surface);

// Text & HUD helpers
void graphics_print_text(u8 x, u8 y, const char* str, u8 pal);
void graphics_print_num(u8 x, u8 y, u32 num, u8 digits, u8 pal);
void graphics_draw_box(u8 x, u8 y, u8 w, u8 h, u8 pal);

#ifdef __cplusplus
}
#endif

#endif // GRAPHICS_H
