#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

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

// 8.8 Fixed-Point Math helpers for smooth 60fps subpixel physics
typedef s32 fixed_t;

#define FP_SHIFT        8
#define FP_ONE          (1 << FP_SHIFT)
#define INT_TO_FP(x)    ((fixed_t)((x) << FP_SHIFT))
#define FP_TO_INT(x)    ((s32)((x) >> FP_SHIFT))
#define FP_MUL(x, y)    ((fixed_t)(((x) * (y)) >> FP_SHIFT))
#define FP_DIV(x, y)    ((fixed_t)(((x) << FP_SHIFT) / (y)))

// Screen dimensions
#define SCREEN_WIDTH    240
#define SCREEN_HEIGHT   160

// GBA 15-bit BGR Color helper (5 bits per channel)
#define RGB15(r, g, b)  ((u16)(((r) & 0x1F) | (((g) & 0x1F) << 5) | (((b) & 0x1F) << 10)))

#endif // TYPES_H
