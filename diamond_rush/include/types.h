#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef int8_t    s8;
typedef int16_t   s16;
typedef int32_t   s32;

typedef volatile uint8_t   vu8;
typedef volatile uint16_t  vu16;
typedef volatile uint32_t  vu32;
typedef volatile int8_t    vs8;
typedef volatile int16_t   vs16;
typedef volatile int32_t   vs32;

// 8.8 Fixed-Point Math
typedef s32 fixed_t;
#define FP_SHIFT        8
#define FP_SCALE        (1 << FP_SHIFT)
#define INT_TO_FP(x)    ((fixed_t)((s32)(x) * FP_SCALE))
#define FP_TO_INT(x)    ((s16)((x) >> FP_SHIFT))
#define FP_MUL(a, b)    ((fixed_t)(((a) * (b)) >> FP_SHIFT))
#define FP_DIV(a, b)    ((fixed_t)(((a) << FP_SHIFT) / (b)))

// 15-bit GBA Color Helper (5 bits per RGB component)
#define RGB15(r, g, b)  ((u16)(((r) & 0x1F) | (((g) & 0x1F) << 5) | (((b) & 0x1F) << 10)))

#define SCREEN_WIDTH    240
#define SCREEN_HEIGHT   160

// Level Dimensions (24x16 metatiles of 16x16 pixels = 384x256 pixels)
#define LEVEL_WIDTH     24
#define LEVEL_HEIGHT    16
#define TILE_SIZE       16
#define VIEW_TILES_X    15
#define VIEW_TILES_Y    10

// 4-Directional Facing
typedef enum {
    DIR_DOWN  = 0,
    DIR_UP    = 1,
    DIR_LEFT  = 2,
    DIR_RIGHT = 3,
    DIR_NONE  = 4
} Direction;

// Metatile / Terrain IDs (Diamond Rush Elements)
typedef enum {
    META_EMPTY           = 0,  // Passable empty stone corridor
    META_WALL            = 1,  // Indestructible ancient temple wall
    META_DIRT            = 2,  // Soft soil / leafy foliage (cleared on walk)
    META_BOULDER         = 3,  // Heavy stone boulder (falls, rolls, can be pushed)
    META_DIAMOND_RED     = 4,  // Red Ruby (+100 pts, falls if unsupported)
    META_DIAMOND_PURPLE  = 5,  // Purple Amethyst (+250 pts, falls)
    META_DIAMOND_GREEN   = 6,  // Green Emerald (+500 pts, secret)
    META_KEY             = 7,  // Ancient golden key
    META_DOOR_LOCKED     = 8,  // Heavy locked stone gate
    META_DOOR_OPEN       = 9,  // Unlocked gate
    META_CHEST           = 10, // Treasure chest (+300 pts & extra diamonds)
    META_CHEST_OPEN      = 11, // Opened treasure chest
    META_SPIKES          = 12, // Floor spikes (hurts player)
    META_PRESSURE_PLATE  = 13, // Stone switch plate (depressed by boulder or player)
    META_BARRIER         = 14, // Retractable barrier pillar
    META_EXIT_GATE       = 15, // Exit portal to complete stage
    META_BOULDER_ROLLING = 16, // Transient rolling boulder
    META_COUNT           = 17
} MetatileId;

// Game State Flow
typedef enum {
    STATE_TITLE,
    STATE_LEVEL_INTRO,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_LEVEL_CLEAR,
    STATE_GAME_OVER,
    STATE_VICTORY
} GameState;

#endif // TYPES_H
