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

// Dungeon Room Dimensions (20x15 metatiles of 16x16 pixels = 320x240 pixels)
#define ROOM_WIDTH      20
#define ROOM_HEIGHT     15
#define TILE_SIZE       16
#define VIEW_TILES_X    15
#define VIEW_TILES_Y    10

// 4-Directional Facing
typedef enum {
    DIR_DOWN  = 0,
    DIR_UP    = 1,
    DIR_LEFT  = 2,
    DIR_RIGHT = 3
} Direction;

// Sub-Weapons / Tools
typedef enum {
    TOOL_NONE       = 0,
    TOOL_BOOMERANG  = 1,
    TOOL_BOMB       = 2,
    TOOL_FIRE_WAND  = 3,
    TOOL_COUNT      = 4
} SubWeaponType;

#endif // TYPES_H
