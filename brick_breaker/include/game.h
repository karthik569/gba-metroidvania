#ifndef GAME_H
#define GAME_H

#include "types.h"

#define PLAYFIELD_X_MIN     8
#define PLAYFIELD_X_MAX     184
#define PLAYFIELD_Y_MIN     8
#define PLAYFIELD_Y_MAX     160

#define BRICK_COLS          11
#define BRICK_ROWS          8
#define BRICK_WIDTH         16
#define BRICK_HEIGHT        8
#define BRICK_START_Y       16

#define MAX_BALLS           3
#define MAX_CAPSULES        4
#define MAX_LASERS          4
#define MAX_STAGES          5

// Brick Types
#define BRK_EMPTY           0
#define BRK_RED             1   // 100 pts
#define BRK_BLUE            2   // 120 pts
#define BRK_GREEN           3   // 150 pts
#define BRK_YELLOW          4   // 180 pts
#define BRK_MAGENTA         5   // 200 pts
#define BRK_CYAN            6   // 250 pts
#define BRK_SILVER          7   // 500 pts (2 hits)
#define BRK_GOLD            8   // Indestructible

// Power-up Types
typedef enum {
    PWR_NONE  = 0,
    PWR_WIDE  = 1,
    PWR_LASER = 2,
    PWR_MULTI = 3,
    PWR_CATCH = 4,
    PWR_SLOW  = 5,
    PWR_LIFE  = 6
} PowerUpType;

// Game State Machine
typedef enum {
    STATE_TITLE       = 0,
    STATE_PLAYING     = 1,
    STATE_PAUSED      = 2,
    STATE_LEVEL_CLEAR = 3,
    STATE_GAME_OVER   = 4,
    STATE_VICTORY     = 5
} GameState;

typedef struct {
    fixed_t x, y;
    fixed_t vx;
    u8 width;           // 32 or 48
    PowerUpType active_pwr;
    u16 pwr_timer;
    bool laser_equipped;
    bool catch_equipped;
    u8 laser_cooldown;
} Paddle;

typedef struct {
    fixed_t x, y;
    fixed_t vx, vy;
    bool active;
    bool stuck_to_paddle;
    s16 stuck_offset_x;
    u8 speed_tier;
} Ball;

typedef struct {
    fixed_t x, y;
    fixed_t vy;
    PowerUpType type;
    bool active;
} Capsule;

typedef struct {
    fixed_t x, y;
    fixed_t vy;
    bool active;
} Laser;

typedef struct {
    u8 type;
    u8 hp;
} Brick;

#ifdef __cplusplus
extern "C" {
#endif

void game_init(void);
void game_start_new(void);
void game_load_stage(u8 stage_num);
void game_update(void);
void game_render(void);
GameState game_get_state(void);

#ifdef __cplusplus
}
#endif

#endif // GAME_H
