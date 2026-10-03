#ifndef ASSETS_H
#define ASSETS_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Tile indices in ScreenBlock (Background tiles)
#define TILE_EMPTY        0
#define TILE_SOLID_HULL   1
#define TILE_GRATE        2
#define TILE_CONDUIT      3
#define TILE_HAZARD       4
#define TILE_RED_BARRIER  5
#define TILE_AIRLOCK_DOOR 6
#define TILE_SAVE_CONSOLE 7

// Sprite tile offsets in OBJ CharBlock 4 (1D mapping mode)
#define SPRITE_TILE_PLAYER_IDLE    0   // 16x32 = 8 tiles (0..7)
#define SPRITE_TILE_PLAYER_RUN     8   // 16x32 = 8 tiles (8..15)
#define SPRITE_TILE_PLAYER_JUMP   16   // 16x32 = 8 tiles (16..23)
#define SPRITE_TILE_PLAYER_DRONE  24   // 16x16 = 4 tiles (24..27)
#define SPRITE_TILE_BEAM          28   // 8x8   = 1 tile  (28)
#define SPRITE_TILE_MISSILE       29   // 8x8   = 1 tile  (29)
#define SPRITE_TILE_CRAWLER       32   // 16x16 = 4 tiles (32..35)
#define SPRITE_TILE_ITEM_MISSILE  36   // 16x16 = 4 tiles (36..39)
#define SPRITE_TILE_BOSS          40   // 32x32 = 16 tiles (40..55)
#define SPRITE_TILE_HUD_ENERGY    56   // 8x8   = 1 tile  (56) - Energy Heart / E-Tank
#define SPRITE_TILE_HUD_MISSILE   57   // 8x8   = 1 tile  (57) - Missile icon
#define SPRITE_TILE_HUD_SELECT    58   // 8x8   = 1 tile  (58) - Selector arrow
#define SPRITE_TILE_HUD_NUM0      60   // 8x8   = 10 tiles (60..69) - Digits '0'..'9'

void assets_init(void);

#ifdef __cplusplus
}
#endif

#endif // ASSETS_H
