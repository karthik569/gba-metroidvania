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

// Court Geometry Constants (Screen Pixel Coordinates in Elevated Perspective)
#define NET_Y               76
#define BASELINE_NEAR_Y     140
#define BASELINE_FAR_Y      24
#define SERVICE_NEAR_Y      108
#define SERVICE_FAR_Y       50

// Court Surfaces
typedef enum {
    SURFACE_GRASS       = 0,
    SURFACE_CLAY        = 1,
    SURFACE_HARD        = 2,
    SURFACE_COUNT       = 3
} CourtSurface;

// Shot Types
typedef enum {
    SHOT_FLAT           = 0,
    SHOT_TOPSPIN        = 1,
    SHOT_SLICE          = 2,
    SHOT_LOB            = 3,
    SHOT_DROP           = 4,
    SHOT_SMASH          = 5,
    SHOT_SERVE          = 6
} ShotType;

#endif // TYPES_H
