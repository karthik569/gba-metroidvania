#ifndef GAME_H
#define GAME_H

#include "types.h"

#define MAX_PLAYER_BULLETS 16
#define MAX_ENEMIES        12
#define MAX_ENEMY_BULLETS  32
#define MAX_CAPSULES       4
#define MAX_PARTICLES      16
#define MAX_BOSS_PODS      2

// Enemy Types
typedef enum {
    ENEMY_NONE = 0,
    ENEMY_SCOUT,        // 16x16: swooping sine path
    ENEMY_INTERCEPTOR,  // 16x16: high speed diagonal dive
    ENEMY_GUNSHIP,      // 32x16: heavy armored platform, 3-way burst
    ENEMY_CRUISER       // 32x16: mid-boss fortress craft
} EnemyType;

// Power-Up Types
typedef enum {
    PWR_NONE = 0,
    PWR_WEAPON,         // [P] Weapon upgrade (Vulcan -> Twin -> Spread -> Plasma)
    PWR_BOMB,           // [B] Nova Bomb +1
    PWR_SHIELD,         // [S] Restore Energy Shield
    PWR_1UP             // [1UP] Extra reserve life + 2000 pts
} PowerUpType;

// Boss Phase
typedef enum {
    BOSS_PHASE_INACTIVE = 0,
    BOSS_PHASE_SHIELDED,
    BOSS_PHASE_EXPOSED,
    BOSS_PHASE_ENRAGED,
    BOSS_PHASE_DYING
} BossPhase;

// Game State
typedef enum {
    STATE_TITLE = 0,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_STAGE_CLEAR,
    STATE_GAME_OVER,
    STATE_VICTORY
} GameState;

// Player Entity
typedef struct {
    fixed_t x, y;
    fixed_t vx, vy;
    u8 bank_frame;      // 0: neutral, 1: left, 2: right
    u8 weapon_lvl;      // 1..4
    u8 bombs;           // 0..5
    u8 lives;           // 0..9
    bool shield_active; // absorbs 1 hit
    u8 invuln_timer;    // invulnerability flash countdown
    u8 fire_cooldown;
    bool is_dead;
    u8 death_timer;
} Player;

// Player Bullet
typedef struct {
    bool active;
    fixed_t x, y, vx, vy;
    u8 damage;
    u8 type;
} PlayerBullet;

// Enemy Entity
typedef struct {
    bool active;
    EnemyType type;
    fixed_t x, y, vx, vy;
    s16 hp, max_hp;
    u16 timer;
    u8 shoot_cooldown;
    u8 flash_timer;
} Enemy;

// Enemy Bullet
typedef struct {
    bool active;
    fixed_t x, y, vx, vy;
    u8 type;            // 0: orb, 1: bolt
} EnemyBullet;

// Boss Defense Pod
typedef struct {
    bool active;
    fixed_t x, y;
    s16 hp;
} BossPod;

// Boss Flagship
typedef struct {
    bool active;
    u8 stage;           // 1, 2, or 3
    BossPhase phase;
    fixed_t x, y, base_x;
    s16 hp, max_hp;
    u16 wave_timer;
    u16 shoot_timer;
    u8 flash_timer;
    u8 death_timer;
    BossPod pods[MAX_BOSS_PODS];
} Boss;

// Power-Up Capsule
typedef struct {
    bool active;
    PowerUpType type;
    fixed_t x, y, vy;
} Capsule;

// Visual Spark/Debris Particle
typedef struct {
    bool active;
    fixed_t x, y, vx, vy;
    u8 life, max_life;
    u8 tile;
    u8 pal;
} Particle;

// Screen-Clearing Nova Bomb Shockwave
typedef struct {
    bool active;
    fixed_t x, y;
    u8 radius;
    u8 max_radius;
} NovaBomb;

#ifdef __cplusplus
extern "C" {
#endif

void game_init(void);
void game_update(void);
void game_render(void);

#ifdef __cplusplus
}
#endif

#endif // GAME_H
