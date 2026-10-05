#ifndef GAME_H
#define GAME_H

#include "types.h"
#include "assets.h"
#include "save.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_ENEMIES    12
#define MAX_PARTICLES  24
#define MOVE_SPEED     2    // Pixels moved per frame during 16px step (8 frames total)
#define PUSH_DELAY     6    // Frames player pushes against boulder before it rolls

typedef enum {
    ENEMY_NONE,
    ENEMY_SNAKE,
    ENEMY_SPIDER
} EnemyType;

typedef struct {
    EnemyType type;
    s16  tile_x;
    s16  tile_y;
    s16  pixel_x;
    s16  pixel_y;
    s16  target_tile_x;
    s16  target_tile_y;
    Direction dir;
    bool active;
    bool moving;
    u8   anim_frame;
    u8   move_timer;
} Enemy;

typedef struct {
    bool active;
    s16  x;
    s16  y;
    s8   vx;
    s8   vy;
    u8   type; // 0 = sparkle, 1 = dust, 2 = squash splat, 3 = impact shockwave, 4 = footstep puff
    u8   life;
    u8   max_life;
    u8   frame;
} Particle;

typedef struct {
    s16  tile_x;
    s16  tile_y;
    s16  pixel_x;
    s16  pixel_y;
    s16  target_tile_x;
    s16  target_tile_y;
    Direction facing;
    bool moving;
    bool pushing;
    u8   push_timer;
    Direction push_dir;
    s16  push_x;
    s16  push_y;
    
    u8   health;
    u8   max_health;
    u8   invuln_timer;
    bool squashed;
    bool has_key;
    
    u16  diamonds_collected;
    u16  diamond_quota;
    u32  score;
    u8   walk_anim_timer;
    u8   walk_frame;
} Player;

typedef struct {
    const char* name;
    u8   width;
    u8   height;
    u8   spawn_x;
    u8   spawn_y;
    u8   diamond_quota;
    u8   total_diamonds;
    const u8* map;
    u8   enemy_count;
    Enemy initial_enemies[MAX_ENEMIES];
} LevelDef;

void game_init(void);
void game_update(void);
void game_draw(void);
void game_load_level(u8 level_idx);
void game_restart_level(void);

#ifdef __cplusplus
}
#endif

#endif // GAME_H
