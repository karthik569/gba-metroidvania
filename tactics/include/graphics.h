#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "gba.h"

#define BG0_SBB 28   // Topmost UI & Menus
#define BG1_SBB 29   // Movement & Attack Reach Overlay
#define BG2_SBB 30   // Terrain Map Grid
#define BG_CBB_TILES 0
#define SPRITE_CBB 4

#ifdef __cplusplus
extern "C" {
#endif

void graphics_init(void);
void graphics_wait_vblank(void);
void graphics_oam_hide_all(void);
void graphics_oam_copy(void);
void graphics_set_sprite(u8 index, s16 x, s16 y, u16 tile, u8 shape, u8 size, u8 pal, bool hflip, bool vflip);

// Camera management
void graphics_set_camera(s16 cam_x, s16 cam_y);
s16  graphics_get_camera_x(void);
s16  graphics_get_camera_y(void);
void graphics_trigger_shake(u8 intensity, u8 frames);
void graphics_update_shake(void);

// Tilemap manipulation
void graphics_set_bg0_tile(u8 x, u8 y, u16 tile, u8 pal);
void graphics_set_bg1_tile(u8 x, u8 y, u16 tile, u8 pal);
void graphics_set_bg2_tile(u8 x, u8 y, u16 tile, u8 pal);
void graphics_clear_bg0(void);
void graphics_clear_bg1(void);

// Text printing on BG0
void graphics_print_text(u8 x, u8 y, const char* str, u8 pal);
void graphics_print_num(u8 x, u8 y, u32 num, u8 digits, u8 pal);

#ifdef __cplusplus
}
#endif

#endif // GRAPHICS_H
