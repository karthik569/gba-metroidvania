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

// Sprite tile offsets in OBJ CharBlock
#define SPRITE_TILE_PLAYER_IDLE    0   // 16x32 = 8 tiles
#define SPRITE_TILE_PLAYER_RUN     8   // 16x32
#define SPRITE_TILE_PLAYER_JUMP   16   // 16x32
#define SPRITE_TILE_PLAYER_DRONE  24   // 16x16 = 4 tiles
#define SPRITE_TILE_BEAM          28   // 8x8 = 1 tile
#define SPRITE_TILE_MISSILE       29   // 8x8 = 1 tile
#define SPRITE_TILE_CRAWLER       32   // 16x16 = 4 tiles
#define SPRITE_TILE_BOSS          40   // 32x32 = 16 tiles
#define SPRITE_TILE_ITEM_MISSILE  56   // 16x16 = 4 tiles

void assets_init(void);

#ifdef __cplusplus
}
#endif

#endif // ASSETS_H
