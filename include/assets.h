#ifndef ASSETS_H
#define ASSETS_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Tile indices in ScreenBlock
#define TILE_EMPTY        0
#define TILE_SOLID_HULL   1
#define TILE_GRATE        2
#define TILE_CONDUIT      3
#define TILE_HAZARD       4
#define TILE_RED_BARRIER  5
#define TILE_AIRLOCK_DOOR 6
#define TILE_SAVE_CONSOLE 7

// Sprite tile offsets in OBJ CharBlock (1D mapping mode)
#define SPRITE_TILE_PLAYER_IDLE    0   // 16x16 = 4 tiles (0..3)
#define SPRITE_TILE_PLAYER_RUN     4   // 16x16 = 4 tiles (4..7)
#define SPRITE_TILE_PLAYER_JUMP    8   // 16x16 = 4 tiles (8..11)
#define SPRITE_TILE_PLAYER_DRONE  12   // 16x16 = 4 tiles (12..15)
#define SPRITE_TILE_BEAM          16   // 8x8   = 1 tile  (16)
#define SPRITE_TILE_MISSILE       17   // 8x8   = 1 tile  (17)
#define SPRITE_TILE_CRAWLER       20   // 16x16 = 4 tiles (20..23)
#define SPRITE_TILE_ITEM_MISSILE  24   // 16x16 = 4 tiles (24..27)
#define SPRITE_TILE_BOSS          32   // 32x32 = 16 tiles (32..47)

void assets_init(void);

#ifdef __cplusplus
}
#endif

#endif // ASSETS_H
