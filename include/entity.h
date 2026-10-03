#ifndef ENTITY_H
#define ENTITY_H

#include "types.h"

#define MAX_PROJECTILES 6

typedef struct {
    fixed_t x, y;
    fixed_t vx, vy;
    bool active;
    bool is_missile;
    s8 dir; // -1 left, 1 right, 0 up
} Projectile;

typedef struct {
    fixed_t x, y;
    fixed_t vx, vy;
    s16 health;
    s16 max_health;
    s16 missiles;
    s16 max_missiles;
    bool has_missile_upgrade;
    bool missile_selected;
    bool is_drone;      // Nano-drone / Morph ball mode
    bool on_ground;
    bool facing_right;
    bool touching_wall_left;
    bool touching_wall_right;
    u8 invuln_timer;
    u8 anim_timer;
} Player;

typedef struct {
    fixed_t x, y;
    fixed_t vx, vy;
    s8 type;       // 1=crawler, 2=boss
    s16 health;
    bool active;
    s8 dir;
    u8 state_timer;
} Enemy;

typedef struct {
    s16 x, y;
    bool active;
    u8 type;       // 1=missile upgrade
} Item;

#ifdef __cplusplus
extern "C" {
#endif

void entities_init(void);
void entities_reset_room(void);
void entities_update(void);
void entities_render(void);

const Player* entity_get_player(void);
bool entity_is_game_won(void);

#ifdef __cplusplus
}
#endif

#endif // ENTITY_H
