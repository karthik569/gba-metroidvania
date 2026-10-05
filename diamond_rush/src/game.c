#include "game.h"
#include "graphics.h"
#include "input.h"
#include "audio.h"
#include "assets.h"
#include "save.h"

static GameState s_state = STATE_TITLE;
static u8 s_current_level = 0;
static u8 s_selected_title_level = 0;
static u8 s_max_unlocked_level = 1;
static u32 s_high_score = 0;
static u32 s_total_diamonds_collected = 0;

static MetatileId s_map[LEVEL_HEIGHT][LEVEL_WIDTH];
static bool s_falling[LEVEL_HEIGHT][LEVEL_WIDTH];
static Player s_player;
static Enemy s_enemies[MAX_ENEMIES];
static u8 s_enemy_count = 0;
static Particle s_particles[MAX_PARTICLES];

static u16 s_state_timer = 0;
static u8  s_pause_selection = 0;
static bool s_plate_active = false;
static u16 s_level_time_frames = 0;

// Level 1: ANGKOR ENTRANCE (24x16)
static const u8 s_level1_map[LEVEL_HEIGHT * LEVEL_WIDTH] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,0,0,2,2,4,2,1,2,2,4,2,1,4,2,2,1,2,2,7,1,4,4,1,
    1,0,0,2,3,2,2,1,2,3,2,2,1,2,3,2,1,2,1,1,1,2,2,1,
    1,2,2,2,2,2,2,1,2,2,2,2,1,2,2,2,1,2,2,2,2,2,2,1,
    1,1,1,2,1,1,1,1,1,1,2,1,1,1,2,1,1,1,1,1,1,1,2,1,
    1,4,2,2,2,4,2,2,2,2,2,2,2,2,10,2,2,2,2,2,2,2,2,1,
    1,3,2,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,2,1,
    1,2,2,1,4,2,2,2,3,2,2,1,2,2,4,2,2,1,2,2,4,1,2,1,
    1,2,1,1,2,3,2,2,2,2,2,1,2,3,2,2,2,1,2,3,2,1,2,1,
    1,2,2,1,2,2,2,0,0,0,0,1,2,2,2,0,0,1,2,2,2,1,2,1,
    1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,2,1,2,1,
    1,4,2,2,2,2,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,
    1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,8,1,1,1,2,1,
    1,2,2,4,2,2,3,2,2,4,2,2,2,2,2,2,2,1,0,1,2,14,0,1,
    1,2,4,2,2,2,2,2,4,2,2,2,4,2,2,2,2,1,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

// Level 2: BOULDER CHASM (24x16)
static const u8 s_level2_map[LEVEL_HEIGHT * LEVEL_WIDTH] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,4,5,2,1,2,2,2,1,5,3,2,1,2,2,2,1,2,7,1,1,10,5,1,
    1,3,3,2,1,2,3,2,1,3,3,2,1,2,3,2,1,2,1,1,1,2,2,1,
    1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,1,
    1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,2,1,
    1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,
    1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,2,1,
    1,5,3,2,2,1,2,2,3,3,2,1,2,2,5,3,2,1,2,3,3,1,2,1,
    1,3,3,2,2,1,2,3,3,3,2,1,2,3,3,3,2,1,2,3,3,1,2,1,
    1,2,2,0,0,1,2,2,2,2,2,1,2,2,0,0,0,1,2,2,2,1,2,1,
    1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,2,1,
    1,2,2,2,5,2,2,2,2,2,5,2,2,2,2,2,5,2,2,2,2,2,2,1,
    1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,8,1,1,1,2,1,
    1,0,0,2,3,2,2,5,2,2,3,2,2,5,2,2,3,1,0,1,2,14,0,1,
    1,0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

// Level 3: SERPENT SANCTUARY (24x16)
static const u8 s_level3_map[LEVEL_HEIGHT * LEVEL_WIDTH] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,0,0,2,1,2,2,6,1,2,2,2,1,2,6,2,1,2,2,7,1,6,10,1,
    1,0,0,2,1,2,3,2,1,2,3,2,1,2,3,2,1,2,1,1,1,2,2,1,
    1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,1,
    1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,2,1,
    1,2,2,2,2,2,2,2,2,2,2,2,2,2,13,2,2,2,2,2,2,2,2,1,
    1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,2,1,
    1,2,2,2,3,2,2,1,2,2,3,2,2,1,2,2,3,2,2,1,2,2,2,1,
    1,2,2,2,2,2,12,1,2,2,2,2,2,1,2,2,2,2,2,1,2,2,2,1,
    1,2,11,11,11,1,2,1,2,0,0,0,2,1,2,11,11,1,2,1,2,1,
    1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,2,1,2,1,
    1,6,2,2,2,2,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,6,1,
    1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,8,1,1,1,2,1,
    1,2,2,6,2,2,3,2,2,6,2,2,2,2,2,2,2,1,0,1,2,14,0,1,
    1,2,6,2,2,2,2,2,6,2,2,2,6,2,2,2,2,1,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

// Level 4: CATACOMBS OF BAVARIA (24x16)
static const u8 s_level4_map[LEVEL_HEIGHT * LEVEL_WIDTH] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,0,0,2,3,3,3,3,1,2,2,2,1,6,5,4,1,2,2,7,1,6,10,1,
    1,0,0,2,2,2,2,2,1,2,3,2,1,3,3,3,1,2,1,1,1,2,2,1,
    1,2,2,1,1,1,1,2,1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,1,
    1,1,2,1,4,5,6,2,1,1,2,1,1,1,2,1,1,1,1,2,1,1,2,1,
    1,2,2,2,2,2,2,2,2,2,2,2,2,2,13,2,2,2,2,2,2,2,2,1,
    1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,2,1,
    1,6,3,2,2,1,2,2,3,3,2,1,2,2,6,3,2,1,2,3,3,1,2,1,
    1,3,3,2,2,1,2,12,3,3,2,1,2,3,3,3,2,1,2,3,3,1,2,1,
    1,2,2,0,0,1,2,2,2,2,2,1,2,2,0,0,0,1,2,2,2,1,2,1,
    1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,2,1,
    1,2,2,2,6,2,2,2,2,2,6,2,2,2,2,2,6,2,2,2,2,2,2,1,
    1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,8,1,1,1,2,1,
    1,2,2,6,3,2,2,6,2,2,3,2,2,6,2,2,3,1,0,1,2,14,0,1,
    1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

static const LevelDef s_levels[MAX_LEVELS] = {
    {
        .name = "ANGKOR ENTRANCE",
        .width = LEVEL_WIDTH,
        .height = LEVEL_HEIGHT,
        .spawn_x = 1,
        .spawn_y = 1,
        .diamond_quota = 8,
        .total_diamonds = 12,
        .map = s_level1_map,
        .enemy_count = 2,
        .initial_enemies = {
            { .type = ENEMY_SNAKE, .tile_x = 7, .tile_y = 9, .dir = DIR_RIGHT, .active = true },
            { .type = ENEMY_SNAKE, .tile_x = 15, .tile_y = 9, .dir = DIR_LEFT, .active = true }
        }
    },
    {
        .name = "BOULDER CHASM",
        .width = LEVEL_WIDTH,
        .height = LEVEL_HEIGHT,
        .spawn_x = 1,
        .spawn_y = 13,
        .diamond_quota = 12,
        .total_diamonds = 16,
        .map = s_level2_map,
        .enemy_count = 3,
        .initial_enemies = {
            { .type = ENEMY_SNAKE, .tile_x = 3, .tile_y = 9, .dir = DIR_RIGHT, .active = true },
            { .type = ENEMY_SNAKE, .tile_x = 14, .tile_y = 9, .dir = DIR_RIGHT, .active = true },
            { .type = ENEMY_SPIDER, .tile_x = 12, .tile_y = 3, .dir = DIR_DOWN, .active = true }
        }
    },
    {
        .name = "SERPENT SANCTUARY",
        .width = LEVEL_WIDTH,
        .height = LEVEL_HEIGHT,
        .spawn_x = 1,
        .spawn_y = 1,
        .diamond_quota = 14,
        .total_diamonds = 18,
        .map = s_level3_map,
        .enemy_count = 3,
        .initial_enemies = {
            { .type = ENEMY_SNAKE, .tile_x = 9, .tile_y = 9, .dir = DIR_RIGHT, .active = true },
            { .type = ENEMY_SPIDER, .tile_x = 6, .tile_y = 3, .dir = DIR_DOWN, .active = true },
            { .type = ENEMY_SPIDER, .tile_x = 17, .tile_y = 3, .dir = DIR_UP, .active = true }
        }
    },
    {
        .name = "BAVARIA CATACOMBS",
        .width = LEVEL_WIDTH,
        .height = LEVEL_HEIGHT,
        .spawn_x = 1,
        .spawn_y = 1,
        .diamond_quota = 16,
        .total_diamonds = 22,
        .map = s_level4_map,
        .enemy_count = 4,
        .initial_enemies = {
            { .type = ENEMY_SNAKE, .tile_x = 3, .tile_y = 9, .dir = DIR_RIGHT, .active = true },
            { .type = ENEMY_SNAKE, .tile_x = 14, .tile_y = 9, .dir = DIR_RIGHT, .active = true },
            { .type = ENEMY_SPIDER, .tile_x = 8, .tile_y = 3, .dir = DIR_DOWN, .active = true },
            { .type = ENEMY_SPIDER, .tile_x = 19, .tile_y = 3, .dir = DIR_UP, .active = true }
        }
    }
};

static void spawn_particle_vel(s16 x, s16 y, s8 vx, s8 vy, u8 type) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!s_particles[i].active) {
            s_particles[i].active = true;
            s_particles[i].x = x;
            s_particles[i].y = y;
            s_particles[i].vx = vx;
            s_particles[i].vy = vy;
            s_particles[i].type = type;
            s_particles[i].frame = 0;
            if (type == 0) s_particles[i].life = 18;       // Sparkle
            else if (type == 1) s_particles[i].life = 14;  // Soil / dust
            else if (type == 2) s_particles[i].life = 60;  // Goo splat
            else if (type == 3) s_particles[i].life = 14;  // Boulder impact shockwave
            else if (type == 4) s_particles[i].life = 8;   // Footstep puff
            s_particles[i].max_life = s_particles[i].life;
            break;
        }
    }
}

static void spawn_particle(s16 x, s16 y, u8 type) {
    spawn_particle_vel(x, y, 0, 0, type);
}

static void update_particles(void) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (s_particles[i].active) {
            if (s_particles[i].life > 0) {
                s_particles[i].life--;
                s_particles[i].frame++;
                s_particles[i].x += s_particles[i].vx;
                s_particles[i].y += s_particles[i].vy;
            } else {
                s_particles[i].active = false;
            }
        }
    }
}

static void player_take_damage(void) {
    if (s_player.invuln_timer > 0) return;

    if (s_player.health > 1) {
        s_player.health--;
        s_player.invuln_timer = 60; // 1 second invulnerability
        sfx_play_player_hurt();
        graphics_trigger_shake(3, 10);
    } else {
        s_player.health = 0;
        s_player.squashed = true;
        sfx_play_player_hurt();
        graphics_trigger_shake(6, 20);
        audio_play_bgm(BGM_GAME_OVER);
        s_state = STATE_GAME_OVER;
        s_state_timer = 0;
    }
}

void game_init(void) {
    SaveSlot slot;
    if (save_read_slot(0, &slot)) {
        s_max_unlocked_level = slot.max_level_unlocked;
        if (s_max_unlocked_level == 0 || s_max_unlocked_level > MAX_LEVELS) {
            s_max_unlocked_level = 1;
        }
        s_high_score = slot.high_score;
        s_total_diamonds_collected = slot.total_diamonds;
    } else {
        s_max_unlocked_level = 1;
        s_high_score = 0;
        s_total_diamonds_collected = 0;
    }

    s_selected_title_level = 0;
    s_current_level = 0;
    s_state = STATE_TITLE;
    s_state_timer = 0;
    audio_play_bgm(BGM_TITLE);
}

void game_load_level(u8 level_idx) {
    if (level_idx >= MAX_LEVELS) level_idx = 0;
    s_current_level = level_idx;
    const LevelDef* def = &s_levels[level_idx];

    // Select and load world theme
    LevelTheme theme = (level_idx >= 3) ? THEME_BAVARIA : THEME_ANGKOR;
    assets_load_theme(theme);
    graphics_populate_bg2_bedrock(theme);
    graphics_enable_alpha_blending(false);

    // Load Metatile Map
    for (int y = 0; y < LEVEL_HEIGHT; y++) {
        for (int x = 0; x < LEVEL_WIDTH; x++) {
            s_map[y][x] = (MetatileId)def->map[y * LEVEL_WIDTH + x];
            s_falling[y][x] = false;
            graphics_set_metatile(x, y, s_map[y][x]);
        }
    }

    // Initialize Player
    s_player.tile_x = def->spawn_x;
    s_player.tile_y = def->spawn_y;
    s_player.pixel_x = def->spawn_x * TILE_SIZE;
    s_player.pixel_y = def->spawn_y * TILE_SIZE;
    s_player.target_tile_x = def->spawn_x;
    s_player.target_tile_y = def->spawn_y;
    s_player.facing = DIR_DOWN;
    s_player.moving = false;
    s_player.pushing = false;
    s_player.push_timer = 0;
    s_player.health = 3;
    s_player.max_health = 3;
    s_player.invuln_timer = 0;
    s_player.squashed = false;
    s_player.has_key = false;
    s_player.diamonds_collected = 0;
    s_player.diamond_quota = def->diamond_quota;
    s_player.score = (level_idx == 0) ? 0 : s_player.score;
    s_player.walk_anim_timer = 0;
    s_player.walk_frame = 0;

    // Initialize Enemies
    s_enemy_count = def->enemy_count;
    for (int i = 0; i < s_enemy_count; i++) {
        s_enemies[i] = def->initial_enemies[i];
        s_enemies[i].pixel_x = s_enemies[i].tile_x * TILE_SIZE;
        s_enemies[i].pixel_y = s_enemies[i].tile_y * TILE_SIZE;
        s_enemies[i].target_tile_x = s_enemies[i].tile_x;
        s_enemies[i].target_tile_y = s_enemies[i].tile_y;
        s_enemies[i].moving = false;
        s_enemies[i].anim_frame = 0;
        s_enemies[i].move_timer = 0;
    }

    // Clear particles
    for (int i = 0; i < MAX_PARTICLES; i++) {
        s_particles[i].active = false;
    }

    s_plate_active = false;
    s_level_time_frames = 0;

    // Reset Camera
    s16 cam_x = s_player.pixel_x - (SCREEN_WIDTH / 2) + 8;
    s16 cam_y = s_player.pixel_y - (SCREEN_HEIGHT / 2) + 8;
    graphics_set_camera(cam_x, cam_y);

    s_state = STATE_LEVEL_INTRO;
    s_state_timer = 0;
    audio_stop_bgm();
}

void game_restart_level(void) {
    game_load_level(s_current_level);
}

// Check pressure plate triggers
static void update_mechanisms(void) {
    bool found_active_plate = false;

    for (int y = 0; y < LEVEL_HEIGHT; y++) {
        for (int x = 0; x < LEVEL_WIDTH; x++) {
            if (s_map[y][x] == META_PRESSURE_PLATE) {
                // If player is on plate
                if (s_player.tile_x == x && s_player.tile_y == y) {
                    found_active_plate = true;
                }
            } else if (s_map[y][x] == META_BOULDER) {
                // Check if boulder is sitting on a pressure plate location
                const LevelDef* def = &s_levels[s_current_level];
                if (def->map[y * LEVEL_WIDTH + x] == META_PRESSURE_PLATE) {
                    found_active_plate = true;
                }
            }
        }
    }

    if (found_active_plate != s_plate_active) {
        s_plate_active = found_active_plate;
        sfx_play_plate_click();

        // Lower / Raise barriers
        for (int y = 0; y < LEVEL_HEIGHT; y++) {
            for (int x = 0; x < LEVEL_WIDTH; x++) {
                const LevelDef* def = &s_levels[s_current_level];
                if (def->map[y * LEVEL_WIDTH + x] == META_BARRIER) {
                    if (s_plate_active) {
                        s_map[y][x] = META_EMPTY;
                        graphics_set_metatile(x, y, META_EMPTY);
                    } else {
                        // Only raise barrier if nothing is occupying the tile
                        bool occupied = false;
                        // Check player
                        if (s_player.tile_x == x && s_player.tile_y == y) occupied = true;
                        // Check if a boulder, gem, or other object was placed here
                        if (s_map[y][x] != META_EMPTY) occupied = true;
                        // Check enemies
                        for (int i = 0; i < s_enemy_count && !occupied; i++) {
                            if (s_enemies[i].active && s_enemies[i].tile_x == x && s_enemies[i].tile_y == y) {
                                occupied = true;
                            }
                        }
                        if (!occupied) {
                            s_map[y][x] = META_BARRIER;
                            graphics_set_metatile(x, y, META_BARRIER);
                        }
                    }
                }
            }
        }
    }
}

// Boulders & Diamonds Gravity and Roll Physics
static void update_physics(void) {
    // Process from bottom to top so falling objects don't cascade instantly in 1 frame
    for (int y = LEVEL_HEIGHT - 2; y >= 1; y--) {
        for (int x = 1; x < LEVEL_WIDTH - 1; x++) {
            MetatileId current = s_map[y][x];
            bool is_boulder = (current == META_BOULDER);
            bool is_gem = (current == META_DIAMOND_RED || current == META_DIAMOND_PURPLE || current == META_DIAMOND_GREEN);

            if (!is_boulder && !is_gem) continue;

            // 1. Direct Gravity Fall
            if (s_map[y + 1][x] == META_EMPTY) {
                // Check if player is directly underneath
                if (s_player.tile_x == x && s_player.tile_y == y + 1) {
                    if (s_falling[y][x]) {
                        // SQUASH PLAYER!
                        player_take_damage();
                    }
                } else {
                    // Check if an enemy is underneath
                    bool squashed_enemy = false;
                    for (int i = 0; i < s_enemy_count; i++) {
                        if (s_enemies[i].active && s_enemies[i].tile_x == x && s_enemies[i].tile_y == y + 1) {
                            s_enemies[i].active = false;
                            squashed_enemy = true;
                            sfx_play_snake_squash();
                            spawn_particle((x * TILE_SIZE) + 8, ((y + 1) * TILE_SIZE) + 8, 2);
                            s_player.score += 300;
                        }
                    }

                    // Move down
                    const LevelDef* def = &s_levels[s_current_level];
                    MetatileId vacated = (def->map[y * LEVEL_WIDTH + x] == META_PRESSURE_PLATE) ? META_PRESSURE_PLATE : META_EMPTY;
                    s_map[y + 1][x] = current;
                    s_map[y][x] = vacated;
                    s_falling[y + 1][x] = true;
                    s_falling[y][x] = false;

                    graphics_set_metatile(x, y, vacated);
                    graphics_set_metatile(x, y + 1, current);

                    if (squashed_enemy) {
                        graphics_trigger_shake(4, 12);
                    }
                    continue;
                }
            } else {
                // If it was falling and just landed on something solid
                if (s_falling[y][x]) {
                    s_falling[y][x] = false;
                    if (is_boulder) {
                        sfx_play_boulder_land();
                        graphics_trigger_shake(2, 6);
                        spawn_particle((x * TILE_SIZE) + 8, (y * TILE_SIZE) + 12, 3);
                    }
                }
            }

            // 2. Roll-off Mechanics (Sliding off other boulders or gems)
            MetatileId below = s_map[y + 1][x];
            bool below_is_round = (below == META_BOULDER || below == META_DIAMOND_RED || below == META_DIAMOND_PURPLE || below == META_DIAMOND_GREEN || below == META_WALL);

            if (below_is_round) {
                const LevelDef* def = &s_levels[s_current_level];
                MetatileId vacated = (def->map[y * LEVEL_WIDTH + x] == META_PRESSURE_PLATE) ? META_PRESSURE_PLATE : META_EMPTY;

                // Try rolling left: check (x-1, y) and (x-1, y+1)
                bool can_roll_left = (s_map[y][x - 1] == META_EMPTY && s_map[y + 1][x - 1] == META_EMPTY);
                if (can_roll_left && (s_player.tile_x != x - 1 || s_player.tile_y != y)) {
                    // Check if player is at the diagonal destination tile
                    if (s_player.tile_x == x - 1 && s_player.tile_y == y + 1) {
                        if (s_falling[y][x]) {
                            player_take_damage();
                        }
                    } else {
                        s_map[y + 1][x - 1] = current;
                        s_map[y][x] = vacated;
                        s_falling[y + 1][x - 1] = true;

                        graphics_set_metatile(x, y, vacated);
                        graphics_set_metatile(x - 1, y + 1, current);
                        sfx_play_boulder_roll();
                        continue;
                    }
                }

                // Try rolling right: check (x+1, y) and (x+1, y+1)
                bool can_roll_right = (s_map[y][x + 1] == META_EMPTY && s_map[y + 1][x + 1] == META_EMPTY);
                if (can_roll_right && (s_player.tile_x != x + 1 || s_player.tile_y != y)) {
                    // Check if player is at the diagonal destination tile
                    if (s_player.tile_x == x + 1 && s_player.tile_y == y + 1) {
                        if (s_falling[y][x]) {
                            player_take_damage();
                        }
                    } else {
                        s_map[y + 1][x + 1] = current;
                        s_map[y][x] = vacated;
                        s_falling[y + 1][x + 1] = true;

                        graphics_set_metatile(x, y, vacated);
                        graphics_set_metatile(x + 1, y + 1, current);
                        sfx_play_boulder_roll();
                        continue;
                    }
                }
            }
        }
    }
}

// Player Step Movement & Input
static void update_player(void) {
    if (s_player.invuln_timer > 0) {
        s_player.invuln_timer--;
    }

    // If currently interpolating movement between tiles
    if (s_player.moving) {
        s16 target_px = s_player.target_tile_x * TILE_SIZE;
        s16 target_py = s_player.target_tile_y * TILE_SIZE;

        if (s_player.pixel_x < target_px) s_player.pixel_x += MOVE_SPEED;
        else if (s_player.pixel_x > target_px) s_player.pixel_x -= MOVE_SPEED;

        if (s_player.pixel_y < target_py) s_player.pixel_y += MOVE_SPEED;
        else if (s_player.pixel_y > target_py) s_player.pixel_y -= MOVE_SPEED;

        // Walk animation cycle (smooth 3-frame cadence: Contact, Pass, Extension)
        s_player.walk_anim_timer++;
        if (s_player.walk_anim_timer >= 3) {
            s_player.walk_anim_timer = 0;
            s_player.walk_frame = (s_player.walk_frame + 1) % 3;
            if (s_player.walk_frame == 1) {
                spawn_particle_vel(s_player.pixel_x + 8, s_player.pixel_y + 12, 0, 0, 4);
            }
        }

        // Arrived at target tile
        if (s_player.pixel_x == target_px && s_player.pixel_y == target_py) {
            s_player.tile_x = s_player.target_tile_x;
            s_player.tile_y = s_player.target_tile_y;
            s_player.moving = false;

            // Trigger tile entered effects (e.g. Exit gate or Spikes)
            MetatileId here = s_map[s_player.tile_y][s_player.tile_x];
            if (here == META_EXIT_GATE) {
                if (s_player.diamonds_collected >= s_player.diamond_quota) {
                    s_state = STATE_LEVEL_CLEAR;
                    s_state_timer = 0;
                    audio_play_bgm(BGM_VICTORY);

                    // Unlock next level in SRAM
                    if (s_current_level + 2 > s_max_unlocked_level && s_current_level + 1 < MAX_LEVELS) {
                        s_max_unlocked_level = s_current_level + 2;
                    }
                    if (s_player.score > s_high_score) {
                        s_high_score = s_player.score;
                    }
                    s_total_diamonds_collected += s_player.diamonds_collected;

                    SaveSlot save;
                    save.max_level_unlocked = s_max_unlocked_level;
                    save.high_score = s_high_score;
                    save.total_diamonds = s_total_diamonds_collected;
                    save_write_slot(0, &save);
                    return;
                }
            } else if (here == META_SPIKES) {
                player_take_damage();
            }
        }
        return;
    }

    // Player is idle at a tile: read directional input
    s16 dx = 0;
    s16 dy = 0;

    if (key_is_down(KEY_RIGHT)) {
        dx = 1;
        s_player.facing = DIR_RIGHT;
    } else if (key_is_down(KEY_LEFT)) {
        dx = -1;
        s_player.facing = DIR_LEFT;
    } else if (key_is_down(KEY_DOWN)) {
        dy = 1;
        s_player.facing = DIR_DOWN;
    } else if (key_is_down(KEY_UP)) {
        dy = -1;
        s_player.facing = DIR_UP;
    }

    if (dx == 0 && dy == 0) {
        s_player.pushing = false;
        s_player.push_timer = 0;
        return;
    }

    s16 tx = s_player.tile_x + dx;
    s16 ty = s_player.tile_y + dy;

    if (tx < 0 || tx >= LEVEL_WIDTH || ty < 0 || ty >= LEVEL_HEIGHT) return;

    MetatileId target = s_map[ty][tx];

    // 1. Walk into Empty space or Open Door
    if (target == META_EMPTY || target == META_DOOR_OPEN || target == META_PRESSURE_PLATE) {
        s_player.moving = true;
        s_player.target_tile_x = tx;
        s_player.target_tile_y = ty;
        s_player.pushing = false;
        s_player.push_timer = 0;
        return;
    }

    // 2. Dig Dirt / Foliage
    if (target == META_DIRT) {
        s_map[ty][tx] = META_EMPTY;
        graphics_set_metatile(tx, ty, META_EMPTY);
        sfx_play_dig_dirt();
        spawn_particle_vel((tx * TILE_SIZE) + 8, (ty * TILE_SIZE) + 8, dx * 2, dy * 2, 1);
        spawn_particle_vel((tx * TILE_SIZE) + 8, (ty * TILE_SIZE) + 8, (s8)(dx - dy), (s8)(dy + dx), 1);
        spawn_particle_vel((tx * TILE_SIZE) + 8, (ty * TILE_SIZE) + 8, (s8)(dx + dy), (s8)(dy - dx), 1);

        s_player.moving = true;
        s_player.target_tile_x = tx;
        s_player.target_tile_y = ty;
        s_player.pushing = false;
        s_player.push_timer = 0;
        return;
    }

    // 3. Collect Diamonds (Red, Purple, Green)
    if (target == META_DIAMOND_RED || target == META_DIAMOND_PURPLE || target == META_DIAMOND_GREEN) {
        u16 pts = 100;
        if (target == META_DIAMOND_PURPLE) pts = 250;
        if (target == META_DIAMOND_GREEN) pts = 500;

        s_player.diamonds_collected++;
        s_player.score += pts;
        sfx_play_gem_pickup();
        spawn_particle((tx * TILE_SIZE) + 8, (ty * TILE_SIZE) + 8, 0);

        s_map[ty][tx] = META_EMPTY;
        graphics_set_metatile(tx, ty, META_EMPTY);

        if (s_player.diamonds_collected == s_player.diamond_quota) {
            sfx_play_exit_open();
            graphics_trigger_shake(2, 8);
        }

        s_player.moving = true;
        s_player.target_tile_x = tx;
        s_player.target_tile_y = ty;
        s_player.pushing = false;
        s_player.push_timer = 0;
        return;
    }

    // 4. Collect Key
    if (target == META_KEY) {
        s_player.has_key = true;
        s_player.score += 200;
        sfx_play_key_pickup();
        spawn_particle((tx * TILE_SIZE) + 8, (ty * TILE_SIZE) + 8, 0);

        s_map[ty][tx] = META_EMPTY;
        graphics_set_metatile(tx, ty, META_EMPTY);

        s_player.moving = true;
        s_player.target_tile_x = tx;
        s_player.target_tile_y = ty;
        return;
    }

    // 5. Unlock Door
    if (target == META_DOOR_LOCKED) {
        if (s_player.has_key) {
            s_map[ty][tx] = META_DOOR_OPEN;
            graphics_set_metatile(tx, ty, META_DOOR_OPEN);
            sfx_play_door_open();
            spawn_particle((tx * TILE_SIZE) + 8, (ty * TILE_SIZE) + 8, 0);
            s_player.score += 300;
        }
        return;
    }

    // 6. Open Treasure Chest
    if (target == META_CHEST) {
        s_map[ty][tx] = META_CHEST_OPEN;
        graphics_set_metatile(tx, ty, META_CHEST_OPEN);
        sfx_play_chest_open();
        s_player.diamonds_collected += 3;
        s_player.score += 500;
        for (int i = 0; i < 4; i++) {
            spawn_particle((tx * TILE_SIZE) + 4 + (i * 2), (ty * TILE_SIZE) + 4, 0);
        }
        return;
    }

    // 7. Step on Exit Gate
    if (target == META_EXIT_GATE) {
        if (s_player.diamonds_collected >= s_player.diamond_quota) {
            s_player.moving = true;
            s_player.target_tile_x = tx;
            s_player.target_tile_y = ty;
        }
        return;
    }

    // 8. Push Boulder Horizontally
    if (target == META_BOULDER && dy == 0 && dx != 0) {
        s16 dest_x = tx + dx;
        s16 dest_y = ty;

        // Destination behind boulder must be empty or a pressure plate
        if (dest_x >= 0 && dest_x < LEVEL_WIDTH && 
            (s_map[dest_y][dest_x] == META_EMPTY || s_map[dest_y][dest_x] == META_PRESSURE_PLATE)) {
            
            s_player.pushing = true;
            s_player.push_dir = (dx > 0) ? DIR_RIGHT : DIR_LEFT;
            s_player.push_timer++;

            if (s_player.push_timer >= PUSH_DELAY) {
                s_player.push_timer = 0;
                s_player.pushing = false;

                // Check if an enemy is in the destination — crush it
                for (int i = 0; i < s_enemy_count; i++) {
                    if (s_enemies[i].active && s_enemies[i].tile_x == dest_x && s_enemies[i].tile_y == dest_y) {
                        s_enemies[i].active = false;
                        sfx_play_snake_squash();
                        spawn_particle((dest_x * TILE_SIZE) + 8, (dest_y * TILE_SIZE) + 8, 2);
                        s_player.score += 300;
                        graphics_trigger_shake(4, 12);
                    }
                }

                // Move Boulder
                const LevelDef* def = &s_levels[s_current_level];
                MetatileId vacated = (def->map[ty * LEVEL_WIDTH + tx] == META_PRESSURE_PLATE) ? META_PRESSURE_PLATE : META_EMPTY;
                s_map[dest_y][dest_x] = META_BOULDER;
                s_map[ty][tx] = vacated;
                graphics_set_metatile(dest_x, dest_y, META_BOULDER);
                graphics_set_metatile(tx, ty, vacated);
                sfx_play_boulder_push();

                // Player moves into former boulder spot
                s_player.moving = true;
                s_player.target_tile_x = tx;
                s_player.target_tile_y = ty;
            }
        } else {
            s_player.pushing = false;
            s_player.push_timer = 0;
        }
        return;
    }

    // 9. Spikes
    if (target == META_SPIKES) {
        s_player.moving = true;
        s_player.target_tile_x = tx;
        s_player.target_tile_y = ty;
        return;
    }

    s_player.pushing = false;
    s_player.push_timer = 0;
}

// Enemy AI (Cobras and Spiders) — Smooth interpolated movement
static void update_enemies(void) {
    for (int i = 0; i < s_enemy_count; i++) {
        Enemy* e = &s_enemies[i];
        if (!e->active) continue;

        e->anim_frame++;

        // If currently interpolating between tiles
        if (e->moving) {
            s16 target_px = e->target_tile_x * TILE_SIZE;
            s16 target_py = e->target_tile_y * TILE_SIZE;

            if (e->pixel_x < target_px) e->pixel_x += MOVE_SPEED;
            else if (e->pixel_x > target_px) e->pixel_x -= MOVE_SPEED;

            if (e->pixel_y < target_py) e->pixel_y += MOVE_SPEED;
            else if (e->pixel_y > target_py) e->pixel_y -= MOVE_SPEED;

            // Arrived at target tile
            if (e->pixel_x == target_px && e->pixel_y == target_py) {
                e->tile_x = e->target_tile_x;
                e->tile_y = e->target_tile_y;
                e->moving = false;
            }
        } else {
            // Not moving — count down to next movement decision
            e->move_timer++;

            if (e->type == ENEMY_SNAKE) {
                // Snakes patrol horizontally
                if (e->move_timer >= 12) {
                    e->move_timer = 0;
                    s16 next_x = (e->dir == DIR_RIGHT) ? (e->tile_x + 1) : (e->tile_x - 1);

                    if (next_x >= 0 && next_x < LEVEL_WIDTH && s_map[e->tile_y][next_x] == META_EMPTY) {
                        e->target_tile_x = next_x;
                        e->target_tile_y = e->tile_y;
                        e->moving = true;
                    } else {
                        // Turn around
                        e->dir = (e->dir == DIR_RIGHT) ? DIR_LEFT : DIR_RIGHT;
                    }
                }
            } else if (e->type == ENEMY_SPIDER) {
                // Spiders crawl vertically
                if (e->move_timer >= 14) {
                    e->move_timer = 0;
                    s16 next_y = (e->dir == DIR_DOWN) ? (e->tile_y + 1) : (e->tile_y - 1);

                    if (next_y >= 0 && next_y < LEVEL_HEIGHT && s_map[next_y][e->tile_x] == META_EMPTY) {
                        e->target_tile_x = e->tile_x;
                        e->target_tile_y = next_y;
                        e->moving = true;
                    } else {
                        e->dir = (e->dir == DIR_DOWN) ? DIR_UP : DIR_DOWN;
                    }
                }
            }
        }

        // Collision with player: use pixel proximity for fair detection during interpolation
        // Both the enemy and the player may be mid-transition, so check pixel distance
        s16 dx = e->pixel_x - s_player.pixel_x;
        s16 dy = e->pixel_y - s_player.pixel_y;
        if (dx < 0) dx = -dx;
        if (dy < 0) dy = -dy;

        // Collision when within 10px on both axes (slightly less than a full tile for fairness)
        if (dx < 10 && dy < 10) {
            if (e->type == ENEMY_SNAKE) {
                sfx_play_snake_hiss();
            }
            player_take_damage();
        }
    }
}

void game_update(void) {
    graphics_update_shake();

    switch (s_state) {
        case STATE_TITLE:
            s_state_timer++;
            if (key_was_pressed(KEY_START) || key_was_pressed(KEY_A)) {
                s_current_level = s_selected_title_level;
                game_load_level(s_current_level);
            } else if (key_was_pressed(KEY_RIGHT)) {
                if (s_selected_title_level + 1 < s_max_unlocked_level) {
                    s_selected_title_level++;
                    sfx_play_plate_click();
                }
            } else if (key_was_pressed(KEY_LEFT)) {
                if (s_selected_title_level > 0) {
                    s_selected_title_level--;
                    sfx_play_plate_click();
                }
            }
            break;

        case STATE_LEVEL_INTRO:
            s_state_timer++;
            if (s_state_timer > 60 || key_was_pressed(KEY_START) || key_was_pressed(KEY_A)) {
                graphics_clear_bg0();
                s_state = STATE_PLAYING;
                s_state_timer = 0;
                audio_play_bgm(BGM_EXPEDITION);
            }
            break;

        case STATE_PLAYING:
            s_level_time_frames++;
            audio_update();

            if (key_was_pressed(KEY_START)) {
                s_state = STATE_PAUSED;
                s_pause_selection = 0;
                sfx_play_plate_click();
                return;
            }

            update_player();

            // Run physics and enemy logic every few frames
            if ((s_level_time_frames % 4) == 0) {
                update_physics();
            }
            if ((s_level_time_frames % 2) == 0) {
                update_enemies();
            }
            update_mechanisms();
            update_particles();

            // Update Camera to follow player smoothly
            {
                s16 target_cam_x = s_player.pixel_x - (SCREEN_WIDTH / 2) + 8;
                s16 target_cam_y = s_player.pixel_y - (SCREEN_HEIGHT / 2) + 8;
                s16 cur_cam_x = graphics_get_camera_x();
                s16 cur_cam_y = graphics_get_camera_y();

                // Smooth camera dampening
                cur_cam_x += (target_cam_x - cur_cam_x) / 4;
                cur_cam_y += (target_cam_y - cur_cam_y) / 4;
                graphics_set_camera(cur_cam_x, cur_cam_y);

                // Update Bavaria Catacombs torchlight / lantern lighting
                if (s_current_level >= 3) {
                    graphics_set_torchlight(true, s_player.pixel_x - cur_cam_x, s_player.pixel_y - cur_cam_y);
                } else {
                    graphics_set_torchlight(false, 0, 0);
                }
            }
            break;

        case STATE_PAUSED:
            graphics_set_torchlight(false, 0, 0);
            graphics_set_fade(0);
            if (key_was_pressed(KEY_UP)) {
                if (s_pause_selection > 0) s_pause_selection--;
                sfx_play_plate_click();
            } else if (key_was_pressed(KEY_DOWN)) {
                if (s_pause_selection < 2) s_pause_selection++;
                sfx_play_plate_click();
            } else if (key_was_pressed(KEY_A) || key_was_pressed(KEY_START)) {
                if (s_pause_selection == 0) {
                    // Resume
                    graphics_clear_bg0();
                    s_state = STATE_PLAYING;
                } else if (s_pause_selection == 1) {
                    // Restart Level
                    graphics_clear_bg0();
                    game_restart_level();
                } else if (s_pause_selection == 2) {
                    // Quit to Title
                    graphics_clear_bg0();
                    s_state = STATE_TITLE;
                    audio_play_bgm(BGM_TITLE);
                }
            } else if (key_was_pressed(KEY_B)) {
                graphics_clear_bg0();
                s_state = STATE_PLAYING;
            }
            break;

        case STATE_LEVEL_CLEAR:
            s_state_timer++;
            if (s_state_timer > 60 && (key_was_pressed(KEY_START) || key_was_pressed(KEY_A))) {
                graphics_clear_bg0();
                if (s_current_level + 1 < MAX_LEVELS) {
                    game_load_level(s_current_level + 1);
                } else {
                    s_state = STATE_VICTORY;
                    s_state_timer = 0;
                }
            }
            break;

        case STATE_GAME_OVER:
            s_state_timer++;
            if (s_state_timer > 45 && (key_was_pressed(KEY_START) || key_was_pressed(KEY_A))) {
                graphics_clear_bg0();
                game_restart_level();
            } else if (s_state_timer > 45 && key_was_pressed(KEY_B)) {
                graphics_clear_bg0();
                s_state = STATE_TITLE;
                audio_play_bgm(BGM_TITLE);
            }
            break;

        case STATE_VICTORY:
            s_state_timer++;
            if (s_state_timer > 60 && (key_was_pressed(KEY_START) || key_was_pressed(KEY_A))) {
                graphics_clear_bg0();
                s_state = STATE_TITLE;
                audio_play_bgm(BGM_TITLE);
            }
            break;
    }
}

void game_draw(void) {
    graphics_oam_hide_all();

    switch (s_state) {
        case STATE_TITLE:
            graphics_draw_box(2, 1, 26, 18, PAL_BG_UI);
            graphics_draw_title_logo(5, 2);

            // Shimmering diamond icon in center
            graphics_set_bg0_tile(14, 7, TILE_DIAMOND_ICON, PAL_BG_UI);

            // Level selection
            graphics_print_text(6, 9, "EXPEDITION: < ", PAL_BG_UI);
            graphics_print_num(20, 9, s_selected_title_level + 1, 1, PAL_BG_UI);
            graphics_print_text(22, 9, " >", PAL_BG_UI);

            // Level Name centered
            graphics_print_text(5, 11, s_levels[s_selected_title_level].name, PAL_BG_UI);

            // Pulsing "PRESS START / A"
            if ((s_state_timer & 32) == 0) {
                graphics_print_text(7, 13, "PRESS START / A", PAL_BG_UI);
            }

            graphics_print_text(6, 15, "RECORD: ", PAL_BG_UI);
            graphics_print_num(14, 15, s_high_score, 6, PAL_BG_UI);
            graphics_print_text(21, 15, "PTS", PAL_BG_UI);
            break;

        case STATE_LEVEL_INTRO:
            graphics_draw_box(4, 5, 22, 10, PAL_BG_UI);
            graphics_print_text(7, 7, s_levels[s_current_level].name, PAL_BG_UI);
            graphics_print_text(6, 9, "DIAMOND QUOTA: ", PAL_BG_UI);
            graphics_print_num(21, 9, s_levels[s_current_level].diamond_quota, 2, PAL_BG_UI);
            graphics_print_text(11, 12, "READY!", PAL_BG_UI);
            break;

        case STATE_PLAYING:
            // 1. Draw Top HUD Bar
            graphics_draw_hud(s_player.diamonds_collected, s_player.diamond_quota, 
                              s_player.health, s_player.has_key, s_player.score, s_current_level + 1);

            // 2. Draw Player Sprite (High-Fidelity 3-Frame Walk, Push Strain, Squash)
            {
                s16 cam_x = graphics_get_camera_x();
                s16 cam_y = graphics_get_camera_y();
                s16 screen_x = s_player.pixel_x - cam_x;
                s16 screen_y = s_player.pixel_y - cam_y;

                u16 sprite_tile = SPRITE_EXPLORER_D0;
                bool hflip = false;

                if (s_player.squashed) {
                    sprite_tile = SPRITE_EXPLORER_SQUASH;
                } else if (s_player.pushing) {
                    sprite_tile = (s_player.push_timer > 6) ? SPRITE_EXPLORER_PUSH1 : SPRITE_EXPLORER_PUSH0;
                    hflip = (s_player.push_dir == DIR_LEFT);
                } else if (!s_player.moving) {
                    if (s_player.facing == DIR_DOWN) sprite_tile = SPRITE_EXPLORER_D0;
                    else if (s_player.facing == DIR_UP) sprite_tile = SPRITE_EXPLORER_U0;
                    else {
                        sprite_tile = SPRITE_EXPLORER_S0;
                        hflip = (s_player.facing == DIR_LEFT);
                    }
                } else {
                    u8 step = s_player.walk_frame % 3; // 0: Contact, 1: Pass, 2: Extension
                    if (s_player.facing == DIR_DOWN) {
                        sprite_tile = SPRITE_EXPLORER_D1 + (step * 4);
                    } else if (s_player.facing == DIR_UP) {
                        sprite_tile = SPRITE_EXPLORER_U1 + (step * 4);
                    } else if (s_player.facing == DIR_RIGHT) {
                        sprite_tile = SPRITE_EXPLORER_S1 + (step * 4);
                        hflip = false;
                    } else if (s_player.facing == DIR_LEFT) {
                        sprite_tile = SPRITE_EXPLORER_S1 + (step * 4);
                        hflip = true;
                    }
                }

                // If invulnerable, blink every 2 frames
                if ((s_player.invuln_timer & 2) == 0) {
                    graphics_set_shadow(13, screen_x, screen_y);
                    graphics_set_sprite(0, screen_x, screen_y, sprite_tile, 0, 1, PAL_OBJ_EXPLORER, hflip, false);
                }
            }

            // 3. Draw Enemy Sprites & Shadows (Smooth 4-Frame Slither & Scuttle)
            {
                s16 cam_x = graphics_get_camera_x();
                s16 cam_y = graphics_get_camera_y();

                for (int i = 0; i < s_enemy_count; i++) {
                    Enemy* e = &s_enemies[i];
                    if (!e->active) continue;

                    s16 sx = e->pixel_x - cam_x;
                    s16 sy = e->pixel_y - cam_y;

                    u16 tile = SPRITE_SNAKE_0;
                    bool hflip = (e->dir == DIR_LEFT);
                    u8 frame = (e->anim_frame >> 2) & 3;

                    if (e->type == ENEMY_SNAKE) {
                        tile = SPRITE_SNAKE_0 + (frame * 4);
                    } else if (e->type == ENEMY_SPIDER) {
                        tile = SPRITE_SPIDER_0 + (frame * 4);
                        hflip = false;
                    }

                    graphics_set_shadow(14 + i, sx, sy);
                    graphics_set_sprite(1 + i, sx, sy, tile, 0, 1, PAL_OBJ_ENEMIES, hflip, false);
                }
            }

            // 4. Draw Rolling / Falling Boulder Animation (4-Frame Rotation)
            {
                s16 cam_x = graphics_get_camera_x();
                s16 cam_y = graphics_get_camera_y();
                u8 b_spr = 26;

                for (int y = 0; y < LEVEL_HEIGHT && b_spr < 38; y++) {
                    for (int x = 0; x < LEVEL_WIDTH && b_spr < 38; x++) {
                        if (s_map[y][x] == META_BOULDER && s_falling[y][x]) {
                            s16 sx = (x * TILE_SIZE) - cam_x;
                            s16 sy = (y * TILE_SIZE) - cam_y;
                            u8 rot = (s_level_time_frames >> 1) & 3;
                            graphics_set_sprite(b_spr++, sx, sy, SPRITE_BOULDER_0 + (rot * 4), 0, 1, PAL_OBJ_BOULDER, false, false);
                        }
                    }
                }
            }

            // 5. Draw Particles (Sparkles, Dust, Shockwaves, Footstep Puffs, Goo Decals)
            {
                s16 cam_x = graphics_get_camera_x();
                s16 cam_y = graphics_get_camera_y();

                for (int i = 0; i < MAX_PARTICLES; i++) {
                    if (!s_particles[i].active) continue;

                    s16 sx = s_particles[i].x - cam_x;
                    s16 sy = s_particles[i].y - cam_y;

                    u16 tile = SPRITE_SPARKLE_0;
                    u8 pal = PAL_OBJ_EFFECTS;

                    if (s_particles[i].type == 0) {
                        u8 f = (s_particles[i].frame / 4) % 3;
                        tile = SPRITE_SPARKLE_0 + (f * 4);
                    } else if (s_particles[i].type == 1) {
                        u8 f = (s_particles[i].frame / 4) % 3;
                        tile = SPRITE_DUST_0 + (f * 4);
                    } else if (s_particles[i].type == 2) {
                        tile = SPRITE_GOO_SPLAT;
                    } else if (s_particles[i].type == 3) {
                        tile = SPRITE_IMPACT_DUST;
                    } else if (s_particles[i].type == 4) {
                        tile = SPRITE_DUST_0;
                    }

                    graphics_set_sprite(38 + i, sx, sy, tile, 0, 1, pal, false, false);
                }
            }
            break;

        case STATE_PAUSED:
            graphics_draw_hud(s_player.diamonds_collected, s_player.diamond_quota, 
                              s_player.health, s_player.has_key, s_player.score, s_current_level + 1);

            graphics_draw_box(7, 4, 16, 11, 0);
            graphics_print_text(12, 5, "PAUSED", 0);

            graphics_print_text(9, 8,  (s_pause_selection == 0) ? "> RESUME" : "  RESUME", 0);
            graphics_print_text(9, 10, (s_pause_selection == 1) ? "> RESTART" : "  RESTART", 0);
            graphics_print_text(9, 12, (s_pause_selection == 2) ? "> QUIT" : "  QUIT", 0);
            break;

        case STATE_LEVEL_CLEAR:
            graphics_draw_box(3, 3, 24, 13, 0);
            graphics_print_text(6, 4, "EXPEDITION CLEAR!", 0);

            if (s_state_timer >= 15) {
                graphics_print_text(5, 7, "DIAMONDS: ", 0);
                u16 max_dia = s_player.diamonds_collected;
                u16 cur_dia = (s_state_timer < 50) ? ((s_state_timer - 15) * max_dia / 35) : max_dia;
                graphics_print_num(15, 7, cur_dia, 2, 0);
                graphics_print_text(17, 7, "/", 0);
                graphics_print_num(18, 7, s_levels[s_current_level].total_diamonds, 2, 0);
            }

            if (s_state_timer >= 45) {
                graphics_print_text(5, 9, "SCORE: ", 0);
                u32 max_score = s_player.score;
                u32 cur_score = (s_state_timer < 75) ? ((u32)(s_state_timer - 45) * max_score / 30) : max_score;
                graphics_print_num(13, 9, cur_score, 6, 0);
            }

            if (s_state_timer >= 70) {
                graphics_print_text(5, 11, "RATING: ", 0);
                u8 stars = 1;
                if (s_player.diamonds_collected >= s_levels[s_current_level].total_diamonds) stars = 3;
                else if (s_player.diamonds_collected >= s_levels[s_current_level].diamond_quota + 2) stars = 2;

                for (u8 s = 0; s < stars; s++) {
                    if (s_state_timer >= 70 + (s * 10)) {
                        graphics_set_bg0_tile(13 + s * 2, 11, TILE_STAR_ICON, 0);
                    }
                }
            }

            if (s_state_timer >= 90) {
                if ((s_state_timer & 16) == 0) {
                    graphics_print_text(7, 14, "PRESS START / A", 0);
                }
            }
            break;

        case STATE_GAME_OVER:
            graphics_draw_box(5, 5, 20, 10, 0);
            graphics_print_text(7, 7, "EXPEDITION FAILED", 0);
            graphics_print_text(6, 10, "START: RETRY LEVEL", 0);
            graphics_print_text(7, 12, "B: QUIT TO TITLE", 0);
            break;

        case STATE_VICTORY:
            graphics_draw_box(4, 3, 22, 14, 0);
            graphics_print_text(7, 4, "TREASURE MASTER!", 0);
            graphics_print_text(5, 6, "ALL RUINS CONQUERED!", 0);

            graphics_print_text(6, 9, "FINAL SCORE: ", 0);
            graphics_print_num(18, 9, s_player.score, 6, 0);

            graphics_print_text(6, 11, "TOTAL GEMS: ", 0);
            graphics_print_num(18, 11, s_total_diamonds_collected, 4, 0);

            graphics_print_text(7, 14, "PRESS START / A", 0);
            break;
    }
}
