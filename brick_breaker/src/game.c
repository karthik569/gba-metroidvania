#include "game.h"
#include "graphics.h"
#include "input.h"
#include "audio.h"
#include "assets.h"
#include "save.h"

// -----------------------------------------------------------------
// Game Variables
// -----------------------------------------------------------------
static GameState s_state = STATE_TITLE;
static u8 s_current_stage = 1;
static u32 s_score = 0;
static u32 s_high_score = 5000;
static s8 s_lives = 3;
static u16 s_state_timer = 0;

static Paddle s_paddle;
static Ball s_balls[MAX_BALLS];
static Capsule s_capsules[MAX_CAPSULES];
static Laser s_lasers[MAX_LASERS];
static Brick s_bricks[BRICK_ROWS][BRICK_COLS];
static u16 s_destructible_remaining = 0;

// Simple PRNG for power-up drops
static u32 s_rng_seed = 0x12345678;
static u32 rng_next(void) {
    s_rng_seed = s_rng_seed * 1103515245 + 12345;
    return (s_rng_seed >> 16) & 0x7FFF;
}

// -----------------------------------------------------------------
// ScreenBlock Tile Helpers
// -----------------------------------------------------------------
static inline void set_bg_tile(u8 x, u8 y, u16 tile) {
    vu16* screenblock = (vu16*)(VRAM_BASE + (28 * 0x800));
    screenblock[y * 32 + x] = tile;
}

static void draw_text(u8 x, u8 y, const char* str) {
    while (*str && x < 30) {
        char c = *str++;
        u16 tile = TILE_EMPTY;
        if (c >= '0' && c <= '9') {
            tile = TILE_FONT_0 + (c - '0');
        } else if (c >= 'A' && c <= 'Z') {
            tile = TILE_FONT_A + (c - 'A');
        } else if (c >= 'a' && c <= 'z') {
            tile = TILE_FONT_A + (c - 'a');
        } else if (c == ':') {
            tile = TILE_FONT_COLON;
        } else if (c == '-') {
            tile = TILE_FONT_HYPHEN;
        } else if (c == ' ') {
            tile = TILE_EMPTY;
        }
        set_bg_tile(x++, y, tile);
    }
}

static void draw_number(u8 x, u8 y, u32 val, u8 digits) {
    char buf[12];
    for (int i = digits - 1; i >= 0; i--) {
        buf[i] = '0' + (val % 10);
        val /= 10;
    }
    buf[digits] = '\0';
    draw_text(x, y, buf);
}

// -----------------------------------------------------------------
// Stage Level Designs (8 rows x 11 cols)
// -----------------------------------------------------------------
// Stage 1: Initiation Grid
static const u8 s_stage1[BRICK_ROWS][BRICK_COLS] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // Red
    {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4}, // Yellow
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3}, // Green
    {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, // Blue
    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5}, // Magenta
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6}, // Cyan
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

// Stage 2: Space Invader
static const u8 s_stage2[BRICK_ROWS][BRICK_COLS] = {
    {0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0},
    {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0},
    {0, 0, 4, 4, 4, 4, 4, 4, 4, 0, 0},
    {0, 3, 3, 0, 3, 3, 3, 0, 3, 3, 0},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {6, 0, 6, 6, 6, 6, 6, 6, 6, 0, 6},
    {6, 0, 6, 0, 0, 0, 0, 0, 6, 0, 6},
    {0, 0, 0, 2, 2, 0, 2, 2, 0, 0, 0}
};

// Stage 3: Checkerboard Vault (Silver armor + high value gems)
static const u8 s_stage3[BRICK_ROWS][BRICK_COLS] = {
    {7, 0, 7, 0, 7, 0, 7, 0, 7, 0, 7},
    {0, 5, 0, 5, 0, 5, 0, 5, 0, 5, 0},
    {7, 0, 7, 0, 7, 0, 7, 0, 7, 0, 7},
    {0, 4, 0, 4, 0, 4, 0, 4, 0, 4, 0},
    {7, 0, 7, 0, 7, 0, 7, 0, 7, 0, 7},
    {0, 6, 0, 6, 0, 6, 0, 6, 0, 6, 0},
    {7, 0, 7, 0, 7, 0, 7, 0, 7, 0, 7},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

// Stage 4: Diamond Fortress
static const u8 s_stage4[BRICK_ROWS][BRICK_COLS] = {
    {0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 7, 1, 7, 0, 0, 0, 0},
    {0, 0, 0, 7, 1, 4, 1, 7, 0, 0, 0},
    {0, 0, 7, 1, 4, 6, 4, 1, 7, 0, 0},
    {0, 7, 1, 4, 6, 5, 6, 4, 1, 7, 0},
    {0, 0, 7, 1, 4, 6, 4, 1, 7, 0, 0},
    {0, 0, 0, 7, 1, 4, 1, 7, 0, 0, 0},
    {0, 0, 0, 0, 7, 1, 7, 0, 0, 0, 0}
};

// Stage 5: Golden Gauntlet (Indestructible Gold pillars + Silver gate)
static const u8 s_stage5[BRICK_ROWS][BRICK_COLS] = {
    {8, 6, 6, 8, 6, 6, 6, 8, 6, 6, 8},
    {8, 4, 4, 8, 4, 4, 4, 8, 4, 4, 8},
    {0, 0, 0, 8, 7, 7, 7, 8, 0, 0, 0},
    {1, 1, 0, 8, 5, 5, 5, 8, 0, 1, 1},
    {2, 2, 0, 8, 3, 3, 3, 8, 0, 2, 2},
    {0, 0, 0, 8, 7, 7, 7, 8, 0, 0, 0},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7}
};

// Helper to draw a single 16x8 brick at grid position (col, row)
static void render_brick_tiles(u8 col, u8 row, u8 type, u8 hp) {
    u8 tx = 1 + (col * 2);      // Tile X: 1, 3, 5 ... 21
    u8 ty = 2 + row;            // Tile Y: 2..9 (y=16..79 in pixels)

    if (type == BRK_EMPTY) {
        set_bg_tile(tx,     ty, TILE_EMPTY);
        set_bg_tile(tx + 1, ty, TILE_EMPTY);
        return;
    }

    u16 left_tile = TILE_EMPTY;
    u16 right_tile = TILE_EMPTY;

    switch (type) {
        case BRK_RED:     left_tile = BRICK_RED_L;     right_tile = BRICK_RED_R;     break;
        case BRK_BLUE:    left_tile = BRICK_BLUE_L;    right_tile = BRICK_BLUE_R;    break;
        case BRK_GREEN:   left_tile = BRICK_GREEN_L;   right_tile = BRICK_GREEN_R;   break;
        case BRK_YELLOW:  left_tile = BRICK_YELLOW_L;  right_tile = BRICK_YELLOW_R;  break;
        case BRK_MAGENTA: left_tile = BRICK_MAGENTA_L; right_tile = BRICK_MAGENTA_R; break;
        case BRK_CYAN:    left_tile = BRICK_CYAN_L;    right_tile = BRICK_CYAN_R;    break;
        case BRK_SILVER:
            if (hp == 1) {
                left_tile = BRICK_SILVER_CRACK_L;
                right_tile = BRICK_SILVER_CRACK_R;
            } else {
                left_tile = BRICK_SILVER_L;
                right_tile = BRICK_SILVER_R;
            }
            break;
        case BRK_GOLD:    left_tile = BRICK_GOLD_L;    right_tile = BRICK_GOLD_R;    break;
        default: break;
    }

    set_bg_tile(tx,     ty, left_tile);
    set_bg_tile(tx + 1, ty, right_tile);
}

// -----------------------------------------------------------------
// Static Board Layout (Walls, Playfield, Sidebar)
// -----------------------------------------------------------------
static void init_background_playfield(void) {
    for (u8 y = 0; y < 20; y++) {
        for (u8 x = 0; x < 30; x++) {
            if (x == 0 && y == 0) {
                set_bg_tile(x, y, TILE_WALL_CORNER_TL);
            } else if (x == 23 && y == 0) {
                set_bg_tile(x, y, TILE_WALL_CORNER_TR);
            } else if (y == 0 && x > 0 && x < 23) {
                set_bg_tile(x, y, TILE_WALL_TOP);
            } else if (x == 0) {
                set_bg_tile(x, y, TILE_WALL_LEFT);
            } else if (x == 23) {
                set_bg_tile(x, y, TILE_WALL_RIGHT);
            } else if (x >= 24) {
                set_bg_tile(x, y, TILE_SIDEBAR_BG);
            } else {
                set_bg_tile(x, y, TILE_EMPTY);
            }
        }
    }

    // Horizontal dividers in sidebar
    for (u8 x = 24; x < 30; x++) {
        set_bg_tile(x, 4,  TILE_SIDEBAR_DIVIDER);
        set_bg_tile(x, 9,  TILE_SIDEBAR_DIVIDER);
        set_bg_tile(x, 14, TILE_SIDEBAR_DIVIDER);
    }

    // Sidebar text headers
    draw_text(24, 1, "HIGH");
    draw_text(24, 5, "SCORE");
    draw_text(24, 10, "STAGE");
    draw_text(24, 15, "LIVES");
}

static void update_hud_text(void) {
    draw_number(24, 2, s_high_score, 6);
    draw_number(24, 6, s_score, 6);
    draw_number(26, 11, s_current_stage, 2);
}

// -----------------------------------------------------------------
// Game Initialization & Stage Loading
// -----------------------------------------------------------------
void game_init(void) {
    save_init();
    s_high_score = save_get_high_score();
    init_background_playfield();
    update_hud_text();
    s_state = STATE_TITLE;
    s_state_timer = 0;
}

#define BALL_BASE_SPEED     480  // 1.875 px/f in 8.8 fixed point (smooth & consistent)
#define BALL_SLOW_SPEED     384  // 1.500 px/f (when [S] slow power-up active)

// Set ball reflection vector based on contact offset from paddle center, guaranteeing constant speed
static void apply_ball_deflection(Ball* ball, s16 hit_offset, s16 half_w) {
    if (half_w <= 0) half_w = 16;
    s16 ratio = (hit_offset * 100) / half_w; // -100 to +100
    fixed_t spd = ball->speed;
    if (spd <= 0) spd = BALL_BASE_SPEED;

    fixed_t vx = 0;
    fixed_t vy = -spd;

    if (ratio < -75) {
        // Sharp Left (~60 degrees): 87% H, 50% V
        vx = - (spd * 87) / 100;
        vy = - (spd * 50) / 100;
    } else if (ratio < -35) {
        // Mid Left (~45 degrees): 71% H, 71% V
        vx = - (spd * 71) / 100;
        vy = - (spd * 71) / 100;
    } else if (ratio < -10) {
        // Shallow Left (~25 degrees): 42% H, 91% V
        vx = - (spd * 42) / 100;
        vy = - (spd * 91) / 100;
    } else if (ratio <= 10) {
        // Center (~10 degrees with slight directional bias so ball never loops vertically)
        s16 dir = (s_paddle.vx > 0) ? 1 : ((s_paddle.vx < 0) ? -1 : ((ball->vx >= 0) ? 1 : -1));
        vx = dir * (spd * 17) / 100;
        vy = - (spd * 98) / 100;
    } else if (ratio <= 35) {
        // Shallow Right (~25 degrees)
        vx = (spd * 42) / 100;
        vy = - (spd * 91) / 100;
    } else if (ratio <= 75) {
        // Mid Right (~45 degrees)
        vx = (spd * 71) / 100;
        vy = - (spd * 71) / 100;
    } else {
        // Sharp Right (~60 degrees)
        vx = (spd * 87) / 100;
        vy = - (spd * 50) / 100;
    }

    ball->vx = vx;
    ball->vy = vy;
}

static void reset_paddle_and_ball(void) {
    s_paddle.width = 32;
    s_paddle.x = INT_TO_FP(PLAYFIELD_X_MIN + (PLAYFIELD_X_MAX - PLAYFIELD_X_MIN - 32) / 2);
    s_paddle.y = INT_TO_FP(144);
    s_paddle.vx = 0;
    s_paddle.active_pwr = PWR_NONE;
    s_paddle.pwr_timer = 0;
    s_paddle.laser_equipped = false;
    s_paddle.catch_equipped = false;
    s_paddle.laser_cooldown = 0;

    // Primary ball stuck to paddle
    s_balls[0].active = true;
    s_balls[0].stuck_to_paddle = true;
    s_balls[0].stuck_offset_x = 12; // Centered on 32px paddle
    s_balls[0].x = s_paddle.x + INT_TO_FP(12);
    s_balls[0].y = s_paddle.y - INT_TO_FP(8);
    s_balls[0].speed = BALL_BASE_SPEED;
    s_balls[0].speed_tier = 0;
    s_balls[0].vx = (BALL_BASE_SPEED * 42) / 100;
    s_balls[0].vy = - (BALL_BASE_SPEED * 91) / 100;

    s_balls[1].active = false;
    s_balls[2].active = false;

    for (int i = 0; i < MAX_CAPSULES; i++) s_capsules[i].active = false;
    for (int i = 0; i < MAX_LASERS; i++) s_lasers[i].active = false;
}

void game_load_stage(u8 stage_num) {
    if (stage_num < 1) stage_num = 1;
    if (stage_num > MAX_STAGES) stage_num = MAX_STAGES;
    s_current_stage = stage_num;

    const u8 (*layout)[BRICK_COLS] = s_stage1;
    switch (stage_num) {
        case 1: layout = s_stage1; break;
        case 2: layout = s_stage2; break;
        case 3: layout = s_stage3; break;
        case 4: layout = s_stage4; break;
        case 5: layout = s_stage5; break;
    }

    s_destructible_remaining = 0;
    for (u8 r = 0; r < BRICK_ROWS; r++) {
        for (u8 c = 0; c < BRICK_COLS; c++) {
            u8 type = layout[r][c];
            s_bricks[r][c].type = type;
            s_bricks[r][c].hp = (type == BRK_SILVER) ? 2 : ((type == BRK_GOLD) ? 255 : (type > 0 ? 1 : 0));
            if (type > 0 && type != BRK_GOLD) {
                s_destructible_remaining++;
            }
            render_brick_tiles(c, r, type, s_bricks[r][c].hp);
        }
    }

    reset_paddle_and_ball();
    update_hud_text();
}

void game_start_new(void) {
    s_score = 0;
    s_lives = 3;
    game_load_stage(1);
    s_state = STATE_PLAYING;
}

// -----------------------------------------------------------------
// Power-Up Drops & Triggers
// -----------------------------------------------------------------
static void spawn_capsule(s16 x, s16 y) {
    for (int i = 0; i < MAX_CAPSULES; i++) {
        if (!s_capsules[i].active) {
            s_capsules[i].active = true;
            s_capsules[i].x = INT_TO_FP(x);
            s_capsules[i].y = INT_TO_FP(y);
            s_capsules[i].vy = INT_TO_FP(1) + (INT_TO_FP(1) / 4); // 1.25 px/f
            
            u32 roll = rng_next() % 100;
            if (roll < 25) {
                s_capsules[i].type = PWR_WIDE;
            } else if (roll < 45) {
                s_capsules[i].type = PWR_LASER;
            } else if (roll < 65) {
                s_capsules[i].type = PWR_MULTI;
            } else if (roll < 80) {
                s_capsules[i].type = PWR_CATCH;
            } else if (roll < 90) {
                s_capsules[i].type = PWR_SLOW;
            } else {
                s_capsules[i].type = PWR_LIFE;
            }
            sfx_powerup_drop();
            break;
        }
    }
}

static void apply_powerup(PowerUpType type) {
    sfx_powerup_get();
    s_paddle.active_pwr = type;
    s_paddle.pwr_timer = 1800; // ~30 seconds duration

    switch (type) {
        case PWR_WIDE:
            s_paddle.width = 48;
            s_paddle.laser_equipped = false;
            s_paddle.catch_equipped = false;
            break;
        case PWR_LASER:
            s_paddle.width = 32;
            s_paddle.laser_equipped = true;
            s_paddle.catch_equipped = false;
            break;
        case PWR_MULTI:
            // Spawn 2 extra balls from active ball with exact consistent speed
            for (int i = 0; i < MAX_BALLS; i++) {
                if (s_balls[i].active) {
                    fixed_t cur_spd = s_balls[i].speed;
                    if (cur_spd <= 0) cur_spd = BALL_BASE_SPEED;
                    for (int j = 0; j < MAX_BALLS; j++) {
                        if (!s_balls[j].active) {
                            s_balls[j].active = true;
                            s_balls[j].stuck_to_paddle = false;
                            s_balls[j].x = s_balls[i].x;
                            s_balls[j].y = s_balls[i].y;
                            s_balls[j].speed = cur_spd;
                            s_balls[j].speed_tier = s_balls[i].speed_tier;
                            s_balls[j].vx = (j == 1) ? - (cur_spd * 71) / 100 : (cur_spd * 71) / 100;
                            s_balls[j].vy = - (cur_spd * 71) / 100;
                        }
                    }
                    break;
                }
            }
            break;
        case PWR_CATCH:
            s_paddle.catch_equipped = true;
            break;
        case PWR_SLOW:
            for (int i = 0; i < MAX_BALLS; i++) {
                if (s_balls[i].active) {
                    s_balls[i].speed = BALL_SLOW_SPEED;
                    s_balls[i].speed_tier = 0;
                    if (s_balls[i].vx < 0) s_balls[i].vx = - (BALL_SLOW_SPEED * 71) / 100;
                    else s_balls[i].vx = (BALL_SLOW_SPEED * 71) / 100;
                    if (s_balls[i].vy < 0) s_balls[i].vy = - (BALL_SLOW_SPEED * 71) / 100;
                    else s_balls[i].vy = (BALL_SLOW_SPEED * 71) / 100;
                }
            }
            break;
        case PWR_LIFE:
            s_lives++;
            s_score += 1000;
            break;
        default: break;
    }
}

static void fire_lasers(void) {
    s16 px = FP_TO_INT(s_paddle.x);
    s16 py = FP_TO_INT(s_paddle.y);

    // Left cannon
    for (int i = 0; i < MAX_LASERS; i++) {
        if (!s_lasers[i].active) {
            s_lasers[i].active = true;
            s_lasers[i].x = INT_TO_FP(px + 2);
            s_lasers[i].y = INT_TO_FP(py - 6);
            s_lasers[i].vy = -INT_TO_FP(5);
            break;
        }
    }
    // Right cannon
    for (int i = 0; i < MAX_LASERS; i++) {
        if (!s_lasers[i].active) {
            s_lasers[i].active = true;
            s_lasers[i].x = INT_TO_FP(px + s_paddle.width - 10);
            s_lasers[i].y = INT_TO_FP(py - 6);
            s_lasers[i].vy = -INT_TO_FP(5);
            break;
        }
    }
    sfx_laser();
}

// -----------------------------------------------------------------
// Collision & Physics Update
// -----------------------------------------------------------------
static void update_playing(void) {
    // 1. Paddle Movement
    fixed_t move_speed = INT_TO_FP(3);
    s_paddle.vx = 0;
    if (key_is_down(KEY_LEFT)) {
        s_paddle.vx = -move_speed;
    } else if (key_is_down(KEY_RIGHT)) {
        s_paddle.vx = move_speed;
    }
    s_paddle.x += s_paddle.vx;

    // Paddle boundaries
    if (s_paddle.x < INT_TO_FP(PLAYFIELD_X_MIN)) {
        s_paddle.x = INT_TO_FP(PLAYFIELD_X_MIN);
    }
    if (s_paddle.x + INT_TO_FP(s_paddle.width) > INT_TO_FP(PLAYFIELD_X_MAX)) {
        s_paddle.x = INT_TO_FP(PLAYFIELD_X_MAX - s_paddle.width);
    }

    s16 px = FP_TO_INT(s_paddle.x);
    s16 py = FP_TO_INT(s_paddle.y);

    // Laser fire
    if (s_paddle.laser_cooldown > 0) s_paddle.laser_cooldown--;
    if (s_paddle.laser_equipped && (key_was_pressed(KEY_A) || key_was_pressed(KEY_B))) {
        if (s_paddle.laser_cooldown == 0) {
            fire_lasers();
            s_paddle.laser_cooldown = 14;
        }
    }

    // Launch stuck balls on A or B
    bool launch_pressed = key_was_pressed(KEY_A) || key_was_pressed(KEY_B);

    // 2. Ball Updates
    u8 active_balls_count = 0;
    for (int b = 0; b < MAX_BALLS; b++) {
        if (!s_balls[b].active) continue;
        active_balls_count++;

        if (s_balls[b].stuck_to_paddle) {
            s_balls[b].x = s_paddle.x + INT_TO_FP(s_balls[b].stuck_offset_x);
            s_balls[b].y = s_paddle.y - INT_TO_FP(8);

            if (launch_pressed) {
                s_balls[b].stuck_to_paddle = false;
                s16 half_w = s_paddle.width / 2;
                s16 diff = s_balls[b].stuck_offset_x - half_w;
                apply_ball_deflection(&s_balls[b], diff, half_w);
                sfx_bounce();
            }
            continue;
        }

        fixed_t prev_x = s_balls[b].x;
        fixed_t prev_y = s_balls[b].y;

        s_balls[b].x += s_balls[b].vx;
        s_balls[b].y += s_balls[b].vy;

        s16 bx = FP_TO_INT(s_balls[b].x);
        s16 by = FP_TO_INT(s_balls[b].y);

        // Wall collisions (with velocity direction validation & push-out)
        if (bx <= PLAYFIELD_X_MIN && s_balls[b].vx < 0) {
            s_balls[b].x = INT_TO_FP(PLAYFIELD_X_MIN);
            s_balls[b].vx = -s_balls[b].vx;
            sfx_bounce();
        } else if (bx + 8 >= PLAYFIELD_X_MAX && s_balls[b].vx > 0) {
            s_balls[b].x = INT_TO_FP(PLAYFIELD_X_MAX - 8);
            s_balls[b].vx = -s_balls[b].vx;
            sfx_bounce();
        }

        if (by <= PLAYFIELD_Y_MIN && s_balls[b].vy < 0) {
            s_balls[b].y = INT_TO_FP(PLAYFIELD_Y_MIN);
            s_balls[b].vy = -s_balls[b].vy;
            sfx_bounce();
        } else if (by >= PLAYFIELD_Y_MAX) {
            // Ball fell down pit
            s_balls[b].active = false;
            active_balls_count--;
            continue;
        }

        // Paddle collision (with velocity direction check and constant speed angle deflection)
        bx = FP_TO_INT(s_balls[b].x);
        by = FP_TO_INT(s_balls[b].y);

        if (s_balls[b].vy > 0 && by + 8 >= py && by <= py + 6 && bx + 7 >= px && bx <= px + s_paddle.width) {
            if (s_paddle.catch_equipped) {
                s_balls[b].stuck_to_paddle = true;
                s_balls[b].stuck_offset_x = bx - px;
                sfx_bounce();
            } else {
                s_balls[b].y = INT_TO_FP(py - 8);
                s16 half_w = s_paddle.width / 2;
                s16 hit_offset = (bx + 4) - (px + half_w);
                apply_ball_deflection(&s_balls[b], hit_offset, half_w);
                sfx_bounce();
            }
        }

        // Brick collision (with penetration push-out & direction check)
        s16 cx = bx + 4;
        s16 cy = by + 4;
        if (cy >= BRICK_START_Y && cy < BRICK_START_Y + (BRICK_ROWS * BRICK_HEIGHT) &&
            cx >= PLAYFIELD_X_MIN && cx < PLAYFIELD_X_MAX) {

            s8 col = (cx - PLAYFIELD_X_MIN) / BRICK_WIDTH;
            s8 row = (cy - BRICK_START_Y) / BRICK_HEIGHT;

            if (col >= 0 && col < BRICK_COLS && row >= 0 && row < BRICK_ROWS) {
                if (s_bricks[row][col].type != BRK_EMPTY) {
                    u8 type = s_bricks[row][col].type;

                    s16 b_left   = PLAYFIELD_X_MIN + col * BRICK_WIDTH;
                    s16 b_right  = b_left + BRICK_WIDTH;
                    s16 b_top    = BRICK_START_Y + row * BRICK_HEIGHT;
                    s16 b_bottom = b_top + BRICK_HEIGHT;

                    s16 pcx = FP_TO_INT(prev_x) + 4;
                    s16 pcy = FP_TO_INT(prev_y) + 4;

                    if (pcy < b_top && s_balls[b].vy > 0) {
                        s_balls[b].y = INT_TO_FP(b_top - 8);
                        s_balls[b].vy = -s_balls[b].vy;
                    } else if (pcy >= b_bottom && s_balls[b].vy < 0) {
                        s_balls[b].y = INT_TO_FP(b_bottom);
                        s_balls[b].vy = -s_balls[b].vy;
                    } else if (pcx < b_left && s_balls[b].vx > 0) {
                        s_balls[b].x = INT_TO_FP(b_left - 8);
                        s_balls[b].vx = -s_balls[b].vx;
                    } else if (pcx >= b_right && s_balls[b].vx < 0) {
                        s_balls[b].x = INT_TO_FP(b_right);
                        s_balls[b].vx = -s_balls[b].vx;
                    } else {
                        s_balls[b].vy = -s_balls[b].vy;
                    }

                    if (type == BRK_GOLD) {
                        sfx_brick_hard();
                    } else if (type == BRK_SILVER) {
                        s_bricks[row][col].hp--;
                        if (s_bricks[row][col].hp == 1) {
                            render_brick_tiles(col, row, BRK_SILVER, 1);
                            sfx_brick_hard();
                            s_score += 50;
                        } else {
                            s_bricks[row][col].type = BRK_EMPTY;
                            render_brick_tiles(col, row, BRK_EMPTY, 0);
                            s_destructible_remaining--;
                            s_score += 500;
                            sfx_brick_hit();
                            if (rng_next() % 100 < 30) spawn_capsule(PLAYFIELD_X_MIN + col * 16, BRICK_START_Y + row * 8);
                        }
                    } else {
                        // Regular colored brick
                        static const u16 s_pts[] = {0, 100, 120, 150, 180, 200, 250};
                        s_score += s_pts[type];
                        s_bricks[row][col].type = BRK_EMPTY;
                        render_brick_tiles(col, row, BRK_EMPTY, 0);
                        s_destructible_remaining--;
                        sfx_brick_hit();
                        if (rng_next() % 100 < 22) spawn_capsule(PLAYFIELD_X_MIN + col * 16, BRICK_START_Y + row * 8);
                    }

                    if (s_score > s_high_score) {
                        s_high_score = s_score;
                        save_set_high_score(s_high_score);
                    }
                    update_hud_text();
                }
            }
        }
    }

    // Check life lost
    if (active_balls_count == 0) {
        s_lives--;
        sfx_life_lost();
        if (s_lives > 0) {
            reset_paddle_and_ball();
        } else {
            s_state = STATE_GAME_OVER;
            s_state_timer = 0;
            return;
        }
    }

    // 3. Laser Bolt Updates
    for (int i = 0; i < MAX_LASERS; i++) {
        if (!s_lasers[i].active) continue;
        s_lasers[i].y += s_lasers[i].vy;
        s16 lx = FP_TO_INT(s_lasers[i].x);
        s16 ly = FP_TO_INT(s_lasers[i].y);

        if (ly <= PLAYFIELD_Y_MIN) {
            s_lasers[i].active = false;
            continue;
        }

        // Laser vs Bricks
        if (ly >= BRICK_START_Y && ly < BRICK_START_Y + (BRICK_ROWS * BRICK_HEIGHT) &&
            lx >= PLAYFIELD_X_MIN && lx < PLAYFIELD_X_MAX) {
            s8 col = (lx - PLAYFIELD_X_MIN) / BRICK_WIDTH;
            s8 row = (ly - BRICK_START_Y) / BRICK_HEIGHT;

            if (col >= 0 && col < BRICK_COLS && row >= 0 && row < BRICK_ROWS) {
                if (s_bricks[row][col].type != BRK_EMPTY) {
                    s_lasers[i].active = false;
                    u8 type = s_bricks[row][col].type;
                    if (type != BRK_GOLD) {
                        s_bricks[row][col].type = BRK_EMPTY;
                        render_brick_tiles(col, row, BRK_EMPTY, 0);
                        s_destructible_remaining--;
                        s_score += 100;
                        sfx_brick_hit();
                    } else {
                        sfx_brick_hard();
                    }
                    update_hud_text();
                }
            }
        }
    }

    // 4. Power-up Capsule Updates
    for (int i = 0; i < MAX_CAPSULES; i++) {
        if (!s_capsules[i].active) continue;
        s_capsules[i].y += s_capsules[i].vy;
        s16 cy = FP_TO_INT(s_capsules[i].y);
        s16 cx = FP_TO_INT(s_capsules[i].x);

        if (cy >= PLAYFIELD_Y_MAX) {
            s_capsules[i].active = false;
            continue;
        }

        // Caught by paddle?
        if (cy + 8 >= py && cy <= py + 8 && cx + 15 >= px && cx <= px + s_paddle.width) {
            s_capsules[i].active = false;
            apply_powerup(s_capsules[i].type);
        }
    }

    // Check Stage Clear
    if (s_destructible_remaining == 0) {
        sfx_stage_clear();
        s_score += 1000 * s_current_stage;
        if (s_score > s_high_score) {
            s_high_score = s_score;
            save_set_high_score(s_high_score);
        }
        update_hud_text();

        if (s_current_stage >= MAX_STAGES) {
            s_state = STATE_VICTORY;
        } else {
            s_state = STATE_LEVEL_CLEAR;
        }
        s_state_timer = 0;
    }
}

// -----------------------------------------------------------------
// Master State Machine Update
// -----------------------------------------------------------------
void game_update(void) {
    s_state_timer++;

    switch (s_state) {
        case STATE_TITLE:
            if (key_was_pressed(KEY_START) || key_was_pressed(KEY_A)) {
                game_start_new();
            }
            break;

        case STATE_PLAYING:
            if (key_was_pressed(KEY_START)) {
                s_state = STATE_PAUSED;
                draw_text(6, 11, "PAUSED");
            } else {
                update_playing();
            }
            break;

        case STATE_PAUSED:
            if (key_was_pressed(KEY_START)) {
                s_state = STATE_PLAYING;
                draw_text(6, 11, "      ");
            }
            break;

        case STATE_LEVEL_CLEAR:
            if (s_state_timer == 1) {
                draw_text(4, 11, "STAGE CLEAR!");
            }
            if (s_state_timer > 120 || key_was_pressed(KEY_A) || key_was_pressed(KEY_START)) {
                draw_text(4, 11, "            ");
                game_load_stage(s_current_stage + 1);
                s_state = STATE_PLAYING;
            }
            break;

        case STATE_GAME_OVER:
            if (s_state_timer == 1) {
                draw_text(5, 11, "GAME OVER");
            }
            if (s_state_timer > 150 || key_was_pressed(KEY_START) || key_was_pressed(KEY_A)) {
                draw_text(5, 11, "         ");
                init_background_playfield();
                s_state = STATE_TITLE;
            }
            break;

        case STATE_VICTORY:
            if (s_state_timer == 1) {
                draw_text(3, 11, "VICTORY! ALL CLEAR");
            }
            if (s_state_timer > 200 || key_was_pressed(KEY_START) || key_was_pressed(KEY_A)) {
                draw_text(3, 11, "                  ");
                init_background_playfield();
                s_state = STATE_TITLE;
            }
            break;
    }
}

// -----------------------------------------------------------------
// Sprite & HUD Rendering
// -----------------------------------------------------------------
void game_render(void) {
    oam_clear();
    u8 sid = 0;

    if (s_state == STATE_TITLE) {
        // Draw title message on center playfield
        draw_text(4, 5, "BRICK BREAKER");
        draw_text(7, 7, "GBA EDITION");
        if ((s_state_timer / 30) % 2 == 0) {
            draw_text(4, 11, "PRESS START");
        } else {
            draw_text(4, 11, "           ");
        }
        oam_commit();
        return;
    }

    s16 px = FP_TO_INT(s_paddle.x);
    s16 py = FP_TO_INT(s_paddle.y);

    // 1. Render Paddle
    if (s_paddle.width == 48) {
        // Wide paddle (3 sprites: left, mid, right)
        u16 left_tile = s_paddle.laser_equipped ? SPRITE_TILE_PADDLE_LASER_L : SPRITE_TILE_PADDLE_LEFT;
        u16 right_tile = s_paddle.laser_equipped ? SPRITE_TILE_PADDLE_LASER_R : SPRITE_TILE_PADDLE_RIGHT;
        oam_set(sid++, px,      py, ATTR0_WIDE, ATTR1_SIZE_8, left_tile,              0, false, false);
        oam_set(sid++, px + 16, py, ATTR0_WIDE, ATTR1_SIZE_8, SPRITE_TILE_PADDLE_MID,  0, false, false);
        oam_set(sid++, px + 32, py, ATTR0_WIDE, ATTR1_SIZE_8, right_tile,             0, false, false);
    } else {
        // Normal 32px paddle (2 sprites: left, right)
        u16 left_tile = s_paddle.laser_equipped ? SPRITE_TILE_PADDLE_LASER_L : SPRITE_TILE_PADDLE_LEFT;
        u16 right_tile = s_paddle.laser_equipped ? SPRITE_TILE_PADDLE_LASER_R : SPRITE_TILE_PADDLE_RIGHT;
        oam_set(sid++, px,      py, ATTR0_WIDE, ATTR1_SIZE_8, left_tile,  0, false, false);
        oam_set(sid++, px + 16, py, ATTR0_WIDE, ATTR1_SIZE_8, right_tile, 0, false, false);
    }

    // 2. Render Balls (8x8)
    for (int i = 0; i < MAX_BALLS; i++) {
        if (!s_balls[i].active) continue;
        s16 bx = FP_TO_INT(s_balls[i].x);
        s16 by = FP_TO_INT(s_balls[i].y);
        oam_set(sid++, bx, by, ATTR0_SQUARE, ATTR1_SIZE_8, SPRITE_TILE_BALL, 0, false, false);
    }

    // 3. Render Lasers (8x8)
    for (int i = 0; i < MAX_LASERS; i++) {
        if (!s_lasers[i].active) continue;
        s16 lx = FP_TO_INT(s_lasers[i].x);
        s16 ly = FP_TO_INT(s_lasers[i].y);
        oam_set(sid++, lx, ly, ATTR0_SQUARE, ATTR1_SIZE_8, SPRITE_TILE_LASER, 0, false, false);
    }

    // 4. Render Power-Up Capsules (16x8)
    for (int i = 0; i < MAX_CAPSULES; i++) {
        if (!s_capsules[i].active) continue;
        s16 cx = FP_TO_INT(s_capsules[i].x);
        s16 cy = FP_TO_INT(s_capsules[i].y);
        u16 tile = SPRITE_TILE_PWR_WIDE;
        switch (s_capsules[i].type) {
            case PWR_WIDE:  tile = SPRITE_TILE_PWR_WIDE;  break;
            case PWR_LASER: tile = SPRITE_TILE_PWR_LASER; break;
            case PWR_MULTI: tile = SPRITE_TILE_PWR_MULTI; break;
            case PWR_CATCH: tile = SPRITE_TILE_PWR_CATCH; break;
            case PWR_SLOW:  tile = SPRITE_TILE_PWR_SLOW;  break;
            case PWR_LIFE:  tile = SPRITE_TILE_PWR_LIFE;  break;
            default: break;
        }
        oam_set(sid++, cx, cy, ATTR0_WIDE, ATTR1_SIZE_8, tile, 0, false, false);
    }

    // 5. Sidebar Lives Icons
    for (s8 i = 0; i < s_lives && i < 5; i++) {
        oam_set(sid++, 196 + (i * 8), 132, ATTR0_SQUARE, ATTR1_SIZE_8, SPRITE_TILE_MINI_PADDLE, 0, false, false);
    }

    // 6. Sidebar Active Powerup Badge (16x16)
    if (s_paddle.active_pwr != PWR_NONE) {
        u16 btile = SPRITE_TILE_BADGE_NONE;
        switch (s_paddle.active_pwr) {
            case PWR_WIDE:  btile = SPRITE_TILE_BADGE_WIDE;  break;
            case PWR_LASER: btile = SPRITE_TILE_BADGE_LASER; break;
            case PWR_MULTI: btile = SPRITE_TILE_BADGE_MULTI; break;
            case PWR_CATCH: btile = SPRITE_TILE_BADGE_CATCH; break;
            default: btile = SPRITE_TILE_BADGE_NONE; break;
        }
        oam_set(sid++, 204, 68, ATTR0_SQUARE, ATTR1_SIZE_16, btile, 0, false, false);
    }

    oam_commit();
}

GameState game_get_state(void) {
    return s_state;
}
