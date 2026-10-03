#include "game.h"
#include "graphics.h"
#include "input.h"
#include "audio.h"
#include "assets.h"
#include "save.h"

// 32-step sine wave lookup (-16 to +16)
static const s8 s_sin32[32] = {
      0,   3,   6,   9,  11,  13,  15,  16,
     16,  15,  13,  11,   9,   6,   3,   0,
     -3,  -6,  -9, -11, -13, -15, -16, -16,
    -15, -13, -11,  -9,  -6,  -3,   0,   0
};

// PRNG state
static u32 s_rng_state = 0x9A4B3C2D;
static u32 rng_next(void) {
    s_rng_state = s_rng_state * 1664525 + 1013904223;
    return s_rng_state;
}

// Game Global State
static GameState s_state = STATE_TITLE;
static u32 s_score = 0;
static u32 s_high_score = 10000;
static u8  s_stage = 1;
static u8  s_max_stage = 1;
static u16 s_stage_timer = 0;
static u16 s_state_timer = 0;

static s16 s_bg1_vofs = 0;
static s16 s_bg2_vofs = 0;

// Entities
static Player s_player;
static PlayerBullet s_player_bullets[MAX_PLAYER_BULLETS];
static Enemy s_enemies[MAX_ENEMIES];
static EnemyBullet s_enemy_bullets[MAX_ENEMY_BULLETS];
static Boss s_boss;
static Capsule s_capsules[MAX_CAPSULES];
static Particle s_particles[MAX_PARTICLES];
static NovaBomb s_bomb;

// Forward declarations
static void init_sidebar_hud(void);
static void start_game(u8 stage);
static void spawn_enemy(EnemyType type, fixed_t x, fixed_t y, fixed_t vx, fixed_t vy, s16 hp);
static void spawn_enemy_bullet(fixed_t x, fixed_t y, fixed_t vx, fixed_t vy, u8 type);
static void spawn_particles(fixed_t x, fixed_t y, u8 count, u8 pal);
static void spawn_capsule(fixed_t x, fixed_t y, PowerUpType type);
static void init_boss_encounter(u8 stage);
static void update_boss_ai(void);

// =========================================================================
// Initialization
// =========================================================================

void game_init(void) {
    save_init();
    s_high_score = save_get_high_score();
    s_max_stage = save_get_max_stage();

    s_state = STATE_TITLE;
    s_state_timer = 0;
    s_stage = 1;

    audio_play_bgm(BGM_TITLE);
    init_sidebar_hud();
}

static void init_sidebar_hud(void) {
    // 1. Draw Left Sidebar Bezel (Columns 0..4, x = 0..39)
    for (u8 y = 0; y < 20; y++) {
        for (u8 x = 0; x < 4; x++) {
            graphics_set_bg0_tile(x, y, TILE_BEZEL_PANEL, 0);
        }
        graphics_set_bg0_tile(4, y, TILE_BORDER_VERT, 0); // Boundary rail
    }

    // 2. Draw Right Sidebar Bezel (Columns 25..29, x = 200..239)
    for (u8 y = 0; y < 20; y++) {
        graphics_set_bg0_tile(25, y, TILE_BORDER_VERT, 0); // Boundary rail
        for (u8 x = 26; x < 30; x++) {
            graphics_set_bg0_tile(x, y, TILE_BEZEL_PANEL, 0);
        }
    }

    // 3. Clear Playfield on BG0 (Columns 5..24, x = 40..199)
    for (u8 y = 0; y < 20; y++) {
        for (u8 x = 5; x < 25; x++) {
            graphics_set_bg0_tile(x, y, TILE_EMPTY, 0);
        }
    }

    // 4. Populate BG1 (Foreground Debris/Space Corridor)
    for (u8 y = 0; y < 32; y++) {
        for (u8 x = 0; x < 32; x++) {
            graphics_set_bg1_tile(x, y, TILE_EMPTY, 1);
        }
    }
    // Scatter structural girders and debris in BG1
    graphics_set_bg1_tile(7, 4, TILE_GIRDER_1, 1);
    graphics_set_bg1_tile(8, 4, TILE_GIRDER_2, 1);
    graphics_set_bg1_tile(18, 12, TILE_DEBRIS_1, 1);
    graphics_set_bg1_tile(12, 20, TILE_DEBRIS_2, 1);
    graphics_set_bg1_tile(22, 28, TILE_DEBRIS_3, 1);

    // 5. Populate BG2 (Deep Space Starfield & Nebulae)
    for (u8 y = 0; y < 32; y++) {
        for (u8 x = 0; x < 32; x++) {
            u8 rnd = (x * 7 + y * 13) % 23;
            u16 star_tile = TILE_EMPTY;
            if (rnd == 1) star_tile = TILE_STAR_DIM;
            else if (rnd == 5) star_tile = TILE_STAR_MED;
            else if (rnd == 11) star_tile = TILE_STAR_BRIGHT;
            else if (rnd == 17) star_tile = TILE_NEBULA_1;
            else if (rnd == 21) star_tile = TILE_NEBULA_2;

            graphics_set_bg2_tile(x, y, star_tile, 2);
        }
    }

    // 6. Sidebar Labels
    graphics_print_text(0, 1, "SCORE", 0);
    graphics_print_num(0, 2, s_score, 6, 0);

    graphics_print_text(0, 4, "HIGH", 0);
    graphics_print_num(0, 5, s_high_score, 6, 0);

    graphics_print_text(0, 7, "STAGE", 0);
    graphics_print_num(1, 8, s_stage, 2, 0);

    graphics_print_text(0, 10, "LIVES", 0);

    // Right Sidebar
    graphics_print_text(26, 1, "WEAPON", 0);
    graphics_print_text(26, 2, "LVL:1", 0);

    graphics_print_text(26, 5, "BOMBS", 0);
    graphics_print_text(26, 6, "[B][B]", 0);

    graphics_print_text(26, 9, "SHIELD", 0);
    graphics_print_text(26, 10, "ACTIVE", 0);

    graphics_print_text(26, 13, "STATUS", 0);
    graphics_print_text(26, 14, "NORMAL", 0);
}

static void start_game(u8 stage) {
    s_stage = stage;
    s_stage_timer = 0;
    s_state_timer = 0;
    s_score = 0;
    s_state = STATE_PLAYING;

    assets_load_stage_theme(s_stage);
    audio_play_bgm(BGM_STAGE);

    // Initialize Player
    s_player.x = INT_TO_FP(112);
    s_player.y = INT_TO_FP(128);
    s_player.vx = 0;
    s_player.vy = 0;
    s_player.bank_frame = 0;
    s_player.weapon_lvl = 1;
    s_player.bombs = 2;
    s_player.lives = 3;
    s_player.shield_active = true;
    s_player.invuln_timer = 60;
    s_player.fire_cooldown = 0;
    s_player.is_dead = false;
    s_player.death_timer = 0;

    // Clear pools
    for (int i = 0; i < MAX_PLAYER_BULLETS; i++) s_player_bullets[i].active = false;
    for (int i = 0; i < MAX_ENEMIES; i++) s_enemies[i].active = false;
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) s_enemy_bullets[i].active = false;
    for (int i = 0; i < MAX_CAPSULES; i++) s_capsules[i].active = false;
    for (int i = 0; i < MAX_PARTICLES; i++) s_particles[i].active = false;

    s_boss.active = false;
    s_bomb.active = false;

    init_sidebar_hud();
}

// =========================================================================
// Spawners
// =========================================================================

static void spawn_enemy(EnemyType type, fixed_t x, fixed_t y, fixed_t vx, fixed_t vy, s16 hp) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!s_enemies[i].active) {
            s_enemies[i].active = true;
            s_enemies[i].type = type;
            s_enemies[i].x = x;
            s_enemies[i].y = y;
            s_enemies[i].vx = vx;
            s_enemies[i].vy = vy;
            s_enemies[i].hp = hp;
            s_enemies[i].max_hp = hp;
            s_enemies[i].timer = 0;
            s_enemies[i].shoot_cooldown = 40 + (rng_next() % 30);
            s_enemies[i].flash_timer = 0;
            break;
        }
    }
}

static void spawn_enemy_bullet(fixed_t x, fixed_t y, fixed_t vx, fixed_t vy, u8 type) {
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!s_enemy_bullets[i].active) {
            s_enemy_bullets[i].active = true;
            s_enemy_bullets[i].x = x;
            s_enemy_bullets[i].y = y;
            s_enemy_bullets[i].vx = vx;
            s_enemy_bullets[i].vy = vy;
            s_enemy_bullets[i].type = type;
            break;
        }
    }
}

static void spawn_particles(fixed_t x, fixed_t y, u8 count, u8 pal) {
    for (u8 c = 0; c < count; c++) {
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (!s_particles[i].active) {
                s_particles[i].active = true;
                s_particles[i].x = x;
                s_particles[i].y = y;
                s8 rvx = (s8)((rng_next() % 7) - 3);
                s8 rvy = (s8)((rng_next() % 7) - 3);
                s_particles[i].vx = INT_TO_FP(rvx) / 2;
                s_particles[i].vy = INT_TO_FP(rvy) / 2;
                s_particles[i].life = 12 + (rng_next() % 8);
                s_particles[i].max_life = s_particles[i].life;
                s_particles[i].tile = SPRITE_TILE_PARTICLE;
                s_particles[i].pal = pal;
                break;
            }
        }
    }
}

static void spawn_capsule(fixed_t x, fixed_t y, PowerUpType type) {
    for (int i = 0; i < MAX_CAPSULES; i++) {
        if (!s_capsules[i].active) {
            s_capsules[i].active = true;
            s_capsules[i].type = type;
            s_capsules[i].x = x;
            s_capsules[i].y = y;
            s_capsules[i].vy = INT_TO_FP(1) / 2; // Slow drift downwards
            break;
        }
    }
}

// =========================================================================
// Boss Initialization & AI
// =========================================================================

static void init_boss_encounter(u8 stage) {
    s_boss.active = true;
    s_boss.stage = stage;
    s_boss.phase = BOSS_PHASE_SHIELDED;
    s_boss.base_x = INT_TO_FP(104);
    s_boss.x = s_boss.base_x;
    s_boss.y = INT_TO_FP(16);
    s_boss.wave_timer = 0;
    s_boss.flash_timer = 0;
    s_boss.death_timer = 0;

    if (stage == 1) {
        // Aegis Dreadnought (160 HP Core, 2x 60 HP pods)
        s_boss.hp = 160;
        s_boss.max_hp = 160;
        s_boss.shoot_timer = 80;
        s_boss.pods[0].active = true;
        s_boss.pods[0].hp = 60;
        s_boss.pods[1].active = true;
        s_boss.pods[1].hp = 60;
    } else if (stage == 2) {
        // Titan Crust Crusher (240 HP Core, 2x 90 HP pods)
        s_boss.hp = 240;
        s_boss.max_hp = 240;
        s_boss.shoot_timer = 60;
        s_boss.pods[0].active = true;
        s_boss.pods[0].hp = 90;
        s_boss.pods[1].active = true;
        s_boss.pods[1].hp = 90;
    } else {
        // Mothership Core Nexus (360 HP Core, 2x 120 HP pods)
        s_boss.hp = 360;
        s_boss.max_hp = 360;
        s_boss.shoot_timer = 50;
        s_boss.pods[0].active = true;
        s_boss.pods[0].hp = 120;
        s_boss.pods[1].active = true;
        s_boss.pods[1].hp = 120;
    }

    audio_play_bgm(BGM_BOSS);
    graphics_trigger_shake(3, 15);
}

static void update_boss_ai(void) {
    if (!s_boss.active) return;

    if (s_boss.flash_timer > 0) s_boss.flash_timer--;

    if (s_boss.phase == BOSS_PHASE_DYING) {
        s_boss.death_timer++;
        if (s_boss.death_timer % 6 == 0) {
            sfx_enemy_explode();
            graphics_trigger_shake(2, 6);
            fixed_t rx = s_boss.x + INT_TO_FP(rng_next() % 32);
            fixed_t ry = s_boss.y + INT_TO_FP(rng_next() % 32);
            spawn_particles(rx, ry, 6, 2);
        }
        if (s_boss.death_timer >= 90) {
            s_boss.active = false;
            s_score += 10000;
            if (s_stage < 3) {
                s_state = STATE_STAGE_CLEAR;
                s_state_timer = 0;
                if (s_stage + 1 > s_max_stage) {
                    s_max_stage = s_stage + 1;
                    save_set_max_stage(s_max_stage);
                }
            } else {
                s_state = STATE_VICTORY;
                s_state_timer = 0;
                audio_play_bgm(BGM_VICTORY);
            }
        }
        return;
    }

    // Sinusoidal Hover
    s_boss.wave_timer = (s_boss.wave_timer + 1) & 31;
    s16 offset = s_sin32[s_boss.wave_timer]; // -16 to +16
    if (s_boss.phase == BOSS_PHASE_ENRAGED) offset = (offset * 3) / 2; // -24 to +24
    s_boss.x = s_boss.base_x + INT_TO_FP(offset);

    // Defense Pods follow core
    if (s_boss.pods[0].active) {
        s_boss.pods[0].x = s_boss.x - INT_TO_FP(20);
        s_boss.pods[0].y = s_boss.y + INT_TO_FP(6);
    }
    if (s_boss.pods[1].active) {
        s_boss.pods[1].x = s_boss.x + INT_TO_FP(36);
        s_boss.pods[1].y = s_boss.y + INT_TO_FP(6);
    }

    // Transition to Exposed when both pods destroyed
    if (s_boss.phase == BOSS_PHASE_SHIELDED && !s_boss.pods[0].active && !s_boss.pods[1].active) {
        s_boss.phase = BOSS_PHASE_EXPOSED;
        sfx_boss_explode();
        graphics_trigger_shake(3, 12);
    }

    // Enraged Transition at 50% HP
    if (s_boss.phase == BOSS_PHASE_EXPOSED && s_boss.hp <= (s_boss.max_hp / 2)) {
        s_boss.phase = BOSS_PHASE_ENRAGED;
        sfx_shield_break();
        graphics_trigger_shake(4, 15);
    }

    // Bolt and bullet firing
    if (s_boss.shoot_timer > 0) {
        s_boss.shoot_timer--;
    } else {
        s_boss.shoot_timer = (s_boss.phase == BOSS_PHASE_ENRAGED) ? 35 : 60;

        // Pod firing
        if (s_boss.pods[0].active) {
            spawn_enemy_bullet(s_boss.pods[0].x + INT_TO_FP(8), s_boss.pods[0].y + INT_TO_FP(16), 0, INT_TO_FP(2), 0);
        }
        if (s_boss.pods[1].active) {
            spawn_enemy_bullet(s_boss.pods[1].x + INT_TO_FP(8), s_boss.pods[1].y + INT_TO_FP(16), 0, INT_TO_FP(2), 0);
        }

        // Core firing: Aimed or Radial Spread
        if (s_boss.phase == BOSS_PHASE_EXPOSED || s_boss.phase == BOSS_PHASE_ENRAGED) {
            fixed_t cx = s_boss.x + INT_TO_FP(16);
            fixed_t cy = s_boss.y + INT_TO_FP(28);

            // 3-way or 5-way spread
            spawn_enemy_bullet(cx, cy, 0, INT_TO_FP(2), 1);
            spawn_enemy_bullet(cx, cy, -INT_TO_FP(1), INT_TO_FP(2), 1);
            spawn_enemy_bullet(cx, cy,  INT_TO_FP(1), INT_TO_FP(2), 1);

            if (s_boss.phase == BOSS_PHASE_ENRAGED) {
                spawn_enemy_bullet(cx, cy, -(INT_TO_FP(3)/2), INT_TO_FP(1), 0);
                spawn_enemy_bullet(cx, cy,  (INT_TO_FP(3)/2), INT_TO_FP(1), 0);
            }
        }
    }
}

// =========================================================================
// Game Update Loop
// =========================================================================

void game_update(void) {
    audio_update();
    graphics_update_shake();

    // Background Parallax Scrolling
    s_bg1_vofs -= 2; // Foreground debris scrolls fast
    s_bg2_vofs -= 1; // Starfield scrolls slower
    graphics_set_scroll(s_bg1_vofs, s_bg2_vofs);

    // State Machine
    if (s_state == STATE_TITLE) {
        s_state_timer++;

        // Stage Selection
        if (key_was_pressed(KEY_LEFT)) {
            if (s_stage > 1) {
                s_stage--;
                sfx_laser_fire();
            }
        }
        if (key_was_pressed(KEY_RIGHT)) {
            if (s_stage < s_max_stage) {
                s_stage++;
                sfx_laser_fire();
            }
        }

        if (key_was_pressed(KEY_START) || key_was_pressed(KEY_A)) {
            sfx_powerup_pickup();
            start_game(s_stage);
        }
        return;
    }

    if (s_state == STATE_PAUSED) {
        if (key_was_pressed(KEY_START)) {
            s_state = STATE_PLAYING;
        }
        return;
    }

    if (s_state == STATE_STAGE_CLEAR) {
        s_state_timer++;
        if (s_state_timer >= 120) {
            start_game(s_stage + 1);
        }
        return;
    }

    if (s_state == STATE_GAME_OVER) {
        s_state_timer++;
        if (s_score > s_high_score) {
            s_high_score = s_score;
            save_set_high_score(s_high_score);
        }
        if (s_state_timer >= 150 && (key_was_pressed(KEY_START) || key_was_pressed(KEY_A))) {
            s_state = STATE_TITLE;
            audio_play_bgm(BGM_TITLE);
            init_sidebar_hud();
        }
        return;
    }

    if (s_state == STATE_VICTORY) {
        s_state_timer++;
        if (s_score > s_high_score) {
            s_high_score = s_score;
            save_set_high_score(s_high_score);
        }
        if (s_state_timer >= 240 && (key_was_pressed(KEY_START) || key_was_pressed(KEY_A))) {
            s_state = STATE_TITLE;
            audio_play_bgm(BGM_TITLE);
            init_sidebar_hud();
        }
        return;
    }

    // Toggle Pause
    if (key_was_pressed(KEY_START)) {
        s_state = STATE_PAUSED;
        return;
    }

    // -------------------------------------------------------------
    // Player Flight & Controls
    // -------------------------------------------------------------
    if (!s_player.is_dead) {
        fixed_t speed = INT_TO_FP(2);

        s_player.bank_frame = 0; // Neutral

        if (key_is_down(KEY_LEFT)) {
            s_player.x -= speed;
            s_player.bank_frame = 1; // Bank left
        }
        if (key_is_down(KEY_RIGHT)) {
            s_player.x += speed;
            s_player.bank_frame = 2; // Bank right
        }
        if (key_is_down(KEY_UP)) {
            s_player.y -= speed;
        }
        if (key_is_down(KEY_DOWN)) {
            s_player.y += speed;
        }

        // Clamp to active playfield (x = 40..184, y = 16..144)
        if (s_player.x < INT_TO_FP(PLAYFIELD_X_MIN)) s_player.x = INT_TO_FP(PLAYFIELD_X_MIN);
        if (s_player.x > INT_TO_FP(PLAYFIELD_X_MAX - 16)) s_player.x = INT_TO_FP(PLAYFIELD_X_MAX - 16);
        if (s_player.y < INT_TO_FP(16)) s_player.y = INT_TO_FP(16);
        if (s_player.y > INT_TO_FP(144)) s_player.y = INT_TO_FP(144);

        if (s_player.invuln_timer > 0) s_player.invuln_timer--;

        // Weapon Fire (B Button or A Button)
        if (s_player.fire_cooldown > 0) {
            s_player.fire_cooldown--;
        } else if (key_is_down(KEY_B) || key_is_down(KEY_A)) {
            if (s_player.weapon_lvl == 1) {
                // Pulse Vulcan
                for (int i = 0; i < MAX_PLAYER_BULLETS; i++) {
                    if (!s_player_bullets[i].active) {
                        s_player_bullets[i].active = true;
                        s_player_bullets[i].x = s_player.x + INT_TO_FP(4);
                        s_player_bullets[i].y = s_player.y - INT_TO_FP(6);
                        s_player_bullets[i].vx = 0;
                        s_player_bullets[i].vy = -INT_TO_FP(5);
                        s_player_bullets[i].damage = 10;
                        s_player_bullets[i].type = 0;
                        break;
                    }
                }
                sfx_laser_fire();
                s_player.fire_cooldown = 8;
            } else if (s_player.weapon_lvl == 2) {
                // Twin Blaster
                for (int i = 0; i < MAX_PLAYER_BULLETS - 1; i++) {
                    if (!s_player_bullets[i].active && !s_player_bullets[i+1].active) {
                        s_player_bullets[i].active = true;
                        s_player_bullets[i].x = s_player.x;
                        s_player_bullets[i].y = s_player.y - INT_TO_FP(6);
                        s_player_bullets[i].vx = 0;
                        s_player_bullets[i].vy = -INT_TO_FP(6);
                        s_player_bullets[i].damage = 14;
                        s_player_bullets[i].type = 1;

                        s_player_bullets[i+1].active = true;
                        s_player_bullets[i+1].x = s_player.x + INT_TO_FP(8);
                        s_player_bullets[i+1].y = s_player.y - INT_TO_FP(6);
                        s_player_bullets[i+1].vx = 0;
                        s_player_bullets[i+1].vy = -INT_TO_FP(6);
                        s_player_bullets[i+1].damage = 14;
                        s_player_bullets[i+1].type = 1;
                        break;
                    }
                }
                sfx_laser_fire();
                s_player.fire_cooldown = 7;
            } else if (s_player.weapon_lvl == 3) {
                // Triple Spread
                for (int i = 0; i < MAX_PLAYER_BULLETS - 2; i++) {
                    if (!s_player_bullets[i].active && !s_player_bullets[i+1].active && !s_player_bullets[i+2].active) {
                        // Center
                        s_player_bullets[i].active = true;
                        s_player_bullets[i].x = s_player.x + INT_TO_FP(4);
                        s_player_bullets[i].y = s_player.y - INT_TO_FP(6);
                        s_player_bullets[i].vx = 0;
                        s_player_bullets[i].vy = -INT_TO_FP(5);
                        s_player_bullets[i].damage = 12;
                        s_player_bullets[i].type = 0;

                        // Left Angled
                        s_player_bullets[i+1].active = true;
                        s_player_bullets[i+1].x = s_player.x;
                        s_player_bullets[i+1].y = s_player.y - INT_TO_FP(4);
                        s_player_bullets[i+1].vx = -INT_TO_FP(1);
                        s_player_bullets[i+1].vy = -INT_TO_FP(5);
                        s_player_bullets[i+1].damage = 10;
                        s_player_bullets[i+1].type = 0;

                        // Right Angled
                        s_player_bullets[i+2].active = true;
                        s_player_bullets[i+2].x = s_player.x + INT_TO_FP(8);
                        s_player_bullets[i+2].y = s_player.y - INT_TO_FP(4);
                        s_player_bullets[i+2].vx = INT_TO_FP(1);
                        s_player_bullets[i+2].vy = -INT_TO_FP(5);
                        s_player_bullets[i+2].damage = 10;
                        s_player_bullets[i+2].type = 0;
                        break;
                    }
                }
                sfx_laser_fire();
                s_player.fire_cooldown = 8;
            } else {
                // Plasma Wave (Level 4)
                for (int i = 0; i < MAX_PLAYER_BULLETS; i++) {
                    if (!s_player_bullets[i].active) {
                        s_player_bullets[i].active = true;
                        s_player_bullets[i].x = s_player.x + INT_TO_FP(4);
                        s_player_bullets[i].y = s_player.y - INT_TO_FP(8);
                        s_player_bullets[i].vx = 0;
                        s_player_bullets[i].vy = -INT_TO_FP(6);
                        s_player_bullets[i].damage = 35;
                        s_player_bullets[i].type = 2; // Plasma Wave
                        break;
                    }
                }
                sfx_plasma_wave();
                s_player.fire_cooldown = 9;
            }
        }

        // Nova Bomb (R Button)
        if (key_was_pressed(KEY_R) && s_player.bombs > 0 && !s_bomb.active) {
            s_player.bombs--;
            s_bomb.active = true;
            s_bomb.x = s_player.x + INT_TO_FP(8);
            s_bomb.y = s_player.y + INT_TO_FP(8);
            s_bomb.radius = 8;
            s_bomb.max_radius = 90;

            graphics_trigger_shake(5, 18);
            sfx_nova_bomb();

            // Clear all enemy bullets into points
            for (int b = 0; b < MAX_ENEMY_BULLETS; b++) {
                if (s_enemy_bullets[b].active) {
                    s_enemy_bullets[b].active = false;
                    s_score += 50;
                    spawn_particles(s_enemy_bullets[b].x, s_enemy_bullets[b].y, 2, 3);
                }
            }

            // Damage all active enemies
            for (int e = 0; e < MAX_ENEMIES; e++) {
                if (s_enemies[e].active) {
                    s_enemies[e].hp -= 150;
                    s_enemies[e].flash_timer = 8;
                }
            }

            // Damage Boss
            if (s_boss.active && s_boss.phase != BOSS_PHASE_DYING) {
                if (s_boss.phase == BOSS_PHASE_SHIELDED) {
                    if (s_boss.pods[0].active) s_boss.pods[0].hp -= 50;
                    if (s_boss.pods[1].active) s_boss.pods[1].hp -= 50;
                } else {
                    s_boss.hp -= 80;
                    s_boss.flash_timer = 10;
                }
            }
        }
    } else {
        // Player death countdown
        s_player.death_timer++;
        if (s_player.death_timer >= 60) {
            if (s_player.lives > 0) {
                s_player.lives--;
                s_player.is_dead = false;
                s_player.x = INT_TO_FP(112);
                s_player.y = INT_TO_FP(128);
                s_player.shield_active = true;
                s_player.invuln_timer = 120;
            } else {
                s_state = STATE_GAME_OVER;
                s_state_timer = 0;
            }
        }
    }

    // Update Nova Bomb Shockwave
    if (s_bomb.active) {
        s_bomb.radius += 4;
        if (s_bomb.radius >= s_bomb.max_radius) {
            s_bomb.active = false;
        }
    }

    // -------------------------------------------------------------
    // Update Player Bullets
    // -------------------------------------------------------------
    for (int i = 0; i < MAX_PLAYER_BULLETS; i++) {
        if (s_player_bullets[i].active) {
            s_player_bullets[i].x += s_player_bullets[i].vx;
            s_player_bullets[i].y += s_player_bullets[i].vy;

            // Offscreen despawn
            if (s_player_bullets[i].y < INT_TO_FP(0) ||
                s_player_bullets[i].x < INT_TO_FP(PLAYFIELD_X_MIN) ||
                s_player_bullets[i].x > INT_TO_FP(PLAYFIELD_X_MAX)) {
                s_player_bullets[i].active = false;
                continue;
            }

            // Bullet vs Enemy Collision
            for (int e = 0; e < MAX_ENEMIES; e++) {
                if (s_enemies[e].active) {
                    s16 bx = FP_TO_INT(s_player_bullets[i].x);
                    s16 by = FP_TO_INT(s_player_bullets[i].y);
                    s16 ex = FP_TO_INT(s_enemies[e].x);
                    s16 ey = FP_TO_INT(s_enemies[e].y);
                    s16 ew = (s_enemies[e].type == ENEMY_GUNSHIP || s_enemies[e].type == ENEMY_CRUISER) ? 32 : 16;
                    s16 eh = 16;

                    if (bx >= ex && bx <= ex + ew && by >= ey && by <= ey + eh) {
                        s_player_bullets[i].active = false;
                        s_enemies[e].hp -= s_player_bullets[i].damage;
                        s_enemies[e].flash_timer = 4;
                        spawn_particles(s_player_bullets[i].x, s_player_bullets[i].y, 2, 0);

                        if (s_enemies[e].hp <= 0) {
                            s_enemies[e].active = false;
                            sfx_enemy_explode();
                            spawn_particles(s_enemies[e].x + INT_TO_FP(ew/2), s_enemies[e].y + INT_TO_FP(eh/2), 8, 1);
                            s_score += (ew > 16) ? 400 : 150;

                            // 15% Power-Up Capsule Drop Chance
                            u32 r = rng_next() % 100;
                            if (r < 15) {
                                PowerUpType ptype = PWR_WEAPON;
                                if (r < 7) ptype = PWR_WEAPON;
                                else if (r < 11) ptype = PWR_SHIELD;
                                else if (r < 14) ptype = PWR_BOMB;
                                else ptype = PWR_1UP;
                                spawn_capsule(s_enemies[e].x + INT_TO_FP(ew/2), s_enemies[e].y + INT_TO_FP(eh/2), ptype);
                            }
                        }
                        break;
                    }
                }
            }

            // Bullet vs Boss Collision
            if (s_boss.active && s_boss.phase != BOSS_PHASE_DYING) {
                s16 bx = FP_TO_INT(s_player_bullets[i].x);
                s16 by = FP_TO_INT(s_player_bullets[i].y);

                // Pod 0 collision
                if (s_boss.pods[0].active) {
                    s16 px = FP_TO_INT(s_boss.pods[0].x);
                    s16 py = FP_TO_INT(s_boss.pods[0].y);
                    if (bx >= px && bx <= px + 16 && by >= py && by <= py + 16) {
                        s_player_bullets[i].active = false;
                        s_boss.pods[0].hp -= s_player_bullets[i].damage;
                        spawn_particles(s_player_bullets[i].x, s_player_bullets[i].y, 3, 2);
                        if (s_boss.pods[0].hp <= 0) {
                            s_boss.pods[0].active = false;
                            sfx_boss_explode();
                            spawn_particles(s_boss.pods[0].x + INT_TO_FP(8), s_boss.pods[0].y + INT_TO_FP(8), 10, 2);
                        }
                        continue;
                    }
                }

                // Pod 1 collision
                if (s_boss.pods[1].active) {
                    s16 px = FP_TO_INT(s_boss.pods[1].x);
                    s16 py = FP_TO_INT(s_boss.pods[1].y);
                    if (bx >= px && bx <= px + 16 && by >= py && by <= py + 16) {
                        s_player_bullets[i].active = false;
                        s_boss.pods[1].hp -= s_player_bullets[i].damage;
                        spawn_particles(s_player_bullets[i].x, s_player_bullets[i].y, 3, 2);
                        if (s_boss.pods[1].hp <= 0) {
                            s_boss.pods[1].active = false;
                            sfx_boss_explode();
                            spawn_particles(s_boss.pods[1].x + INT_TO_FP(8), s_boss.pods[1].y + INT_TO_FP(8), 10, 2);
                        }
                        continue;
                    }
                }

                // Core Collision (only when exposed or enraged)
                if (s_boss.phase == BOSS_PHASE_EXPOSED || s_boss.phase == BOSS_PHASE_ENRAGED) {
                    s16 cx = FP_TO_INT(s_boss.x);
                    s16 cy = FP_TO_INT(s_boss.y);
                    if (bx >= cx + 4 && bx <= cx + 28 && by >= cy + 4 && by <= cy + 28) {
                        s_player_bullets[i].active = false;
                        s_boss.hp -= s_player_bullets[i].damage;
                        s_boss.flash_timer = 3;
                        spawn_particles(s_player_bullets[i].x, s_player_bullets[i].y, 3, 2);

                        if (s_boss.hp <= 0) {
                            s_boss.phase = BOSS_PHASE_DYING;
                            s_boss.death_timer = 0;
                            sfx_boss_explode();
                            graphics_trigger_shake(6, 30);
                        }
                    }
                }
            }
        }
    }

    // -------------------------------------------------------------
    // Update Enemies
    // -------------------------------------------------------------
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (s_enemies[i].active) {
            s_enemies[i].timer++;
            if (s_enemies[i].flash_timer > 0) s_enemies[i].flash_timer--;

            if (s_enemies[i].type == ENEMY_SCOUT) {
                // Swooping Sine Pattern
                s_enemies[i].y += s_enemies[i].vy;
                s8 sin_val = s_sin32[(s_enemies[i].timer >> 1) & 31];
                s_enemies[i].x += INT_TO_FP(sin_val) / 4;
            } else if (s_enemies[i].type == ENEMY_INTERCEPTOR) {
                // Fast diagonal dive
                s_enemies[i].x += s_enemies[i].vx;
                s_enemies[i].y += s_enemies[i].vy;
            } else if (s_enemies[i].type == ENEMY_GUNSHIP || s_enemies[i].type == ENEMY_CRUISER) {
                // Descend to y=30, then hover horizontally
                if (s_enemies[i].y < INT_TO_FP(30)) {
                    s_enemies[i].y += INT_TO_FP(1);
                } else {
                    s_enemies[i].x += s_enemies[i].vx;
                    if (s_enemies[i].x < INT_TO_FP(PLAYFIELD_X_MIN + 4) || s_enemies[i].x > INT_TO_FP(PLAYFIELD_X_MAX - 36)) {
                        s_enemies[i].vx = -s_enemies[i].vx;
                    }
                }
            }

            // Despawn off bottom screen
            if (s_enemies[i].y > INT_TO_FP(SCREEN_HEIGHT + 16)) {
                s_enemies[i].active = false;
                continue;
            }

            // Enemy Shooting
            if (s_enemies[i].shoot_cooldown > 0) {
                s_enemies[i].shoot_cooldown--;
            } else {
                s_enemies[i].shoot_cooldown = 60 + (rng_next() % 40);
                fixed_t ex = s_enemies[i].x + INT_TO_FP(8);
                fixed_t ey = s_enemies[i].y + INT_TO_FP(14);

                if (s_enemies[i].type == ENEMY_GUNSHIP || s_enemies[i].type == ENEMY_CRUISER) {
                    // 3-way radial burst
                    spawn_enemy_bullet(ex, ey, 0, INT_TO_FP(2), 0);
                    spawn_enemy_bullet(ex, ey, -INT_TO_FP(1), INT_TO_FP(2), 0);
                    spawn_enemy_bullet(ex, ey,  INT_TO_FP(1), INT_TO_FP(2), 0);
                } else {
                    // Aimed single shot toward player
                    fixed_t dx = s_player.x - ex;
                    fixed_t bvx = 0;
                    if (dx > INT_TO_FP(10)) bvx = INT_TO_FP(1);
                    else if (dx < -INT_TO_FP(10)) bvx = -INT_TO_FP(1);
                    spawn_enemy_bullet(ex, ey, bvx, INT_TO_FP(2), 0);
                }
            }

            // Ship vs Enemy Body Collision
            if (!s_player.is_dead && s_player.invuln_timer == 0) {
                s16 px = FP_TO_INT(s_player.x) + 4;
                s16 py = FP_TO_INT(s_player.y) + 4;
                s16 ex = FP_TO_INT(s_enemies[i].x);
                s16 ey = FP_TO_INT(s_enemies[i].y);
                s16 ew = (s_enemies[i].type == ENEMY_GUNSHIP || s_enemies[i].type == ENEMY_CRUISER) ? 32 : 16;

                if (px >= ex && px <= ex + ew && py >= ey && py <= ey + 16) {
                    // Collision impact!
                    if (s_player.shield_active) {
                        s_player.shield_active = false;
                        s_player.invuln_timer = 60;
                        sfx_shield_break();
                        graphics_trigger_shake(4, 10);
                    } else {
                        s_player.is_dead = true;
                        s_player.death_timer = 0;
                        sfx_player_death();
                        graphics_trigger_shake(6, 20);
                        spawn_particles(s_player.x + INT_TO_FP(8), s_player.y + INT_TO_FP(8), 12, 0);
                    }
                    s_enemies[i].hp -= 50;
                    if (s_enemies[i].hp <= 0) s_enemies[i].active = false;
                }
            }
        }
    }

    // -------------------------------------------------------------
    // Update Enemy Bullets
    // -------------------------------------------------------------
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (s_enemy_bullets[i].active) {
            s_enemy_bullets[i].x += s_enemy_bullets[i].vx;
            s_enemy_bullets[i].y += s_enemy_bullets[i].vy;

            // Despawn offscreen
            if (s_enemy_bullets[i].y > INT_TO_FP(SCREEN_HEIGHT + 8) ||
                s_enemy_bullets[i].x < INT_TO_FP(PLAYFIELD_X_MIN - 8) ||
                s_enemy_bullets[i].x > INT_TO_FP(PLAYFIELD_X_MAX + 8)) {
                s_enemy_bullets[i].active = false;
                continue;
            }

            // Bullet vs Player Precision Micro-Hitbox (4x4 pixels centered)
            if (!s_player.is_dead && s_player.invuln_timer == 0) {
                s16 bx = FP_TO_INT(s_enemy_bullets[i].x) + 4;
                s16 by = FP_TO_INT(s_enemy_bullets[i].y) + 4;
                s16 px = FP_TO_INT(s_player.x) + 6;
                s16 py = FP_TO_INT(s_player.y) + 6;

                if (bx >= px && bx <= px + 4 && by >= py && by <= py + 4) {
                    s_enemy_bullets[i].active = false;
                    spawn_particles(s_player.x + INT_TO_FP(8), s_player.y + INT_TO_FP(8), 6, 3);

                    if (s_player.shield_active) {
                        s_player.shield_active = false;
                        s_player.invuln_timer = 60;
                        sfx_shield_break();
                        graphics_trigger_shake(3, 10);
                    } else {
                        s_player.is_dead = true;
                        s_player.death_timer = 0;
                        sfx_player_death();
                        graphics_trigger_shake(6, 20);
                        spawn_particles(s_player.x + INT_TO_FP(8), s_player.y + INT_TO_FP(8), 12, 0);
                    }
                }
            }
        }
    }

    // -------------------------------------------------------------
    // Update Power-Up Capsules
    // -------------------------------------------------------------
    for (int i = 0; i < MAX_CAPSULES; i++) {
        if (s_capsules[i].active) {
            s_capsules[i].y += s_capsules[i].vy;

            if (s_capsules[i].y > INT_TO_FP(SCREEN_HEIGHT)) {
                s_capsules[i].active = false;
                continue;
            }

            // Capsule vs Player Pickup
            if (!s_player.is_dead) {
                s16 cx = FP_TO_INT(s_capsules[i].x);
                s16 cy = FP_TO_INT(s_capsules[i].y);
                s16 px = FP_TO_INT(s_player.x);
                s16 py = FP_TO_INT(s_player.y);

                if (cx >= px - 8 && cx <= px + 20 && cy >= py - 8 && cy <= py + 20) {
                    s_capsules[i].active = false;
                    sfx_powerup_pickup();

                    if (s_capsules[i].type == PWR_WEAPON) {
                        if (s_player.weapon_lvl < 4) s_player.weapon_lvl++;
                        s_score += 500;
                    } else if (s_capsules[i].type == PWR_BOMB) {
                        if (s_player.bombs < 5) s_player.bombs++;
                        s_score += 500;
                    } else if (s_capsules[i].type == PWR_SHIELD) {
                        s_player.shield_active = true;
                        s_score += 500;
                    } else if (s_capsules[i].type == PWR_1UP) {
                        if (s_player.lives < 9) s_player.lives++;
                        s_score += 2000;
                        sfx_extra_life();
                    }
                }
            }
        }
    }

    // -------------------------------------------------------------
    // Update Particles
    // -------------------------------------------------------------
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (s_particles[i].active) {
            s_particles[i].x += s_particles[i].vx;
            s_particles[i].y += s_particles[i].vy;
            s_particles[i].life--;
            if (s_particles[i].life == 0) {
                s_particles[i].active = false;
            }
        }
    }

    // -------------------------------------------------------------
    // Stage Director & Timed Wave Choreography
    // -------------------------------------------------------------
    s_stage_timer++;

    if (!s_boss.active) {
        // Stage 1 Waves (Orbital Perimeter)
        if (s_stage == 1) {
            if (s_stage_timer == 60) {
                for (int n = 0; n < 4; n++) {
                    spawn_enemy(ENEMY_SCOUT, INT_TO_FP(60 + n * 24), INT_TO_FP(-16 - n * 16), 0, INT_TO_FP(1), 10);
                }
            } else if (s_stage_timer == 220) {
                for (int n = 0; n < 4; n++) {
                    spawn_enemy(ENEMY_SCOUT, INT_TO_FP(160 - n * 24), INT_TO_FP(-16 - n * 16), 0, INT_TO_FP(1), 10);
                }
            } else if (s_stage_timer == 380) {
                spawn_enemy(ENEMY_INTERCEPTOR, INT_TO_FP(70), INT_TO_FP(-16), INT_TO_FP(1)/2, INT_TO_FP(2), 15);
                spawn_enemy(ENEMY_INTERCEPTOR, INT_TO_FP(150), INT_TO_FP(-16), -INT_TO_FP(1)/2, INT_TO_FP(2), 15);
            } else if (s_stage_timer == 550) {
                spawn_enemy(ENEMY_GUNSHIP, INT_TO_FP(104), INT_TO_FP(-24), INT_TO_FP(1)/2, INT_TO_FP(1), 60);
            } else if (s_stage_timer == 800) {
                for (int n = 0; n < 5; n++) {
                    spawn_enemy(ENEMY_SCOUT, INT_TO_FP(50 + n * 24), INT_TO_FP(-16 - n * 12), 0, INT_TO_FP(1), 10);
                }
            } else if (s_stage_timer == 1050) {
                spawn_enemy(ENEMY_GUNSHIP, INT_TO_FP(60), INT_TO_FP(-24), INT_TO_FP(1)/2, INT_TO_FP(1), 60);
                spawn_enemy(ENEMY_GUNSHIP, INT_TO_FP(140), INT_TO_FP(-24), -INT_TO_FP(1)/2, INT_TO_FP(1), 60);
            } else if (s_stage_timer == 1300) {
                init_boss_encounter(1);
            }
        }
        // Stage 2 Waves (Asteroid Infiltration)
        else if (s_stage == 2) {
            if (s_stage_timer == 60) {
                spawn_enemy(ENEMY_INTERCEPTOR, INT_TO_FP(80), INT_TO_FP(-16), INT_TO_FP(1)/2, INT_TO_FP(2), 20);
                spawn_enemy(ENEMY_INTERCEPTOR, INT_TO_FP(140), INT_TO_FP(-16), -INT_TO_FP(1)/2, INT_TO_FP(2), 20);
            } else if (s_stage_timer == 200) {
                spawn_enemy(ENEMY_GUNSHIP, INT_TO_FP(100), INT_TO_FP(-24), INT_TO_FP(1)/2, INT_TO_FP(1), 80);
            } else if (s_stage_timer == 420) {
                for (int n = 0; n < 4; n++) {
                    spawn_enemy(ENEMY_SCOUT, INT_TO_FP(60 + n * 28), INT_TO_FP(-16 - n * 14), 0, INT_TO_FP(1) + (INT_TO_FP(1)/4), 15);
                }
            } else if (s_stage_timer == 680) {
                spawn_enemy(ENEMY_CRUISER, INT_TO_FP(90), INT_TO_FP(-24), INT_TO_FP(1)/2, INT_TO_FP(1), 120);
            } else if (s_stage_timer == 1000) {
                for (int n = 0; n < 3; n++) {
                    spawn_enemy(ENEMY_INTERCEPTOR, INT_TO_FP(70 + n * 35), INT_TO_FP(-16), 0, INT_TO_FP(2), 25);
                }
            } else if (s_stage_timer == 1300) {
                init_boss_encounter(2);
            }
        }
        // Stage 3 Waves (Void Core Fortress)
        else {
            if (s_stage_timer == 60) {
                spawn_enemy(ENEMY_CRUISER, INT_TO_FP(90), INT_TO_FP(-24), INT_TO_FP(1)/2, INT_TO_FP(1), 150);
            } else if (s_stage_timer == 280) {
                for (int n = 0; n < 5; n++) {
                    spawn_enemy(ENEMY_INTERCEPTOR, INT_TO_FP(55 + n * 24), INT_TO_FP(-16 - n * 10), 0, INT_TO_FP(2), 20);
                }
            } else if (s_stage_timer == 520) {
                spawn_enemy(ENEMY_GUNSHIP, INT_TO_FP(60), INT_TO_FP(-24), INT_TO_FP(1)/2, INT_TO_FP(1), 80);
                spawn_enemy(ENEMY_GUNSHIP, INT_TO_FP(140), INT_TO_FP(-24), -INT_TO_FP(1)/2, INT_TO_FP(1), 80);
            } else if (s_stage_timer == 800) {
                spawn_enemy(ENEMY_CRUISER, INT_TO_FP(100), INT_TO_FP(-24), -INT_TO_FP(1)/2, INT_TO_FP(1), 180);
            } else if (s_stage_timer == 1200) {
                init_boss_encounter(3);
            }
        }
    } else {
        update_boss_ai();
    }
}

// =========================================================================
// Render Subsystem
// =========================================================================

void game_render(void) {
    graphics_oam_hide_all();

    // -------------------------------------------------------------
    // Title Screen Rendering
    // -------------------------------------------------------------
    if (s_state == STATE_TITLE) {
        graphics_print_text(7, 3, "SOLAR STRIKE", 0);
        graphics_print_text(8, 5, "VOID VIPER", 0);

        graphics_print_text(6, 8, "SELECT MISSION:", 0);

        char stage_str[16];
        stage_str[0] = '<';
        stage_str[1] = ' ';
        stage_str[2] = 'S';
        stage_str[3] = 'T';
        stage_str[4] = 'A';
        stage_str[5] = 'G';
        stage_str[6] = 'E';
        stage_str[7] = ' ';
        stage_str[8] = '0' + s_stage;
        stage_str[9] = ' ';
        stage_str[10] = '>';
        stage_str[11] = '\0';
        graphics_print_text(9, 10, stage_str, 0);

        if (s_stage == 1) graphics_print_text(7, 12, "[ORBIT PERIMETER]", 0);
        else if (s_stage == 2) graphics_print_text(6, 12, "[ASTEROID BELT]", 0);
        else graphics_print_text(6, 12, "[VOID CORE NEXUS]", 0);

        if ((s_state_timer & 32) == 0) {
            graphics_print_text(8, 15, "PRESS START", 0);
        } else {
            graphics_print_text(8, 15, "           ", 0);
        }

        // Show player ship showcase on title screen
        graphics_set_sprite(0, 112, 108, SPRITE_TILE_PLAYER_NEUTRAL, 0, 1, 0, false, false);
        graphics_oam_copy();
        return;
    }

    // -------------------------------------------------------------
    // Playing / Pause / Clear / Game Over / Victory
    // -------------------------------------------------------------

    // 1. Render Player Ship (Slot 0)
    if (!s_player.is_dead) {
        // Flashing when invulnerable
        if (s_player.invuln_timer == 0 || (s_player.invuln_timer & 4) == 0) {
            s16 px = FP_TO_INT(s_player.x);
            s16 py = FP_TO_INT(s_player.y);

            u16 tile = SPRITE_TILE_PLAYER_NEUTRAL;
            if (s_player.bank_frame == 1) tile = SPRITE_TILE_PLAYER_LEFT;
            else if (s_player.bank_frame == 2) tile = SPRITE_TILE_PLAYER_RIGHT;

            graphics_set_sprite(0, px, py, tile, 0, 1, 0, false, false); // 16x16, pal 0

            // Slot 1: Thruster Flame (8x8)
            graphics_set_sprite(1, px + 4, py + 14, SPRITE_TILE_THRUSTER, 0, 0, 0, false, false);

            // Slot 2: Shield Aura (16x16)
            if (s_player.shield_active) {
                graphics_set_sprite(2, px, py, SPRITE_TILE_SHIELD_AURA, 0, 1, 0, false, false);
            }
        }
    }

    // 2. Render Nova Bomb Shockwave (Slot 3)
    if (s_bomb.active) {
        s16 bx = FP_TO_INT(s_bomb.x) - 16;
        s16 by = FP_TO_INT(s_bomb.y) - 16;
        graphics_set_sprite(3, bx, by, SPRITE_TILE_BOMB_RING, 0, 2, 0, false, false); // 32x32
    }

    // 3. Render Player Bullets (Slots 4..19)
    for (int i = 0; i < MAX_PLAYER_BULLETS; i++) {
        if (s_player_bullets[i].active) {
            s16 bx = FP_TO_INT(s_player_bullets[i].x);
            s16 by = FP_TO_INT(s_player_bullets[i].y);
            u16 tile = SPRITE_TILE_WEAPON_VULCAN;
            if (s_player_bullets[i].type == 1) tile = SPRITE_TILE_WEAPON_LASER;
            else if (s_player_bullets[i].type == 2) tile = SPRITE_TILE_WEAPON_PLASMA;

            graphics_set_sprite(4 + i, bx, by, tile, 0, 0, 0, false, false); // 8x8, pal 0
        }
    }

    // 4. Render Enemies (Slots 20..31)
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (s_enemies[i].active) {
            s16 ex = FP_TO_INT(s_enemies[i].x);
            s16 ey = FP_TO_INT(s_enemies[i].y);
            u8 pal = (s_enemies[i].flash_timer > 0) ? 2 : 1; // Flash white/gold on hit

            if (s_enemies[i].type == ENEMY_GUNSHIP || s_enemies[i].type == ENEMY_CRUISER) {
                // 32x16 Wide Sprite (shape=1, size=2)
                graphics_set_sprite(20 + i, ex, ey, SPRITE_TILE_ENEMY_GUNSHIP, 1, 2, pal, false, false);
            } else if (s_enemies[i].type == ENEMY_INTERCEPTOR) {
                graphics_set_sprite(20 + i, ex, ey, SPRITE_TILE_ENEMY_INTER, 0, 1, pal, false, false);
            } else {
                graphics_set_sprite(20 + i, ex, ey, SPRITE_TILE_ENEMY_SCOUT, 0, 1, pal, false, false);
            }
        }
    }

    // 5. Render Enemy Bullets (Slots 32..63)
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (s_enemy_bullets[i].active) {
            s16 bx = FP_TO_INT(s_enemy_bullets[i].x);
            s16 by = FP_TO_INT(s_enemy_bullets[i].y);
            u16 tile = (s_enemy_bullets[i].type == 1) ? SPRITE_TILE_ENEMY_BOLT : SPRITE_TILE_ENEMY_BULLET;
            graphics_set_sprite(32 + i, bx, by, tile, 0, 0, 3, false, false); // 8x8, pal 3
        }
    }

    // 6. Render Power-Up Capsules (Slots 64..67)
    for (int i = 0; i < MAX_CAPSULES; i++) {
        if (s_capsules[i].active) {
            s16 cx = FP_TO_INT(s_capsules[i].x);
            s16 cy = FP_TO_INT(s_capsules[i].y);
            u16 tile = SPRITE_TILE_PWR_WEAPON;
            if (s_capsules[i].type == PWR_BOMB) tile = SPRITE_TILE_PWR_BOMB;
            else if (s_capsules[i].type == PWR_SHIELD) tile = SPRITE_TILE_PWR_SHIELD;
            else if (s_capsules[i].type == PWR_1UP) tile = SPRITE_TILE_PWR_1UP;

            // 16x8 Wide Capsule (shape=1, size=0)
            graphics_set_sprite(64 + i, cx, cy, tile, 1, 0, 3, false, false);
        }
    }

    // 7. Render Particles (Slots 68..83)
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (s_particles[i].active) {
            s16 px = FP_TO_INT(s_particles[i].x);
            s16 py = FP_TO_INT(s_particles[i].y);
            graphics_set_sprite(68 + i, px, py, s_particles[i].tile, 0, 0, s_particles[i].pal, false, false);
        }
    }

    // 8. Render Boss (Slots 84..99)
    if (s_boss.active) {
        u8 bpal = (s_boss.flash_timer > 0) ? 1 : 2;
        s16 bx = FP_TO_INT(s_boss.x);
        s16 by = FP_TO_INT(s_boss.y);

        // Core (32x32, shape=0, size=2)
        graphics_set_sprite(84, bx, by, SPRITE_TILE_BOSS_CORE, 0, 2, bpal, false, false);

        // Pod 0 (16x16)
        if (s_boss.pods[0].active) {
            s16 px = FP_TO_INT(s_boss.pods[0].x);
            s16 py = FP_TO_INT(s_boss.pods[0].y);
            graphics_set_sprite(85, px, py, SPRITE_TILE_BOSS_POD, 0, 1, 2, false, false);
        }
        // Pod 1 (16x16)
        if (s_boss.pods[1].active) {
            s16 px = FP_TO_INT(s_boss.pods[1].x);
            s16 py = FP_TO_INT(s_boss.pods[1].y);
            graphics_set_sprite(86, px, py, SPRITE_TILE_BOSS_POD, 0, 1, 2, false, false);
        }
    }

    // -------------------------------------------------------------
    // Update Sidebar Telemetry Numbers on BG0
    // -------------------------------------------------------------
    graphics_print_num(0, 2, s_score, 6, 0);
    graphics_print_num(0, 5, s_high_score, 6, 0);

    // Lives Icons (Column 0..3, Row 11)
    for (u8 l = 0; l < 4; l++) {
        u16 ltile = (l < s_player.lives) ? TILE_ARROW_RIGHT : TILE_BEZEL_PANEL;
        graphics_set_bg0_tile(l, 11, ltile, 0);
    }

    // Weapon Level
    if (s_player.weapon_lvl == 1) graphics_print_text(26, 2, "VULCAN", 0);
    else if (s_player.weapon_lvl == 2) graphics_print_text(26, 2, "TWIN  ", 0);
    else if (s_player.weapon_lvl == 3) graphics_print_text(26, 2, "SPREAD", 0);
    else graphics_print_text(26, 2, "PLASMA", 0);

    // Bombs count
    char bstr[6];
    bstr[0] = '[';
    bstr[1] = '0' + s_player.bombs;
    bstr[2] = ']';
    bstr[3] = '\0';
    graphics_print_text(26, 6, bstr, 0);

    // Shield status
    if (s_player.shield_active) {
        graphics_print_text(26, 10, "ACTIVE", 0);
    } else {
        graphics_print_text(26, 10, "DOWN! ", 0);
    }

    // Boss Radar / Status
    if (s_boss.active) {
        if (s_boss.phase == BOSS_PHASE_ENRAGED) {
            graphics_print_text(26, 14, "ENRAGE", 0);
        } else {
            graphics_print_text(26, 14, "ALERT!", 0);
        }
    } else {
        graphics_print_text(26, 14, "NORMAL", 0);
    }

    // Overlay Banners
    if (s_state == STATE_PAUSED) {
        graphics_print_text(9, 9, ">> PAUSED <<", 0);
    } else if (s_state == STATE_STAGE_CLEAR) {
        graphics_print_text(8, 8, "STAGE CLEAR!", 0);
        graphics_print_text(7, 10, "WARPING TO NEXT", 0);
    } else if (s_state == STATE_GAME_OVER) {
        graphics_print_text(8, 8, "GAME  OVER", 0);
        graphics_print_text(6, 10, "PRESS START TO RETRY", 0);
    } else if (s_state == STATE_VICTORY) {
        graphics_print_text(6, 7, "CAMPAIGN VICTORY!", 0);
        graphics_print_text(5, 9, "SOLAR SYSTEM LIBERATED", 0);
        graphics_print_text(6, 11, "FINAL SCORE: ", 0);
        graphics_print_num(18, 11, s_score, 6, 0);
    } else {
        // Clear overlay line in gameplay
        graphics_print_text(5, 7, "                    ", 0);
        graphics_print_text(5, 8, "                    ", 0);
        graphics_print_text(5, 9, "                    ", 0);
        graphics_print_text(5, 10, "                    ", 0);
        graphics_print_text(5, 11, "                    ", 0);
    }

    // Copy shadow OAM buffer to hardware via DMA
    graphics_oam_copy();
}
