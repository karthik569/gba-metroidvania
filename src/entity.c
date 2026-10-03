#include "entity.h"
#include "map.h"
#include "input.h"
#include "audio.h"
#include "graphics.h"
#include "assets.h"

static Player s_player;
static Projectile s_projectiles[MAX_PROJECTILES];
static Enemy s_enemy;
static Item s_item;
static bool s_game_won = false;

// Physics constants (8.8 Fixed point)
#define GRAVITY         (INT_TO_FP(1) / 4)       // 0.25 px/f^2
#define RUN_SPEED       (INT_TO_FP(2))           // 2.0 px/f
#define JUMP_FORCE      -(INT_TO_FP(4) + (INT_TO_FP(1) / 2)) // -4.5 px/f
#define WALL_JUMP_FORCE -(INT_TO_FP(4))          // -4.0 px/f
#define DRONE_SPEED     (INT_TO_FP(2) + (INT_TO_FP(1) / 2)) // 2.5 px/f

static bool is_tile_solid(u8 tile) {
    return (tile == TILE_SOLID_HULL || tile == TILE_RED_BARRIER || tile == TILE_AIRLOCK_DOOR);
}

void entities_init(void) {
    s_player.x = INT_TO_FP(32);
    s_player.y = INT_TO_FP(112);
    s_player.vx = 0;
    s_player.vy = 0;
    s_player.health = 99;
    s_player.max_health = 99;
    s_player.missiles = 0;
    s_player.max_missiles = 10;
    s_player.has_missile_upgrade = false;
    s_player.missile_selected = false;
    s_player.is_drone = false;
    s_player.on_ground = false;
    s_player.facing_right = true;
    s_player.jump_held = false;
    s_player.shoot_cooldown = 0;
    s_player.crouch_timer = 0;
    s_player.invuln_timer = 0;
    s_player.anim_timer = 0;

    for (int i = 0; i < MAX_PROJECTILES; i++) {
        s_projectiles[i].active = false;
    }

    s_game_won = false;
    entities_reset_room();
}

void entities_reset_room(void) {
    const Room* r = map_get_current_room();

    for (int i = 0; i < MAX_PROJECTILES; i++) {
        s_projectiles[i].active = false;
    }

    s_enemy.active = (r->enemy_type > 0);
    s_enemy.type = r->enemy_type;
    s_enemy.x = INT_TO_FP(r->enemy_x);
    s_enemy.y = INT_TO_FP(r->enemy_y);
    s_enemy.vx = (r->enemy_type == 1) ? (INT_TO_FP(1) / 2) : 0;
    s_enemy.vy = 0;
    s_enemy.dir = 1;
    s_enemy.health = (r->enemy_type == 2) ? 12 : 3;
    s_enemy.state_timer = 0;

    s_item.active = (r->item_type > 0 && !s_player.has_missile_upgrade);
    s_item.type = r->item_type;
    s_item.x = r->item_x;
    s_item.y = r->item_y;
}

static void fire_projectile(void) {
    bool shoot_missile = s_player.missile_selected && (s_player.missiles > 0);

    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!s_projectiles[i].active) {
            s_projectiles[i].active = true;
            s_projectiles[i].is_missile = shoot_missile;

            s16 px = FP_TO_INT(s_player.x);
            s16 py = FP_TO_INT(s_player.y);

            if (key_is_down(KEY_UP)) {
                // Shoot upwards
                s_projectiles[i].x = INT_TO_FP(px + 4);
                s_projectiles[i].y = INT_TO_FP(py - 4);
                s_projectiles[i].vx = 0;
                s_projectiles[i].vy = -INT_TO_FP(5);
                s_projectiles[i].dir = 0;
            } else {
                // Shoot horizontally
                if (s_player.facing_right) {
                    s_projectiles[i].x = INT_TO_FP(px + 12);
                    s_projectiles[i].vx = INT_TO_FP(5);
                    s_projectiles[i].dir = 1;
                } else {
                    s_projectiles[i].x = INT_TO_FP(px - 4);
                    s_projectiles[i].vx = -INT_TO_FP(5);
                    s_projectiles[i].dir = -1;
                }
                s_projectiles[i].y = INT_TO_FP(py + 4);
                s_projectiles[i].vy = 0;
            }

            if (shoot_missile) {
                s_player.missiles--;
                sfx_missile();
            } else {
                sfx_laser();
            }
            break;
        }
    }
}

void entities_update(void) {
    if (s_player.invuln_timer > 0) s_player.invuln_timer--;
    if (s_player.shoot_cooldown > 0) s_player.shoot_cooldown--;
    s_player.anim_timer++;

    // -------------------------------------------------------------
    // 1. Player Input & Controls
    // -------------------------------------------------------------
    // Toggle Missile Mode with R or Select
    if (s_player.has_missile_upgrade && (key_was_pressed(KEY_R) || key_was_pressed(KEY_SELECT))) {
        s_player.missile_selected = !s_player.missile_selected;
    }

    // Morph / Nano-Drone Form Toggle (Down to morph, Up to unmorph)
    if (!s_player.is_drone) {
        if (key_was_pressed(KEY_DOWN) || (key_is_down(KEY_DOWN) && s_player.on_ground && ++s_player.crouch_timer > 10)) {
            s_player.is_drone = true;
            s_player.crouch_timer = 0;
        }
    } else {
        if (key_was_pressed(KEY_UP) || (key_is_down(KEY_UP) && ++s_player.crouch_timer > 5)) {
            s16 px = FP_TO_INT(s_player.x);
            s16 py = FP_TO_INT(s_player.y);
            // Only unmorph if headroom is clear
            if (!is_tile_solid(map_get_tile(px + 4, py - 4))) {
                s_player.is_drone = false;
                s_player.crouch_timer = 0;
            }
        }
    }
    if (!key_is_down(KEY_DOWN) && !key_is_down(KEY_UP)) {
        s_player.crouch_timer = 0;
    }

    // Horizontal Movement
    fixed_t move_speed = s_player.is_drone ? DRONE_SPEED : RUN_SPEED;
    s_player.vx = 0;
    if (key_is_down(KEY_RIGHT)) {
        s_player.vx = move_speed;
        s_player.facing_right = true;
    } else if (key_is_down(KEY_LEFT)) {
        s_player.vx = -move_speed;
        s_player.facing_right = false;
    }

    // Jumping & Wall-Jumping
    bool a_pressed = key_was_pressed(KEY_A) || (key_is_down(KEY_A) && !s_player.jump_held);
    if (!s_player.is_drone && a_pressed) {
        if (s_player.on_ground) {
            s_player.vy = JUMP_FORCE;
            s_player.on_ground = false;
            s_player.jump_held = true;
            sfx_jump();
        } else if (s_player.touching_wall_left) {
            s_player.vy = WALL_JUMP_FORCE;
            s_player.vx = RUN_SPEED;
            s_player.facing_right = true;
            s_player.jump_held = true;
            sfx_jump();
        } else if (s_player.touching_wall_right) {
            s_player.vy = WALL_JUMP_FORCE;
            s_player.vx = -RUN_SPEED;
            s_player.facing_right = false;
            s_player.jump_held = true;
            sfx_jump();
        }
    }
    if (!key_is_down(KEY_A)) {
        s_player.jump_held = false;
        // Variable jump height: release early to cut upward velocity
        if (s_player.vy < -INT_TO_FP(2)) {
            s_player.vy = -INT_TO_FP(2);
        }
    }

    // Gravity
    s_player.vy += GRAVITY;
    if (s_player.vy > INT_TO_FP(4)) s_player.vy = INT_TO_FP(4);

    // Shooting: rapid-fire on hold or tap
    if (!s_player.is_drone && key_is_down(KEY_B)) {
        if (s_player.shoot_cooldown == 0) {
            fire_projectile();
            s_player.shoot_cooldown = 12; // 5 shots/sec
        }
    }

    // -------------------------------------------------------------
    // 2. Physics & Collision Resolution
    // -------------------------------------------------------------
    s16 px = FP_TO_INT(s_player.x);
    s16 py = FP_TO_INT(s_player.y);
    s16 h = s_player.is_drone ? 10 : 15;
    s16 w = 12;

    // Horizontal Collision
    fixed_t next_x = s_player.x + s_player.vx;
    s16 nx = FP_TO_INT(next_x);
    s_player.touching_wall_left = false;
    s_player.touching_wall_right = false;

    if (s_player.vx > 0) {
        if (is_tile_solid(map_get_tile(nx + w, py + 2)) || is_tile_solid(map_get_tile(nx + w, py + h - 1))) {
            s_player.vx = 0;
            s_player.touching_wall_right = true;
        }
    } else if (s_player.vx < 0) {
        if (is_tile_solid(map_get_tile(nx, py + 2)) || is_tile_solid(map_get_tile(nx, py + h - 1))) {
            s_player.vx = 0;
            s_player.touching_wall_left = true;
        }
    }
    s_player.x += s_player.vx;

    // Vertical Collision
    fixed_t next_y = s_player.y + s_player.vy;
    s16 ny = FP_TO_INT(next_y);
    px = FP_TO_INT(s_player.x);

    if (s_player.vy > 0) { // Falling downward
        if (is_tile_solid(map_get_tile(px + 2, ny + h)) || is_tile_solid(map_get_tile(px + w - 2, ny + h))) {
            s_player.y = INT_TO_FP((ny + h) / 8 * 8 - h);
            s_player.vy = 0;
            s_player.on_ground = true;
        } else {
            s_player.y += s_player.vy;
            s_player.on_ground = false;
        }
    } else if (s_player.vy < 0) { // Moving upward
        if (is_tile_solid(map_get_tile(px + 2, ny)) || is_tile_solid(map_get_tile(px + w - 2, ny))) {
            s_player.vy = 0;
            s_player.y = INT_TO_FP((ny / 8 + 1) * 8);
        } else {
            s_player.y += s_player.vy;
            s_player.on_ground = false;
        }
    }

    // Hazard acid check
    px = FP_TO_INT(s_player.x);
    py = FP_TO_INT(s_player.y);
    if (map_get_tile(px + 4, py + h) == TILE_HAZARD) {
        if (s_player.invuln_timer == 0) {
            s_player.health -= 15;
            s_player.vy = -INT_TO_FP(3);
            s_player.invuln_timer = 30;
            sfx_hit();
        }
    }

    // Save Console interaction (in Room 6)
    if (map_get_tile(px + 4, py + h) == TILE_SAVE_CONSOLE || map_get_tile(px + 4, py + h - 1) == TILE_SAVE_CONSOLE) {
        if (s_player.health < s_player.max_health || (s_player.has_missile_upgrade && s_player.missiles < s_player.max_missiles)) {
            s_player.health = s_player.max_health;
            if (s_player.has_missile_upgrade) s_player.missiles = s_player.max_missiles;
            sfx_pickup();
        }
    }

    // Item Pickup collision
    if (s_item.active) {
        if (px + 12 >= s_item.x && px <= s_item.x + 16 && py + h >= s_item.y && py <= s_item.y + 16) {
            s_item.active = false;
            s_player.has_missile_upgrade = true;
            s_player.missiles = 10;
            s_player.missile_selected = true;
            sfx_pickup();
        }
    }

    // -------------------------------------------------------------
    // 3. Room Boundary Transitions
    // -------------------------------------------------------------
    const Room* curr_r = map_get_current_room();
    if (px < 0 && curr_r->exit_left >= 0) {
        map_load_room(curr_r->exit_left);
        s_player.x = INT_TO_FP(SCREEN_WIDTH - 16);
        entities_reset_room();
    } else if (px > SCREEN_WIDTH - 8 && curr_r->exit_right >= 0) {
        map_load_room(curr_r->exit_right);
        s_player.x = INT_TO_FP(8);
        entities_reset_room();
    } else if (py < 0 && curr_r->exit_up >= 0) {
        map_load_room(curr_r->exit_up);
        s_player.y = INT_TO_FP(SCREEN_HEIGHT - 24);
        entities_reset_room();
    } else if (py > SCREEN_HEIGHT - 8 && curr_r->exit_down >= 0) {
        map_load_room(curr_r->exit_down);
        s_player.y = INT_TO_FP(8);
        entities_reset_room();
    }

    // -------------------------------------------------------------
    // 4. Projectiles & Destruction
    // -------------------------------------------------------------
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!s_projectiles[i].active) continue;

        s_projectiles[i].x += s_projectiles[i].vx;
        s_projectiles[i].y += s_projectiles[i].vy;

        s16 bx = FP_TO_INT(s_projectiles[i].x);
        s16 by = FP_TO_INT(s_projectiles[i].y);

        if (bx < 0 || bx >= SCREEN_WIDTH || by < 0 || by >= SCREEN_HEIGHT) {
            s_projectiles[i].active = false;
            continue;
        }

        u8 hit_tile = map_get_tile(bx, by);
        if (hit_tile == TILE_RED_BARRIER) {
            if (s_projectiles[i].is_missile) {
                u8 tx = bx / 8;
                u8 ty = by / 8;
                map_set_tile(tx, ty, TILE_EMPTY);
                sfx_missile();
            }
            s_projectiles[i].active = false;
            continue;
        } else if (is_tile_solid(hit_tile)) {
            s_projectiles[i].active = false;
            continue;
        }

        // Enemy collision
        if (s_enemy.active) {
            s16 ex = FP_TO_INT(s_enemy.x);
            s16 ey = FP_TO_INT(s_enemy.y);
            s16 ew = (s_enemy.type == 2) ? 32 : 16;
            s16 eh = (s_enemy.type == 2) ? 32 : 16;

            if (bx >= ex && bx <= ex + ew && by >= ey && by <= ey + eh) {
                s_projectiles[i].active = false;
                s16 dmg = s_projectiles[i].is_missile ? 4 : 1;
                s_enemy.health -= dmg;
                sfx_hit();

                if (s_enemy.health <= 0) {
                    s_enemy.active = false;
                    if (s_enemy.type == 2) {
                        sfx_boss_roar();
                        s_game_won = true;
                    }
                }
            }
        }
    }

    // -------------------------------------------------------------
    // 5. Enemy AI
    // -------------------------------------------------------------
    if (s_enemy.active) {
        if (s_enemy.type == 1) { // Crawler patrol
            s_enemy.x += s_enemy.vx * s_enemy.dir;
            s16 ex = FP_TO_INT(s_enemy.x);
            s16 ey = FP_TO_INT(s_enemy.y);

            // Wall or ledge check: turn around if wall ahead OR if floor drops off
            if (s_enemy.dir > 0) {
                if (is_tile_solid(map_get_tile(ex + 16, ey + 8)) || !is_tile_solid(map_get_tile(ex + 16, ey + 17))) {
                    s_enemy.dir = -1;
                }
            } else {
                if (is_tile_solid(map_get_tile(ex - 1, ey + 8)) || !is_tile_solid(map_get_tile(ex - 1, ey + 17))) {
                    s_enemy.dir = 1;
                }
            }

            // Hurt player on touch
            px = FP_TO_INT(s_player.x);
            py = FP_TO_INT(s_player.y);
            if (s_player.invuln_timer == 0 && px + w >= ex && px <= ex + 14 && py + h >= ey && py <= ey + 14) {
                s_player.health -= 10;
                s_player.invuln_timer = 40;
                s_player.vy = -INT_TO_FP(2);
                sfx_hit();
            }
        } else if (s_enemy.type == 2) { // Boss Sentinel
            s_enemy.state_timer++;
            s_enemy.x += (s_enemy.dir == 1) ? (INT_TO_FP(1) / 2) : -(INT_TO_FP(1) / 2);
            s16 ex = FP_TO_INT(s_enemy.x);
            if (ex > 180) s_enemy.dir = -1;
            if (ex < 60) s_enemy.dir = 1;

            px = FP_TO_INT(s_player.x);
            py = FP_TO_INT(s_player.y);
            s16 ey = FP_TO_INT(s_enemy.y);
            if (s_player.invuln_timer == 0 && px + w >= ex && px <= ex + 28 && py + h >= ey && py <= ey + 28) {
                s_player.health -= 20;
                s_player.invuln_timer = 50;
                s_player.vy = -INT_TO_FP(3);
                sfx_hit();
            }
        }
    }
}

void entities_render(void) {
    oam_clear();
    u8 sprite_id = 0;

    // 1. Render Player (16x16)
    s16 px = FP_TO_INT(s_player.x);
    s16 py = FP_TO_INT(s_player.y);

    if ((s_player.invuln_timer % 4) < 2) {
        u16 tile = SPRITE_TILE_PLAYER_IDLE;
        if (s_player.is_drone) {
            tile = SPRITE_TILE_PLAYER_DRONE;
        } else if (!s_player.on_ground) {
            tile = SPRITE_TILE_PLAYER_JUMP;
        } else if (s_player.vx != 0) {
            tile = ((s_player.anim_timer / 6) % 2 == 0) ? SPRITE_TILE_PLAYER_RUN : SPRITE_TILE_PLAYER_IDLE;
        }
        oam_set(sprite_id++, px, py, ATTR0_SQUARE, ATTR1_SIZE_16, tile, 0, !s_player.facing_right, false);
    }

    // 2. Render Projectiles (8x8)
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!s_projectiles[i].active) continue;
        s16 bx = FP_TO_INT(s_projectiles[i].x);
        s16 by = FP_TO_INT(s_projectiles[i].y);
        u16 tile = s_projectiles[i].is_missile ? SPRITE_TILE_MISSILE : SPRITE_TILE_BEAM;
        oam_set(sprite_id++, bx, by, ATTR0_SQUARE, ATTR1_SIZE_8, tile, 0, (s_projectiles[i].dir < 0), false);
    }

    // 3. Render Enemy
    if (s_enemy.active) {
        s16 ex = FP_TO_INT(s_enemy.x);
        s16 ey = FP_TO_INT(s_enemy.y);
        if (s_enemy.type == 1) { // Crawler (16x16)
            oam_set(sprite_id++, ex, ey, ATTR0_SQUARE, ATTR1_SIZE_16, SPRITE_TILE_CRAWLER, 0, (s_enemy.dir < 0), false);
        } else if (s_enemy.type == 2) { // Boss Sentinel (32x32)
            oam_set(sprite_id++, ex, ey, ATTR0_SQUARE, ATTR1_SIZE_32, SPRITE_TILE_BOSS, 0, false, false);
        }
    }

    // 4. Render Item Pickup (16x16)
    if (s_item.active) {
        oam_set(sprite_id++, s_item.x, s_item.y, ATTR0_SQUARE, ATTR1_SIZE_16, SPRITE_TILE_ITEM_MISSILE, 0, false, false);
    }

    oam_commit();
}

const Player* entity_get_player(void) {
    return &s_player;
}

bool entity_is_game_won(void) {
    return s_game_won;
}
