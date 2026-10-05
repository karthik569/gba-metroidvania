#ifndef ASSET_DATA_H
#define ASSET_DATA_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Palettes
extern const u16 g_pal_bg_ui[16];
extern const u16 g_pal_bg_angkor[16];
extern const u16 g_pal_bg_bavaria[16];
extern const u16 g_pal_obj_explorer[16];
extern const u16 g_pal_obj_enemies[16];
extern const u16 g_pal_obj_objects[16];

// Tile Data (4bpp planar, 32 bytes per 8x8 tile)
extern const u8 g_tiles_angkor[4096];     // 128 tiles (32 metatiles * 4)
extern const u8 g_tiles_bavaria[4096];    // 128 tiles
extern const u8 g_sprites_explorer[2048]; // 64 tiles (16 sprites * 4)
extern const u8 g_sprites_enemies[1024];  // 32 tiles (8 sprites * 4)
extern const u8 g_sprites_objects[2048];  // 64 tiles (16 sprites * 4)
extern const u8 g_tiles_ui[448];          // 14 tiles (HUD icons & frames)
extern const u8 g_tiles_title_logo[2560]; // 80 tiles (20x4 title banner)

#ifdef __cplusplus
}
#endif

#endif // ASSET_DATA_H
