#ifndef ASSETS_H
#define ASSETS_H

#include "types.h"

// BG Tile Indices
#define TILE_EMPTY              0
#define TILE_WALL_CORNER_TL     1
#define TILE_WALL_TOP           2
#define TILE_WALL_CORNER_TR     3
#define TILE_WALL_LEFT          4
#define TILE_WALL_RIGHT         5
#define TILE_SIDEBAR_BG         6
#define TILE_SIDEBAR_DIVIDER    7

// Brick BG Tiles (16x8 = 2 tiles each)
#define BRICK_RED_L             8
#define BRICK_RED_R             9
#define BRICK_BLUE_L            10
#define BRICK_BLUE_R            11
#define BRICK_GREEN_L           12
#define BRICK_GREEN_R           13
#define BRICK_YELLOW_L          14
#define BRICK_YELLOW_R          15
#define BRICK_MAGENTA_L         16
#define BRICK_MAGENTA_R         17
#define BRICK_CYAN_L            18
#define BRICK_CYAN_R            19
#define BRICK_SILVER_L          20
#define BRICK_SILVER_R          21
#define BRICK_SILVER_CRACK_L    22
#define BRICK_SILVER_CRACK_R    23
#define BRICK_GOLD_L            24
#define BRICK_GOLD_R            25

// Font in BG tiles
#define TILE_FONT_0             30
#define TILE_FONT_A             40
#define TILE_FONT_COLON         66
#define TILE_FONT_HYPHEN        67

// Sprite Tile Offsets (1D mapping)
#define SPRITE_TILE_PADDLE_LEFT     0   // 16x8 = 2 tiles (0..1)
#define SPRITE_TILE_PADDLE_RIGHT    2   // 16x8 = 2 tiles (2..3)
#define SPRITE_TILE_PADDLE_MID      4   // 16x8 = 2 tiles (4..5)
#define SPRITE_TILE_PADDLE_LASER_L  6   // 16x8 = 2 tiles (6..7)
#define SPRITE_TILE_PADDLE_LASER_R  8   // 16x8 = 2 tiles (8..9)
#define SPRITE_TILE_BALL           10   // 8x8   = 1 tile  (10)
#define SPRITE_TILE_LASER          11   // 8x8   = 1 tile  (11)

// Falling Power-up Capsules (16x8 = 2 tiles each)
#define SPRITE_TILE_PWR_WIDE       12
#define SPRITE_TILE_PWR_LASER      14
#define SPRITE_TILE_PWR_MULTI      16
#define SPRITE_TILE_PWR_CATCH      18
#define SPRITE_TILE_PWR_SLOW       20
#define SPRITE_TILE_PWR_LIFE       22

// Sidebar icons
#define SPRITE_TILE_MINI_PADDLE    24   // 8x8   = 1 tile  (24)
#define SPRITE_TILE_BADGE_NONE     26   // 16x16 = 4 tiles (26..29)
#define SPRITE_TILE_BADGE_WIDE     30
#define SPRITE_TILE_BADGE_LASER    34
#define SPRITE_TILE_BADGE_MULTI    38
#define SPRITE_TILE_BADGE_CATCH    42

#ifdef __cplusplus
extern "C" {
#endif

void assets_init(void);

#ifdef __cplusplus
}
#endif

#endif // ASSETS_H
