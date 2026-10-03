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
static u8 s_max_stage_unlocked = 1;
static u8 s_selected_stage = 1;
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

static KineticBrick s_kinetic_bricks[MAX_KINETIC_BRICKS];
static RegenTracker s_regen_trackers[MAX_REGEN_TRACKERS];
static bool s_tnt_capsule_dropped = false;
static u8 s_pwr_toast_timer = 0;
static PowerUpType s_pwr_toast_type = PWR_NONE;
static u8 s_shield_hits = 0;
static u16 s_mega_timer = 0;
static Boss s_boss;

static Hazard s_hazards[MAX_HAZARDS];
static Particle s_particles[MAX_PARTICLES];
static u16 s_hazard_spawn_timer = 0;
static u8 s_shake_timer = 0;
static u8 s_shake_mag = 0;

// 32-entry sine table for smooth horizontal hazard drone oscillation (amplitude ~16 px)
static const s8 s_sin32[32] = {
     0,  3,  6,  9, 11, 13, 15, 16,
    16, 15, 13, 11,  9,  6,  3,  0,
     0, -3, -6, -9,-11,-13,-15,-16,
   -16,-15,-13,-11, -9, -6, -3,  0
};

static inline void trigger_screen_shake(u8 mag, u8 duration) {
    (void)mag;
    (void)duration;
}


static void spawn_particles(s16 x, s16 y, u8 pal) {
    static const s16 vxs[4] = {-192, 192, -128, 128};
    static const s16 vys[4] = {-256, -256, -160, -192};
    u8 spawned = 0;
    for (int i = 0; i < MAX_PARTICLES && spawned < 4; i++) {
        if (!s_particles[i].active) {
            s_particles[i].active = true;
            s_particles[i].x = INT_TO_FP(x + 4 + (spawned * 2));
            s_particles[i].y = INT_TO_FP(y + 2);
            s_particles[i].vx = vxs[spawned];
            s_particles[i].vy = vys[spawned];
            s_particles[i].life = 18;
            s_particles[i].pal = pal;
            spawned++;
        }
    }
}

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
        } else if (c == '<') {
            tile = TILE_FONT_LESS;
        } else if (c == '>') {
            tile = TILE_FONT_GREATER;
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

// Stage 6: Demolition Depot (Introduction of TNT Explosive Bricks)
static const u8 s_stage6[BRICK_ROWS][BRICK_COLS] = {
    {1, 1, 9, 1, 1, 6, 1, 1, 9, 1, 1},
    {4, 4, 1, 4, 9, 6, 9, 4, 1, 4, 4},
    {3, 3, 3, 3, 4, 6, 4, 3, 3, 3, 3},
    {2, 9, 2, 2, 3, 6, 3, 2, 2, 9, 2},
    {5, 2, 5, 5, 2, 6, 2, 5, 5, 2, 5},
    {5, 5, 9, 5, 5, 9, 5, 5, 9, 5, 5},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

// Stage 7: Matrix Chamber (Introduction of 3-Hit Regenerating Bricks)
static const u8 s_stage7[BRICK_ROWS][BRICK_COLS] = {
    {7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7},
    {10, 0, 10, 0, 10, 0, 10, 0, 10, 0, 10},
    {10, 5, 10, 6, 10, 5, 10, 6, 10, 5, 10},
    {10, 5, 10, 6, 10, 5, 10, 6, 10, 5, 10},
    {10, 0, 10, 0, 10, 0, 10, 0, 10, 0, 10},
    {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

// Stage 8: Orbital Perimeter (Introduction of Kinetic Moving Patrol Bricks)
static const u8 s_stage8[BRICK_ROWS][BRICK_COLS] = {
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {7, 1, 7, 1, 7, 1, 7, 1, 7, 1, 7},
    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
    {2, 3, 4, 2, 3, 4, 2, 3, 4, 2, 3},
    {7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

// Stage 9: Hazard Core (Gold Pillars, TNT Traps, and Regenerating Rows)
static const u8 s_stage9[BRICK_ROWS][BRICK_COLS] = {
    {8, 9, 8, 5, 5, 8, 5, 5, 8, 9, 8},
    {8, 1, 8, 6, 6, 8, 6, 6, 8, 1, 8},
    {8, 9, 8, 10, 10, 8, 10, 10, 8, 9, 8},
    {0, 0, 0, 7, 7, 7, 7, 7, 0, 0, 0},
    {10, 10, 0, 9, 4, 4, 4, 9, 0, 10, 10},
    {8, 0, 0, 3, 3, 3, 3, 3, 0, 0, 8},
    {8, 2, 2, 2, 2, 2, 2, 2, 2, 2, 8},
    {7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7}
};

// Stage 10: The Apex Fortress (Grand Climax with Kinetic Shields, Regen Cores, Gold Bastions & TNT)
static const u8 s_stage10[BRICK_ROWS][BRICK_COLS] = {
    {8, 7, 8, 6, 6, 9, 6, 6, 8, 7, 8},
    {8, 9, 8, 10, 10, 10, 10, 10, 8, 9, 8},
    {7, 5, 7, 10, 5, 6, 5, 10, 7, 5, 7},
    {8, 1, 8, 10, 6, 5, 6, 10, 8, 1, 8},
    {8, 9, 8, 10, 10, 10, 10, 10, 8, 9, 8},
    {0, 0, 0, 7, 7, 9, 7, 7, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

// Stage 11: Cyber Protocol (Alternating Bit-Streams & Armor Capacitors)
static const u8 s_stage11[BRICK_ROWS][BRICK_COLS] = {
    {6, 2, 6, 2, 6, 2, 6, 2, 6, 2, 6},
    {2, 7, 2, 7, 2, 7, 2, 7, 2, 7, 2},
    {6, 2, 6, 2, 6, 2, 6, 2, 6, 2, 6},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {4, 4, 7, 4, 4, 7, 4, 4, 7, 4, 4},
    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

// Stage 12: Binary Maze (Gold Circuit Channels & TNT Embedded Nodes)
static const u8 s_stage12[BRICK_ROWS][BRICK_COLS] = {
    {8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
    {8, 9, 6, 8, 6, 9, 6, 8, 6, 9, 8},
    {8, 6, 6, 8, 6, 6, 6, 8, 6, 6, 8},
    {8, 4, 4, 8, 4, 9, 4, 8, 4, 4, 8},
    {8, 3, 3, 8, 3, 3, 3, 8, 3, 3, 8},
    {8, 9, 2, 8, 2, 9, 2, 8, 2, 9, 8},
    {8, 2, 2, 8, 2, 2, 2, 8, 2, 2, 8},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

// Stage 13: Neon Firewall (Double Silver Barrier & Regenerating Core Shield)
static const u8 s_stage13[BRICK_ROWS][BRICK_COLS] = {
    {7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7},
    {7, 10, 10, 10, 7, 7, 7, 10, 10, 10, 7},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {5, 10, 5, 10, 5, 6, 5, 10, 5, 10, 5},
    {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
    {3, 10, 3, 10, 3, 4, 3, 10, 3, 10, 3},
    {7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

// Stage 14: Data Stream (Corridor Pattern with Kinetic Moving Patrol Bricks)
static const u8 s_stage14[BRICK_ROWS][BRICK_COLS] = {
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {1, 2, 3, 4, 5, 6, 5, 4, 3, 2, 1},
    {7, 7, 0, 7, 7, 7, 7, 7, 0, 7, 7},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {7, 7, 0, 7, 7, 7, 7, 7, 0, 7, 7},
    {5, 4, 3, 2, 1, 6, 1, 2, 3, 4, 5},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

// Stage 15: Circuit Cascade (Dense Interlocking TNT Nodes with Gold Baffles)
static const u8 s_stage15[BRICK_ROWS][BRICK_COLS] = {
    {8, 9, 8, 9, 8, 9, 8, 9, 8, 9, 8},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {8, 9, 8, 7, 8, 9, 8, 7, 8, 9, 8},
    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
    {8, 9, 8, 9, 8, 9, 8, 9, 8, 9, 8},
    {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
    {7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

// Stage 16: Quantum Core (Triple Regenerating Clusters Guarded by Silver Rings)
static const u8 s_stage16[BRICK_ROWS][BRICK_COLS] = {
    {7, 7, 7, 8, 7, 7, 7, 8, 7, 7, 7},
    {7, 10, 7, 8, 7, 10, 7, 8, 7, 10, 7},
    {7, 7, 7, 8, 7, 7, 7, 8, 7, 7, 7},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
    {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

// Stage 17: Logic Gate (Gold Gates channeling Precision Ricochets)
static const u8 s_stage17[BRICK_ROWS][BRICK_COLS] = {
    {8, 6, 8, 0, 8, 6, 8, 0, 8, 6, 8},
    {8, 6, 8, 0, 8, 6, 8, 0, 8, 6, 8},
    {8, 9, 8, 7, 8, 9, 8, 7, 8, 9, 8},
    {8, 5, 8, 0, 8, 5, 8, 0, 8, 5, 8},
    {8, 4, 8, 7, 8, 4, 8, 7, 8, 4, 8},
    {8, 3, 8, 0, 8, 3, 8, 0, 8, 3, 8},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7}
};

// Stage 18: Mainframe Citadel (Fortress Designed for Mega Piercing Assaults)
static const u8 s_stage18[BRICK_ROWS][BRICK_COLS] = {
    {7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7},
    {7, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7},
    {7, 6, 5, 5, 5, 5, 5, 5, 5, 6, 7},
    {7, 6, 5, 4, 4, 9, 4, 4, 5, 6, 7},
    {7, 6, 5, 4, 3, 3, 3, 4, 5, 6, 7},
    {7, 6, 5, 4, 4, 4, 4, 4, 5, 6, 7},
    {7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

// Stage 19: Glitch Infiltration (Extreme Gauntlet: TNT, Regen, Gold & Speed Hazards)
static const u8 s_stage19[BRICK_ROWS][BRICK_COLS] = {
    {8, 9, 10, 8, 7, 6, 7, 8, 10, 9, 8},
    {8, 10, 9, 8, 6, 7, 6, 8, 9, 10, 8},
    {7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7},
    {0, 8, 9, 10, 0, 9, 0, 10, 9, 8, 0},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
    {7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

// Stage 20: The Cyber Nexus (Arena for Master AI Core Boss)
static const u8 s_stage20[BRICK_ROWS][BRICK_COLS] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {7, 7, 0, 0, 0, 0, 0, 0, 0, 7, 7},
    {8, 8, 0, 0, 6, 6, 6, 0, 0, 8, 8},
    {7, 7, 0, 0, 9, 7, 9, 0, 0, 7, 7},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

// Floor Shield Barrier BG Tilemap Renderer
static void render_shield_barrier(void) {
    u16 tile = TILE_EMPTY;
    if (s_shield_hits == 2) {
        tile = TILE_BARRIER_FULL;
    } else if (s_shield_hits == 1) {
        tile = TILE_BARRIER_DMG;
    }
    for (u8 c = 1; c <= 22; c++) {
        set_bg_tile(c, 19, tile);
    }
}

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
        case BRK_TNT:     left_tile = BRICK_TNT_L;     right_tile = BRICK_TNT_R;     break;
        case BRK_REGEN:
            if (hp == 1) {
                left_tile = BRICK_REGEN_DMG2_L;
                right_tile = BRICK_REGEN_DMG2_R;
            } else if (hp == 2) {
                left_tile = BRICK_REGEN_DMG1_L;
                right_tile = BRICK_REGEN_DMG1_R;
            } else {
                left_tile = BRICK_REGEN_L;
                right_tile = BRICK_REGEN_R;
            }
            break;
        default: break;
    }

    set_bg_tile(tx,     ty, left_tile);
    set_bg_tile(tx + 1, ty, right_tile);
}

// -----------------------------------------------------------------
// Static Board Layout (Walls, Playfield, Sidebar)
// -----------------------------------------------------------------
static void init_background_playfield(void) {
    // Clear entire 32x32 ScreenBlock 28 so all margins and off-screen areas are empty
    for (u8 y = 0; y < 32; y++) {
        for (u8 x = 0; x < 32; x++) {
            set_bg_tile(x, y, TILE_EMPTY);
        }
    }

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
            }
        }
    }


    // Horizontal dividers in sidebar
    for (u8 x = 24; x < 30; x++) {
        set_bg_tile(x, 3,  TILE_SIDEBAR_DIVIDER);
        set_bg_tile(x, 6,  TILE_SIDEBAR_DIVIDER);
        set_bg_tile(x, 9,  TILE_SIDEBAR_DIVIDER);
        set_bg_tile(x, 14, TILE_SIDEBAR_DIVIDER);
    }

    // Sidebar text headers
    draw_text(24, 1,  "HIGH");
    draw_text(24, 4,  "SCORE");
    draw_text(24, 7,  "STAGE");
    draw_text(24, 10, "ITEM");
    draw_text(24, 15, "LIVES");
    render_shield_barrier();
}

static void update_hud_text(void) {
    draw_number(24, 2, s_high_score, 6);
    draw_number(24, 5, s_score, 6);
    draw_number(26, 8, s_current_stage, 2);

    PowerUpType display_pwr = s_paddle.active_pwr;
    if (s_pwr_toast_timer > 0 && s_pwr_toast_type == PWR_LIFE) {
        display_pwr = PWR_LIFE;
    } else if (s_pwr_toast_timer > 0 && s_pwr_toast_type == PWR_SHIELD) {
        display_pwr = PWR_SHIELD;
    } else if (s_pwr_toast_timer > 0 && s_pwr_toast_type == PWR_MEGA) {
        display_pwr = PWR_MEGA;
    } else if (display_pwr == PWR_NONE) {
        if (s_mega_timer > 0) display_pwr = PWR_MEGA;
        else if (s_shield_hits > 0) display_pwr = PWR_SHIELD;
    }

    switch (display_pwr) {
        case PWR_WIDE:
            draw_text(24, 11, " WIDE ");
            draw_text(24, 12, "EXPAND");
            break;
        case PWR_LASER:
            draw_text(24, 11, "LASER ");
            draw_text(24, 12, "CANNON");
            break;
        case PWR_MULTI:
            draw_text(24, 11, "MULTI ");
            draw_text(24, 12, "3-BALL");
            break;
        case PWR_CATCH:
            draw_text(24, 11, "CATCH ");
            draw_text(24, 12, "MAGNET");
            break;
        case PWR_SLOW:
            draw_text(24, 11, " SLOW ");
            draw_text(24, 12, "SPEED-");
            break;
        case PWR_LIFE:
            draw_text(24, 11, " 1-UP ");
            draw_text(24, 12, "BONUS ");
            break;
        case PWR_SHIELD:
            draw_text(24, 11, "SHIELD");
            draw_text(24, 12, (s_shield_hits == 2) ? "2 HITS" : "1 HIT ");
            break;
        case PWR_MEGA:
            draw_text(24, 11, " MEGA ");
            draw_text(24, 12, "PIERCE");
            break;
        case PWR_NONE:
        default:
            draw_text(24, 11, " NONE ");
            draw_text(24, 12, " ---- ");
            break;
    }
}

// -----------------------------------------------------------------
// Game Initialization & Stage Loading
// -----------------------------------------------------------------
void game_init(void) {
    save_init();
    s_high_score = save_get_high_score();
    s_max_stage_unlocked = save_get_max_stage();
    if (s_max_stage_unlocked < 1) s_max_stage_unlocked = 1;
    if (s_max_stage_unlocked > MAX_STAGES) s_max_stage_unlocked = MAX_STAGES;
    s_selected_stage = 1;

    init_background_playfield();
    update_hud_text();
    s_state = STATE_TITLE;
    s_state_timer = 0;
    jingle_title();
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
    s_pwr_toast_timer = 0;
    s_pwr_toast_type = PWR_NONE;
    draw_text(3, 1, "                   ");

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
    for (int i = 0; i < MAX_HAZARDS; i++) s_hazards[i].active = false;
    for (int i = 0; i < MAX_PARTICLES; i++) s_particles[i].active = false;
    s_hazard_spawn_timer = 300;
    s_shake_timer = 0;
    s_shake_mag = 0;
    REG_BG0HOFS = 0;
    REG_BG0VOFS = 0;
}


static void init_boss(u8 stage) {
    s_boss.active = true;
    s_boss.phase = BOSS_PHASE_SHIELDED;
    s_boss.wave_timer = 0;
    s_boss.death_timer = 0;
    s_boss.invuln_flash = 0;
    s_boss.base_x = INT_TO_FP(80);
    s_boss.x = s_boss.base_x;
    s_boss.y = INT_TO_FP(20);

    if (stage == 10) {
        // Sector 1 Guardian
        s_boss.max_hp = 12;
        s_boss.hp = 12;
        s_boss.shoot_timer = 120;
        s_boss.pods[0].active = true;
        s_boss.pods[0].x = INT_TO_FP(52);
        s_boss.pods[0].y = INT_TO_FP(24);
        s_boss.pods[0].hp = 4;

        s_boss.pods[1].active = true;
        s_boss.pods[1].x = INT_TO_FP(124);
        s_boss.pods[1].y = INT_TO_FP(24);
        s_boss.pods[1].hp = 4;
    } else {
        // Stage 20: Master AI Core
        s_boss.max_hp = 18;
        s_boss.hp = 18;
        s_boss.shoot_timer = 90;
        s_boss.pods[0].active = true;
        s_boss.pods[0].x = INT_TO_FP(48);
        s_boss.pods[0].y = INT_TO_FP(24);
        s_boss.pods[0].hp = 6;

        s_boss.pods[1].active = true;
        s_boss.pods[1].x = INT_TO_FP(128);
        s_boss.pods[1].y = INT_TO_FP(24);
        s_boss.pods[1].hp = 6;
    }

    for (int b = 0; b < MAX_BOSS_BOLTS; b++) s_boss.bolts[b].active = false;
    bgm_boss_start();
}

static void update_boss(void) {
    if (!s_boss.active) return;

    if (s_boss.invuln_flash > 0) s_boss.invuln_flash--;

    if (s_boss.phase == BOSS_PHASE_DYING) {
        s_boss.death_timer++;
        if (s_boss.death_timer % 6 == 0) {
            trigger_screen_shake(2, 6);
            sfx_hazard_hit();
            s16 rx = FP_TO_INT(s_boss.x) + (rng_next() % 24);
            s16 ry = FP_TO_INT(s_boss.y) + (rng_next() % 24);
            spawn_particles(rx, ry, (rng_next() % 7) + 1);
        }
        return;
    }

    // Sinusoidal hover
    s_boss.wave_timer = (s_boss.wave_timer + 1) & 31;
    s16 offset = s_sin32[s_boss.wave_timer]; // -16 to +16
    if (s_boss.phase == BOSS_PHASE_ENRAGED) offset = (offset * 3) / 2; // -24 to +24
    s_boss.x = s_boss.base_x + INT_TO_FP(offset);

    // Pods follow core
    if (s_boss.pods[0].active) {
        s_boss.pods[0].x = s_boss.x - INT_TO_FP(24);
        s_boss.pods[0].y = s_boss.y + INT_TO_FP(4);
    }
    if (s_boss.pods[1].active) {
        s_boss.pods[1].x = s_boss.x + INT_TO_FP(36);
        s_boss.pods[1].y = s_boss.y + INT_TO_FP(4);
    }

    // Phase transition if both pods destroyed
    if (s_boss.phase == BOSS_PHASE_SHIELDED && !s_boss.pods[0].active && !s_boss.pods[1].active) {
        s_boss.phase = BOSS_PHASE_EXPOSED;
        sfx_boss_pod_destroyed();
        trigger_screen_shake(2, 10);
    }

    // Bolt firing
    if (s_boss.shoot_timer > 0) {
        s_boss.shoot_timer--;
    } else {
        s_boss.shoot_timer = (s_boss.phase == BOSS_PHASE_ENRAGED) ? 60 : 100;
        for (int b = 0; b < MAX_BOSS_BOLTS; b++) {
            if (!s_boss.bolts[b].active) {
                s_boss.bolts[b].active = true;
                s_boss.bolts[b].x = s_boss.x + INT_TO_FP(12);
                s_boss.bolts[b].y = s_boss.y + INT_TO_FP(28);
                s_boss.bolts[b].vy = INT_TO_FP(1) + (INT_TO_FP(1) / 2); // 1.5 px/f
                s_boss.bolts[b].vx = (rng_next() % 2 == 0) ? (INT_TO_FP(1)/4) : (-INT_TO_FP(1)/4);
                sfx_boss_fire();
                break;
            }
        }
    }

    // Update bolts
    s16 px = FP_TO_INT(s_paddle.x);
    s16 py = FP_TO_INT(s_paddle.y);
    for (int b = 0; b < MAX_BOSS_BOLTS; b++) {
        if (!s_boss.bolts[b].active) continue;
        s_boss.bolts[b].x += s_boss.bolts[b].vx;
        s_boss.bolts[b].y += s_boss.bolts[b].vy;

        s16 bx = FP_TO_INT(s_boss.bolts[b].x);
        s16 by = FP_TO_INT(s_boss.bolts[b].y);

        if (by > 160 || bx < PLAYFIELD_X_MIN || bx > PLAYFIELD_X_MAX) {
            s_boss.bolts[b].active = false;
            continue;
        }

        // Bolt vs Paddle
        if (by + 7 >= py && by <= py + 6 && bx + 7 >= px && bx <= px + s_paddle.width) {
            s_boss.bolts[b].active = false;
            trigger_screen_shake(3, 14);
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
    }
}

void game_load_stage(u8 stage_num) {
    if (stage_num < 1) stage_num = 1;
    if (stage_num > MAX_STAGES) stage_num = MAX_STAGES;
    s_current_stage = stage_num;

    // Load Zone 1 or Zone 2 palette
    assets_load_zone_palette((stage_num >= 11) ? 2 : 1);

    // Reset powerup states for fresh stage start
    s_shield_hits = 0;
    s_mega_timer = 0;

    init_background_playfield();

    const u8 (*layout)[BRICK_COLS] = s_stage1;
    switch (stage_num) {
        case 1:  layout = s_stage1;  break;
        case 2:  layout = s_stage2;  break;
        case 3:  layout = s_stage3;  break;
        case 4:  layout = s_stage4;  break;
        case 5:  layout = s_stage5;  break;
        case 6:  layout = s_stage6;  break;
        case 7:  layout = s_stage7;  break;
        case 8:  layout = s_stage8;  break;
        case 9:  layout = s_stage9;  break;
        case 10: layout = s_stage10; break;
        case 11: layout = s_stage11; break;
        case 12: layout = s_stage12; break;
        case 13: layout = s_stage13; break;
        case 14: layout = s_stage14; break;
        case 15: layout = s_stage15; break;
        case 16: layout = s_stage16; break;
        case 17: layout = s_stage17; break;
        case 18: layout = s_stage18; break;
        case 19: layout = s_stage19; break;
        case 20: layout = s_stage20; break;
        default: layout = s_stage1;  break;
    }

    // Reset dynamic entities
    for (int k = 0; k < MAX_KINETIC_BRICKS; k++) s_kinetic_bricks[k].active = false;
    for (int t = 0; t < MAX_REGEN_TRACKERS; t++) s_regen_trackers[t].active = false;
    u8 regen_idx = 0;

    s_destructible_remaining = 0;
    for (u8 r = 0; r < BRICK_ROWS; r++) {
        for (u8 c = 0; c < BRICK_COLS; c++) {
            u8 type = layout[r][c];
            s_bricks[r][c].type = type;
            if (type == BRK_SILVER) {
                s_bricks[r][c].hp = 2;
            } else if (type == BRK_GOLD) {
                s_bricks[r][c].hp = 255;
            } else if (type == BRK_REGEN) {
                s_bricks[r][c].hp = 3;
                if (regen_idx < MAX_REGEN_TRACKERS) {
                    s_regen_trackers[regen_idx].col = c;
                    s_regen_trackers[regen_idx].row = r;
                    s_regen_trackers[regen_idx].hp = 3;
                    s_regen_trackers[regen_idx].heal_timer = 300;
                    s_regen_trackers[regen_idx].active = true;
                    regen_idx++;
                }
            } else if (type > 0) {
                s_bricks[r][c].hp = 1;
            } else {
                s_bricks[r][c].hp = 0;
            }

            if (type > 0 && type != BRK_GOLD) {
                s_destructible_remaining++;
            }
            render_brick_tiles(c, r, type, s_bricks[r][c].hp);
        }
    }

    // Spawn Kinetic Moving Bricks on designated stages
    if (stage_num == 8) {
        s_kinetic_bricks[0].active = true;
        s_kinetic_bricks[0].x = INT_TO_FP(24);
        s_kinetic_bricks[0].y = INT_TO_FP(64);
        s_kinetic_bricks[0].vx = INT_TO_FP(1);
        s_kinetic_bricks[0].hp = 2;

        s_kinetic_bricks[1].active = true;
        s_kinetic_bricks[1].x = INT_TO_FP(136);
        s_kinetic_bricks[1].y = INT_TO_FP(76);
        s_kinetic_bricks[1].vx = -INT_TO_FP(1);
        s_kinetic_bricks[1].hp = 2;
    } else if (stage_num == 14) {
        s_kinetic_bricks[0].active = true;
        s_kinetic_bricks[0].x = INT_TO_FP(24);
        s_kinetic_bricks[0].y = INT_TO_FP(40);
        s_kinetic_bricks[0].vx = INT_TO_FP(1);
        s_kinetic_bricks[0].hp = 2;

        s_kinetic_bricks[1].active = true;
        s_kinetic_bricks[1].x = INT_TO_FP(136);
        s_kinetic_bricks[1].y = INT_TO_FP(48);
        s_kinetic_bricks[1].vx = -INT_TO_FP(1);
        s_kinetic_bricks[1].hp = 2;
    } else if (stage_num == 16) {
        s_kinetic_bricks[0].active = true;
        s_kinetic_bricks[0].x = INT_TO_FP(32);
        s_kinetic_bricks[0].y = INT_TO_FP(56);
        s_kinetic_bricks[0].vx = INT_TO_FP(1) + (INT_TO_FP(1) / 4);
        s_kinetic_bricks[0].hp = 2;

        s_kinetic_bricks[1].active = true;
        s_kinetic_bricks[1].x = INT_TO_FP(128);
        s_kinetic_bricks[1].y = INT_TO_FP(64);
        s_kinetic_bricks[1].vx = - (INT_TO_FP(1) + (INT_TO_FP(1) / 4));
        s_kinetic_bricks[1].hp = 2;
    }

    // Boss initialization on Stage 10 & Stage 20
    if (stage_num == 10 || stage_num == 20) {
        init_boss(stage_num);
    } else {
        s_boss.active = false;
        bgm_boss_stop();
    }

    reset_paddle_and_ball();
    update_hud_text();
}

void game_start_new(void) {
    s_score = 0;
    s_lives = 3;
    game_load_stage(s_selected_stage);
    s_state = STATE_PLAYING;
    sfx_powerup_get();
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
            if (roll < 18) {
                s_capsules[i].type = PWR_WIDE;
            } else if (roll < 34) {
                s_capsules[i].type = PWR_LASER;
            } else if (roll < 50) {
                s_capsules[i].type = PWR_MULTI;
            } else if (roll < 64) {
                s_capsules[i].type = PWR_CATCH;
            } else if (roll < 76) {
                s_capsules[i].type = PWR_SHIELD;
            } else if (roll < 86) {
                s_capsules[i].type = PWR_SLOW;
            } else if (roll < 96) {
                s_capsules[i].type = PWR_MEGA;
            } else {
                s_capsules[i].type = PWR_LIFE;
            }
            sfx_powerup_drop();
            break;
        }
    }
}

static void detonate_tnt(s8 col, s8 row) {
    if (col < 0 || col >= BRICK_COLS || row < 0 || row >= BRICK_ROWS) return;
    if (s_bricks[row][col].type != BRK_TNT) return;

    // Clear center TNT
    s_bricks[row][col].type = BRK_EMPTY;
    render_brick_tiles(col, row, BRK_EMPTY, 0);
    s_destructible_remaining--;
    s_score += 150;
    sfx_tnt_explode();
    spawn_particles(PLAYFIELD_X_MIN + col * 16, BRICK_START_Y + row * 8, 1);

    // Queue adjacent TNTs to prevent deep stack frames
    s8 queue_c[8];
    s8 queue_r[8];
    u8 queue_len = 0;

    for (s8 dr = -1; dr <= 1; dr++) {
        for (s8 dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue;
            s8 nc = col + dc;
            s8 nr = row + dr;
            if (nc < 0 || nc >= BRICK_COLS || nr < 0 || nr >= BRICK_ROWS) continue;

            u8 ntype = s_bricks[nr][nc].type;
            if (ntype == BRK_EMPTY || ntype == BRK_GOLD) continue;

            if (ntype == BRK_TNT) {
                if (queue_len < 8) {
                    queue_c[queue_len] = nc;
                    queue_r[queue_len] = nr;
                    queue_len++;
                }
            } else if (ntype == BRK_REGEN) {
                for (int t = 0; t < MAX_REGEN_TRACKERS; t++) {
                    if (s_regen_trackers[t].active && s_regen_trackers[t].col == nc && s_regen_trackers[t].row == nr) {
                        s_regen_trackers[t].hp--;
                        s_regen_trackers[t].heal_timer = 300;
                        if (s_regen_trackers[t].hp > 0) {
                            render_brick_tiles(nc, nr, BRK_REGEN, s_regen_trackers[t].hp);
                            spawn_particles(PLAYFIELD_X_MIN + nc * 16, BRICK_START_Y + nr * 8, 3);
                        } else {
                            s_bricks[nr][nc].type = BRK_EMPTY;
                            render_brick_tiles(nc, nr, BRK_EMPTY, 0);
                            s_destructible_remaining--;
                            s_score += 300;
                            s_regen_trackers[t].active = false;
                            spawn_particles(PLAYFIELD_X_MIN + nc * 16, BRICK_START_Y + nr * 8, 3);
                            if (!s_tnt_capsule_dropped && (rng_next() % 100 < 35)) {
                                s_tnt_capsule_dropped = true;
                                spawn_capsule(PLAYFIELD_X_MIN + nc * 16, BRICK_START_Y + nr * 8);
                            }
                        }
                        break;
                    }
                }
            } else if (ntype == BRK_SILVER) {
                s_bricks[nr][nc].hp--;
                if (s_bricks[nr][nc].hp == 1) {
                    render_brick_tiles(nc, nr, BRK_SILVER, 1);
                    spawn_particles(PLAYFIELD_X_MIN + nc * 16, BRICK_START_Y + nr * 8, BRK_SILVER);
                } else {
                    s_bricks[nr][nc].type = BRK_EMPTY;
                    render_brick_tiles(nc, nr, BRK_EMPTY, 0);
                    s_destructible_remaining--;
                    s_score += 500;
                    spawn_particles(PLAYFIELD_X_MIN + nc * 16, BRICK_START_Y + nr * 8, BRK_SILVER);
                    if (!s_tnt_capsule_dropped && (rng_next() % 100 < 35)) {
                        s_tnt_capsule_dropped = true;
                        spawn_capsule(PLAYFIELD_X_MIN + nc * 16, BRICK_START_Y + nr * 8);
                    }
                }
            } else {
                static const u16 s_pts[] = {0, 100, 120, 150, 180, 200, 250};
                s_score += s_pts[ntype];
                s_bricks[nr][nc].type = BRK_EMPTY;
                render_brick_tiles(nc, nr, BRK_EMPTY, 0);
                s_destructible_remaining--;
                spawn_particles(PLAYFIELD_X_MIN + nc * 16, BRICK_START_Y + nr * 8, ntype);
                if (!s_tnt_capsule_dropped && (rng_next() % 100 < 25)) {
                    s_tnt_capsule_dropped = true;
                    spawn_capsule(PLAYFIELD_X_MIN + nc * 16, BRICK_START_Y + nr * 8);
                }
            }
        }
    }

    // Detonate queued neighboring TNTs in cascade
    for (u8 i = 0; i < queue_len; i++) {
        detonate_tnt(queue_c[i], queue_r[i]);
    }
}

static void apply_powerup(PowerUpType type) {
    sfx_powerup_get();
    s_pwr_toast_timer = 90; // 1.5 seconds banner at 60 FPS
    s_pwr_toast_type = type;

    if (type != PWR_LIFE && type != PWR_SHIELD && type != PWR_MEGA) {
        s_paddle.active_pwr = type;
        s_paddle.pwr_timer = 1800; // ~30 seconds duration
    }

    switch (type) {
        case PWR_WIDE:
            s_paddle.width = 48;
            s_paddle.laser_equipped = false;
            s_paddle.catch_equipped = false;
            draw_text(3, 1, ">> EXPAND PADDLE <<");
            break;
        case PWR_LASER:
            s_paddle.width = 32;
            s_paddle.laser_equipped = true;
            s_paddle.catch_equipped = false;
            draw_text(3, 1, ">>  LASER CANNON <<");
            break;
        case PWR_MULTI:
            draw_text(3, 1, ">>   MULTI-BALL  <<");
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
            draw_text(3, 1, ">>  CATCH MAGNET <<");
            break;
        case PWR_SLOW:
            draw_text(3, 1, ">>   SLOW SPEED  <<");
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
            draw_text(3, 1, ">>   EXTRA LIFE  <<");
            break;
        case PWR_SHIELD:
            s_shield_hits = 2;
            render_shield_barrier();
            draw_text(3, 1, ">> BARRIER ACTIVE <<");
            break;
        case PWR_MEGA:
            s_mega_timer = 720; // ~12 seconds @ 60 FPS
            draw_text(3, 1, ">>  MEGA PIERCE  <<");
            break;
        default: break;
    }

    update_hud_text();
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

    // Kinetic Moving Bricks Patrol Update
    for (int k = 0; k < MAX_KINETIC_BRICKS; k++) {
        if (!s_kinetic_bricks[k].active) continue;
        s_kinetic_bricks[k].x += s_kinetic_bricks[k].vx;
        s16 kx = FP_TO_INT(s_kinetic_bricks[k].x);
        if (kx <= PLAYFIELD_X_MIN && s_kinetic_bricks[k].vx < 0) {
            s_kinetic_bricks[k].x = INT_TO_FP(PLAYFIELD_X_MIN);
            s_kinetic_bricks[k].vx = -s_kinetic_bricks[k].vx;
        } else if (kx + 16 >= PLAYFIELD_X_MAX && s_kinetic_bricks[k].vx > 0) {
            s_kinetic_bricks[k].x = INT_TO_FP(PLAYFIELD_X_MAX - 16);
            s_kinetic_bricks[k].vx = -s_kinetic_bricks[k].vx;
        }
    }

    // Regenerating Matrix Bricks Healing Logic
    for (int t = 0; t < MAX_REGEN_TRACKERS; t++) {
        if (!s_regen_trackers[t].active) continue;
        if (s_regen_trackers[t].hp < 3) {
            if (s_regen_trackers[t].heal_timer > 0) {
                s_regen_trackers[t].heal_timer--;
            } else {
                s_regen_trackers[t].hp++;
                s_regen_trackers[t].heal_timer = 300;
                render_brick_tiles(s_regen_trackers[t].col, s_regen_trackers[t].row, BRK_REGEN, s_regen_trackers[t].hp);
                sfx_regen_pulse();
            }
        }
    }

    // Mega Ball countdown & trailing sparks
    if (s_mega_timer > 0) {
        s_mega_timer--;
        if (s_mega_timer % 3 == 0) {
            for (int b = 0; b < MAX_BALLS; b++) {
                if (s_balls[b].active && !s_balls[b].stuck_to_paddle) {
                    spawn_particles(FP_TO_INT(s_balls[b].x), FP_TO_INT(s_balls[b].y), 1);
                }
            }
        }
    }

    // Boss AI and dynamic chiptune BGM update
    if (s_boss.active) {
        update_boss();
        bgm_boss_tick(s_boss.phase == BOSS_PHASE_ENRAGED);
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
        } else if (s_shield_hits > 0 && by >= 150 && s_balls[b].vy > 0) {
            // Deflected by floor energy barrier
            s_balls[b].y = INT_TO_FP(149);
            s_balls[b].vy = -s_balls[b].vy;
            s_shield_hits--;
            render_shield_barrier();
            update_hud_text();
            trigger_screen_shake(1, 6);
            if (s_shield_hits > 0) {
                sfx_barrier_hit();
                spawn_particles(bx, 152, 4);
            } else {
                sfx_barrier_shatter();
                spawn_particles(bx, 152, 7);
                spawn_particles(50, 152, 4);
                spawn_particles(130, 152, 4);
            }
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

        // Hazard collision (deflect ball, destroy hazard, award 100 pts)
        for (int h = 0; h < MAX_HAZARDS; h++) {
            if (!s_hazards[h].active) continue;
            s16 hx = FP_TO_INT(s_hazards[h].x);
            s16 hy = FP_TO_INT(s_hazards[h].y);
            s16 cur_bx = FP_TO_INT(s_balls[b].x);
            s16 cur_by = FP_TO_INT(s_balls[b].y);

            if (cur_bx + 7 >= hx && cur_bx <= hx + 7 && cur_by + 7 >= hy && cur_by <= hy + 7) {
                s_hazards[h].active = false;
                sfx_hazard_hit();
                spawn_particles(hx, hy, 0);
                trigger_screen_shake(1, 6);
                s_score += 100;
                if (s_score > s_high_score) {
                    s_high_score = s_score;
                    save_set_high_score(s_high_score);
                }
                update_hud_text();

                // Deflect ball away from hazard preserving speed
                s16 diff_x = (cur_bx + 4) - (hx + 4);
                s16 diff_y = (cur_by + 4) - (hy + 4);
                s16 abs_x = (diff_x >= 0) ? diff_x : -diff_x;
                s16 abs_y = (diff_y >= 0) ? diff_y : -diff_y;
                if (abs_x > abs_y) {
                    if (diff_x < 0 && s_balls[b].vx > 0) s_balls[b].vx = -s_balls[b].vx;
                    else if (diff_x > 0 && s_balls[b].vx < 0) s_balls[b].vx = -s_balls[b].vx;
                } else {
                    if (diff_y < 0 && s_balls[b].vy > 0) s_balls[b].vy = -s_balls[b].vy;
                    else if (diff_y > 0 && s_balls[b].vy < 0) s_balls[b].vy = -s_balls[b].vy;
                }
                sfx_bounce();
                break;
            }
        }

        // Kinetic Moving Brick collision (deflect ball, damage kinetic brick, award pts)
        for (int k = 0; k < MAX_KINETIC_BRICKS; k++) {
            if (!s_kinetic_bricks[k].active) continue;
            s16 kx = FP_TO_INT(s_kinetic_bricks[k].x);
            s16 ky = FP_TO_INT(s_kinetic_bricks[k].y);
            s16 cur_bx = FP_TO_INT(s_balls[b].x);
            s16 cur_by = FP_TO_INT(s_balls[b].y);

            if (cur_bx + 7 >= kx && cur_bx <= kx + 15 && cur_by + 7 >= ky && cur_by <= ky + 7) {
                s16 diff_x = (cur_bx + 4) - (kx + 8);
                s16 diff_y = (cur_by + 4) - (ky + 4);
                s16 abs_x = (diff_x >= 0) ? diff_x : -diff_x;
                s16 abs_y = (diff_y >= 0) ? diff_y : -diff_y;
                if (abs_x > abs_y) {
                    if (diff_x < 0 && s_balls[b].vx > 0) s_balls[b].vx = -s_balls[b].vx;
                    else if (diff_x > 0 && s_balls[b].vx < 0) s_balls[b].vx = -s_balls[b].vx;
                } else {
                    if (diff_y < 0 && s_balls[b].vy > 0) s_balls[b].vy = -s_balls[b].vy;
                    else if (diff_y > 0 && s_balls[b].vy < 0) s_balls[b].vy = -s_balls[b].vy;
                }

                s_kinetic_bricks[k].hp--;
                if (s_kinetic_bricks[k].hp == 0) {
                    s_kinetic_bricks[k].active = false;
                    s_score += 300;
                    sfx_brick_hit();
                    spawn_particles(kx, ky, 7);
                    if (rng_next() % 100 < 30) spawn_capsule(kx, ky);
                } else {
                    sfx_kinetic_hit();
                    spawn_particles(kx, ky, 7);
                }
                if (s_score > s_high_score) {
                    s_high_score = s_score;
                    save_set_high_score(s_high_score);
                }
                update_hud_text();
                break;
            }
        }

        // Ball vs Boss Entity
        if (s_boss.active && s_boss.phase != BOSS_PHASE_DYING) {
            s16 cur_bx = FP_TO_INT(s_balls[b].x);
            s16 cur_by = FP_TO_INT(s_balls[b].y);

            // Ball vs Pods
            for (int p = 0; p < MAX_BOSS_PODS; p++) {
                if (s_boss.pods[p].active) {
                    s16 px_pod = FP_TO_INT(s_boss.pods[p].x);
                    s16 py_pod = FP_TO_INT(s_boss.pods[p].y);
                    if (cur_bx + 7 >= px_pod && cur_bx <= px_pod + 15 && cur_by + 7 >= py_pod && cur_by <= py_pod + 15) {
                        if (s_mega_timer == 0) s_balls[b].vy = -s_balls[b].vy;
                        s_boss.pods[p].hp -= (s_mega_timer > 0) ? 2 : 1;
                        sfx_boss_hurt();
                        spawn_particles(cur_bx + 4, cur_by + 4, 3);
                        if (s_boss.pods[p].hp <= 0) {
                            s_boss.pods[p].active = false;
                            sfx_boss_pod_destroyed();
                            trigger_screen_shake(2, 10);
                            spawn_particles(px_pod + 8, py_pod + 8, 7);
                            s_score += 1000;
                        }
                        break;
                    }
                }
            }

            // Ball vs Core (32x32)
            s16 bx_boss = FP_TO_INT(s_boss.x);
            s16 by_boss = FP_TO_INT(s_boss.y);
            if (cur_bx + 7 >= bx_boss && cur_bx <= bx_boss + 31 && cur_by + 7 >= by_boss && cur_by <= by_boss + 31) {
                if (s_mega_timer == 0 || s_boss.phase == BOSS_PHASE_SHIELDED) {
                    s_balls[b].vy = -s_balls[b].vy;
                }
                if (s_boss.phase == BOSS_PHASE_SHIELDED) {
                    sfx_kinetic_hit();
                    spawn_particles(cur_bx + 4, cur_by + 4, 7);
                } else {
                    s_boss.hp -= (s_mega_timer > 0) ? 2 : 1;
                    s_boss.invuln_flash = 6;
                    sfx_boss_hurt();
                    trigger_screen_shake(2, 8);
                    spawn_particles(cur_bx + 4, cur_by + 4, 1);
                    if (s_boss.hp <= s_boss.max_hp / 2) s_boss.phase = BOSS_PHASE_ENRAGED;
                    if (s_boss.hp <= 0) {
                        s_boss.phase = BOSS_PHASE_DYING;
                        s_boss.death_timer = 0;
                        s_score += 5000;
                        sfx_boss_defeat();
                        bgm_boss_stop();
                    }
                }
            }

            // Ball vs Boss Bolts (destroy bolt on contact)
            for (int bt = 0; bt < MAX_BOSS_BOLTS; bt++) {
                if (s_boss.bolts[bt].active) {
                    s16 bbx = FP_TO_INT(s_boss.bolts[bt].x);
                    s16 bby = FP_TO_INT(s_boss.bolts[bt].y);
                    if (cur_bx + 7 >= bbx && cur_bx <= bbx + 7 && cur_by + 7 >= bby && cur_by <= bby + 7) {
                        s_boss.bolts[bt].active = false;
                        sfx_hazard_hit();
                        spawn_particles(bbx, bby, 0);
                    }
                }
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

                    if (s_mega_timer > 0 && type != BRK_GOLD) {
                        // Mega Piercing Ball plows straight through without deflection!
                    } else {
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
                    }

                    if (type == BRK_GOLD) {
                        sfx_brick_hard();
                        trigger_screen_shake(1, 4);
                    } else if (type == BRK_TNT) {
                        s_tnt_capsule_dropped = false;
                        detonate_tnt(col, row);
                    } else if (type == BRK_REGEN) {
                        for (int t = 0; t < MAX_REGEN_TRACKERS; t++) {
                            if (s_regen_trackers[t].active && s_regen_trackers[t].col == col && s_regen_trackers[t].row == row) {
                                s_regen_trackers[t].hp--;
                                s_regen_trackers[t].heal_timer = 300;
                                if (s_regen_trackers[t].hp > 0 && s_mega_timer == 0) {
                                    render_brick_tiles(col, row, BRK_REGEN, s_regen_trackers[t].hp);
                                    sfx_brick_hard();
                                    spawn_particles(PLAYFIELD_X_MIN + col * 16, BRICK_START_Y + row * 8, 3);
                                } else {
                                    s_bricks[row][col].type = BRK_EMPTY;
                                    render_brick_tiles(col, row, BRK_EMPTY, 0);
                                    s_destructible_remaining--;
                                    s_score += 300;
                                    s_regen_trackers[t].active = false;
                                    if (s_mega_timer > 0) sfx_mega_smash();
                                    else sfx_brick_hit();
                                    spawn_particles(PLAYFIELD_X_MIN + col * 16, BRICK_START_Y + row * 8, 3);
                                    if (rng_next() % 100 < 30) spawn_capsule(PLAYFIELD_X_MIN + col * 16, BRICK_START_Y + row * 8);
                                }
                                break;
                            }
                        }
                    } else if (type == BRK_SILVER) {
                        if (s_mega_timer > 0) {
                            s_bricks[row][col].type = BRK_EMPTY;
                            render_brick_tiles(col, row, BRK_EMPTY, 0);
                            s_destructible_remaining--;
                            s_score += 500;
                            sfx_mega_smash();
                            trigger_screen_shake(1, 6);
                            spawn_particles(PLAYFIELD_X_MIN + col * 16, BRICK_START_Y + row * 8, BRK_SILVER);
                            if (rng_next() % 100 < 30) spawn_capsule(PLAYFIELD_X_MIN + col * 16, BRICK_START_Y + row * 8);
                        } else {
                            s_bricks[row][col].hp--;
                            if (s_bricks[row][col].hp == 1) {
                                render_brick_tiles(col, row, BRK_SILVER, 1);
                                sfx_brick_hard();
                                s_score += 50;
                                trigger_screen_shake(1, 4);
                                spawn_particles(PLAYFIELD_X_MIN + col * 16, BRICK_START_Y + row * 8, BRK_SILVER);
                            } else {
                                s_bricks[row][col].type = BRK_EMPTY;
                                render_brick_tiles(col, row, BRK_EMPTY, 0);
                                s_destructible_remaining--;
                                s_score += 500;
                                sfx_brick_hit();
                                trigger_screen_shake(1, 6);
                                spawn_particles(PLAYFIELD_X_MIN + col * 16, BRICK_START_Y + row * 8, BRK_SILVER);
                                if (rng_next() % 100 < 30) spawn_capsule(PLAYFIELD_X_MIN + col * 16, BRICK_START_Y + row * 8);
                            }
                        }
                    } else {
                        // Regular colored brick
                        static const u16 s_pts[] = {0, 100, 120, 150, 180, 200, 250};
                        s_score += s_pts[type];
                        s_bricks[row][col].type = BRK_EMPTY;
                        render_brick_tiles(col, row, BRK_EMPTY, 0);
                        s_destructible_remaining--;
                        if (s_mega_timer > 0) sfx_mega_smash();
                        else sfx_brick_hit();
                        spawn_particles(PLAYFIELD_X_MIN + col * 16, BRICK_START_Y + row * 8, type);
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
        trigger_screen_shake(3, 16);
        spawn_particles(px + s_paddle.width / 2, py, 0);
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

        // Laser vs Hazards
        for (int h = 0; h < MAX_HAZARDS; h++) {
            if (!s_hazards[h].active) continue;
            s16 hx = FP_TO_INT(s_hazards[h].x);
            s16 hy = FP_TO_INT(s_hazards[h].y);
            if (lx + 6 >= hx && lx <= hx + 7 && ly + 7 >= hy && ly <= hy + 7) {
                s_lasers[i].active = false;
                s_hazards[h].active = false;
                sfx_hazard_hit();
                spawn_particles(hx, hy, 0);
                trigger_screen_shake(1, 6);
                s_score += 100;
                if (s_score > s_high_score) {
                    s_high_score = s_score;
                    save_set_high_score(s_high_score);
                }
                update_hud_text();
                break;
            }
        }
        // Laser vs Kinetic Moving Bricks
        for (int k = 0; k < MAX_KINETIC_BRICKS; k++) {
            if (!s_kinetic_bricks[k].active) continue;
            s16 kx = FP_TO_INT(s_kinetic_bricks[k].x);
            s16 ky = FP_TO_INT(s_kinetic_bricks[k].y);
            if (lx + 6 >= kx && lx <= kx + 15 && ly + 7 >= ky && ly <= ky + 7) {
                s_lasers[i].active = false;
                s_kinetic_bricks[k].hp--;
                if (s_kinetic_bricks[k].hp == 0) {
                    s_kinetic_bricks[k].active = false;
                    s_score += 300;
                    sfx_brick_hit();
                    spawn_particles(kx, ky, 7);
                } else {
                    sfx_kinetic_hit();
                    spawn_particles(kx, ky, 7);
                }
                update_hud_text();
                break;
            }
        }
        if (!s_lasers[i].active) continue;

        // Laser vs Boss Entity
        if (s_boss.active && s_boss.phase != BOSS_PHASE_DYING) {
            // Laser vs Pods
            for (int p = 0; p < MAX_BOSS_PODS; p++) {
                if (s_boss.pods[p].active) {
                    s16 px_pod = FP_TO_INT(s_boss.pods[p].x);
                    s16 py_pod = FP_TO_INT(s_boss.pods[p].y);
                    if (lx + 6 >= px_pod && lx <= px_pod + 15 && ly + 7 >= py_pod && ly <= py_pod + 15) {
                        s_lasers[i].active = false;
                        s_boss.pods[p].hp--;
                        sfx_boss_hurt();
                        spawn_particles(lx, ly, 3);
                        if (s_boss.pods[p].hp <= 0) {
                            s_boss.pods[p].active = false;
                            sfx_boss_pod_destroyed();
                            trigger_screen_shake(2, 10);
                            spawn_particles(px_pod + 8, py_pod + 8, 7);
                            s_score += 1000;
                        }
                        break;
                    }
                }
            }
            if (!s_lasers[i].active) continue;

            // Laser vs Core
            s16 bx_boss = FP_TO_INT(s_boss.x);
            s16 by_boss = FP_TO_INT(s_boss.y);
            if (lx + 6 >= bx_boss && lx <= bx_boss + 31 && ly + 7 >= by_boss && ly <= by_boss + 31) {
                s_lasers[i].active = false;
                if (s_boss.phase == BOSS_PHASE_SHIELDED) {
                    sfx_kinetic_hit();
                    spawn_particles(lx, ly, 7);
                } else {
                    s_boss.hp--;
                    s_boss.invuln_flash = 6;
                    sfx_boss_hurt();
                    spawn_particles(lx, ly, 1);
                    if (s_boss.hp <= s_boss.max_hp / 2) s_boss.phase = BOSS_PHASE_ENRAGED;
                    if (s_boss.hp <= 0) {
                        s_boss.phase = BOSS_PHASE_DYING;
                        s_boss.death_timer = 0;
                        s_score += 5000;
                        sfx_boss_defeat();
                        bgm_boss_stop();
                    }
                }
                continue;
            }

            // Laser vs Boss Bolts
            for (int bt = 0; bt < MAX_BOSS_BOLTS; bt++) {
                if (s_boss.bolts[bt].active) {
                    s16 bbx = FP_TO_INT(s_boss.bolts[bt].x);
                    s16 bby = FP_TO_INT(s_boss.bolts[bt].y);
                    if (lx + 6 >= bbx && lx <= bbx + 7 && ly + 7 >= bby && ly <= bby + 7) {
                        s_lasers[i].active = false;
                        s_boss.bolts[bt].active = false;
                        sfx_hazard_hit();
                        spawn_particles(bbx, bby, 0);
                        break;
                    }
                }
            }
            if (!s_lasers[i].active) continue;
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
                    if (type == BRK_GOLD) {
                        sfx_brick_hard();
                        trigger_screen_shake(1, 4);
                    } else if (type == BRK_TNT) {
                        s_tnt_capsule_dropped = false;
                        detonate_tnt(col, row);
                    } else if (type == BRK_REGEN) {
                        for (int t = 0; t < MAX_REGEN_TRACKERS; t++) {
                            if (s_regen_trackers[t].active && s_regen_trackers[t].col == col && s_regen_trackers[t].row == row) {
                                s_regen_trackers[t].hp--;
                                s_regen_trackers[t].heal_timer = 300;
                                if (s_regen_trackers[t].hp > 0) {
                                    render_brick_tiles(col, row, BRK_REGEN, s_regen_trackers[t].hp);
                                    sfx_brick_hard();
                                    spawn_particles(PLAYFIELD_X_MIN + col * 16, BRICK_START_Y + row * 8, 3);
                                } else {
                                    s_bricks[row][col].type = BRK_EMPTY;
                                    render_brick_tiles(col, row, BRK_EMPTY, 0);
                                    s_destructible_remaining--;
                                    s_score += 300;
                                    s_regen_trackers[t].active = false;
                                    sfx_brick_hit();
                                    spawn_particles(PLAYFIELD_X_MIN + col * 16, BRICK_START_Y + row * 8, 3);
                                }
                                break;
                            }
                        }
                    } else if (type == BRK_SILVER) {
                        s_bricks[row][col].hp--;
                        if (s_bricks[row][col].hp == 1) {
                            render_brick_tiles(col, row, BRK_SILVER, 1);
                            sfx_brick_hard();
                            s_score += 50;
                            spawn_particles(PLAYFIELD_X_MIN + col * 16, BRICK_START_Y + row * 8, BRK_SILVER);
                        } else {
                            s_bricks[row][col].type = BRK_EMPTY;
                            render_brick_tiles(col, row, BRK_EMPTY, 0);
                            s_destructible_remaining--;
                            s_score += 500;
                            sfx_brick_hit();
                            spawn_particles(PLAYFIELD_X_MIN + col * 16, BRICK_START_Y + row * 8, BRK_SILVER);
                        }
                    } else {
                        // Regular colored brick
                        static const u16 s_pts[] = {0, 100, 120, 150, 180, 200, 250};
                        s_score += s_pts[type];
                        s_bricks[row][col].type = BRK_EMPTY;
                        render_brick_tiles(col, row, BRK_EMPTY, 0);
                        s_destructible_remaining--;
                        sfx_brick_hit();
                        spawn_particles(PLAYFIELD_X_MIN + col * 16, BRICK_START_Y + row * 8, type);
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

    // 5. Floating Hazards Update
    s_hazard_spawn_timer++;
    if (s_hazard_spawn_timer >= 900) {
        s_hazard_spawn_timer = 0;
        for (int i = 0; i < MAX_HAZARDS; i++) {
            if (!s_hazards[i].active) {
                s_hazards[i].active = true;
                s16 start_x = 32 + (rng_next() % 118);
                s_hazards[i].base_x = INT_TO_FP(start_x);
                s_hazards[i].x = s_hazards[i].base_x;
                s_hazards[i].y = INT_TO_FP(PLAYFIELD_Y_MIN + 2);
                s_hazards[i].vy = 64; // 0.25 px/frame descent
                s_hazards[i].wave_timer = rng_next() % 256;
                s_hazards[i].anim_frame = 0;
                sfx_hazard_spawn();
                break;
            }
        }
    }

    for (int i = 0; i < MAX_HAZARDS; i++) {
        if (!s_hazards[i].active) continue;
        s_hazards[i].wave_timer++;
        s_hazards[i].anim_frame = (s_hazards[i].wave_timer >> 4) & 1;

        u8 angle = (s_hazards[i].wave_timer >> 2) & 31;
        s16 offset_x = s_sin32[angle];
        s_hazards[i].x = s_hazards[i].base_x + INT_TO_FP(offset_x);
        s_hazards[i].y += s_hazards[i].vy;

        s16 hx = FP_TO_INT(s_hazards[i].x);
        s16 hy = FP_TO_INT(s_hazards[i].y);

        if (hy >= PLAYFIELD_Y_MAX) {
            s_hazards[i].active = false;
            continue;
        }

        // Hazard vs Paddle collision (costs 1 life)
        if (hy + 7 >= py && hy <= py + 8 && hx + 7 >= px && hx <= px + s_paddle.width) {
            s_hazards[i].active = false;
            s_lives--;
            sfx_hazard_hit();
            sfx_life_lost();
            spawn_particles(px + s_paddle.width / 2, py, 0);
            trigger_screen_shake(3, 20);
            if (s_lives > 0) {
                reset_paddle_and_ball();
            } else {
                s_state = STATE_GAME_OVER;
                s_state_timer = 0;
            }
            return;
        }
    }

    // 6. Particle Physics Update
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!s_particles[i].active) continue;
        s_particles[i].x += s_particles[i].vx;
        s_particles[i].y += s_particles[i].vy;
        s_particles[i].vy += 16; // gravity ~ 0.0625 px/f
        if (s_particles[i].life > 0) {
            s_particles[i].life--;
        } else {
            s_particles[i].active = false;
        }
    }

    // 7. Power-up Toast Banner Countdown
    if (s_pwr_toast_timer > 0) {
        s_pwr_toast_timer--;
        if (s_pwr_toast_timer == 0) {
            draw_text(3, 1, "                   ");
            update_hud_text();
        }
    }


    // Check Stage Clear
    bool stage_cleared = false;
    if (s_boss.active) {
        if (s_boss.phase == BOSS_PHASE_DYING && s_boss.death_timer >= 90) {
            stage_cleared = true;
            s_boss.active = false;
        }
    } else {
        if (s_destructible_remaining == 0) {
            stage_cleared = true;
        }
    }

    if (stage_cleared) {
        sfx_stage_clear();
        s_score += 1000 * s_current_stage;
        if (s_score > s_high_score) {
            s_high_score = s_score;
        }

        // Unlock next stage in SRAM
        if (s_current_stage + 1 > s_max_stage_unlocked && s_current_stage < MAX_STAGES) {
            s_max_stage_unlocked = s_current_stage + 1;
        }
        save_record_progress(s_high_score, s_max_stage_unlocked);
        update_hud_text();

        if (s_current_stage >= MAX_STAGES) {
            s_state = STATE_VICTORY;
            jingle_victory();
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

    REG_BG0HOFS = 0;
    REG_BG0VOFS = 0;

    switch (s_state) {
        case STATE_TITLE:
            if (key_was_pressed(KEY_LEFT)) {
                if (s_selected_stage > 1) {
                    s_selected_stage--;
                    sfx_menu_tick();
                }
            } else if (key_was_pressed(KEY_RIGHT)) {
                if (s_selected_stage < s_max_stage_unlocked) {
                    s_selected_stage++;
                    sfx_menu_tick();
                }
            }
            if (key_was_pressed(KEY_START) || key_was_pressed(KEY_A)) {
                game_start_new();
            }
            break;

        case STATE_PLAYING:
            if (key_was_pressed(KEY_START)) {
                s_state = STATE_PAUSED;
                draw_text(6, 11, "PAUSED");
                sfx_bounce();
            } else {
                update_playing();
            }
            break;

        case STATE_PAUSED:
            if (key_was_pressed(KEY_START)) {
                s_state = STATE_PLAYING;
                draw_text(6, 11, "      ");
                sfx_bounce();
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
                s_state_timer = 0;
                jingle_title();
            }
            break;

        case STATE_VICTORY:
            if (s_state_timer == 1) {
                draw_text(3, 11, "VICTORY! ALL CLEAR");
            }
            if (s_state_timer > 200 && (key_was_pressed(KEY_START) || key_was_pressed(KEY_A))) {
                draw_text(3, 11, "                  ");
                init_background_playfield();
                s_state = STATE_TITLE;
                s_state_timer = 0;
                jingle_title();
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
        draw_text(4, 4, "BRICK BREAKER");
        draw_text(7, 6, "ARKANOID DX");

        // Display stage selector with interactive arrow indicators
        char st_buf[14];
        st_buf[0] = 'S'; st_buf[1] = 'T'; st_buf[2] = 'A'; st_buf[3] = 'G'; st_buf[4] = 'E';
        st_buf[5] = ' ';
        st_buf[6] = (s_selected_stage > 1) ? '<' : ' ';
        st_buf[7] = ' ';
        st_buf[8] = '0' + (s_selected_stage / 10);
        st_buf[9] = '0' + (s_selected_stage % 10);
        st_buf[10] = ' ';
        st_buf[11] = (s_selected_stage < s_max_stage_unlocked) ? '>' : ' ';
        st_buf[12] = '\0';
        draw_text(6, 9, st_buf);

        if (s_selected_stage >= 11) {
            draw_text(6, 11, "ZONE: CYBER");
        } else {
            draw_text(6, 11, "ZONE: SPACE");
        }

        if ((s_state_timer / 30) % 2 == 0) {
            draw_text(4, 13, "PRESS START");
        } else {
            draw_text(4, 13, "           ");
        }
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

    // 1b. Paddle Thruster Exhaust Flames
    if (s_paddle.vx != 0) {
        if (s_paddle.vx < 0) {
            // Moving Left -> Exhaust flame under right side
            s16 flame_x = px + s_paddle.width - 6;
            s16 flame_y = py + 7 + ((s_state_timer & 2) ? 1 : 0);
            oam_set(sid++, flame_x, flame_y, ATTR0_SQUARE, ATTR1_SIZE_8, SPRITE_TILE_THRUSTER_L, 0, false, false);
        } else {
            // Moving Right -> Exhaust flame under left side
            s16 flame_x = px - 2;
            s16 flame_y = py + 7 + ((s_state_timer & 2) ? 1 : 0);
            oam_set(sid++, flame_x, flame_y, ATTR0_SQUARE, ATTR1_SIZE_8, SPRITE_TILE_THRUSTER_R, 0, false, false);
        }
    }

    // 2. Render Balls (8x8)
    u16 ball_tile = (s_mega_timer > 0) ? SPRITE_TILE_BALL_MEGA : SPRITE_TILE_BALL;
    for (int i = 0; i < MAX_BALLS; i++) {
        if (!s_balls[i].active) continue;
        s16 bx = FP_TO_INT(s_balls[i].x);
        s16 by = FP_TO_INT(s_balls[i].y);
        oam_set(sid++, bx, by, ATTR0_SQUARE, ATTR1_SIZE_8, ball_tile, 0, false, false);
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
            case PWR_WIDE:   tile = SPRITE_TILE_PWR_WIDE;   break;
            case PWR_LASER:  tile = SPRITE_TILE_PWR_LASER;  break;
            case PWR_MULTI:  tile = SPRITE_TILE_PWR_MULTI;  break;
            case PWR_CATCH:  tile = SPRITE_TILE_PWR_CATCH;  break;
            case PWR_SLOW:   tile = SPRITE_TILE_PWR_SLOW;   break;
            case PWR_LIFE:   tile = SPRITE_TILE_PWR_LIFE;   break;
            case PWR_SHIELD: tile = SPRITE_TILE_PWR_SHIELD; break;
            case PWR_MEGA:   tile = SPRITE_TILE_PWR_MEGA;   break;
            default: break;
        }
        oam_set(sid++, cx, cy, ATTR0_WIDE, ATTR1_SIZE_8, tile, 0, false, false);
    }

    // 5. Render Kinetic Moving Bricks (16x8)
    for (int k = 0; k < MAX_KINETIC_BRICKS; k++) {
        if (!s_kinetic_bricks[k].active) continue;
        s16 kx = FP_TO_INT(s_kinetic_bricks[k].x);
        s16 ky = FP_TO_INT(s_kinetic_bricks[k].y);
        oam_set(sid++, kx, ky, ATTR0_WIDE, ATTR1_SIZE_8, SPRITE_TILE_KINETIC_BRICK, 0, false, false);
    }

    // 6. Render Floating Hazards (8x8)
    for (int i = 0; i < MAX_HAZARDS; i++) {
        if (!s_hazards[i].active) continue;
        s16 hx = FP_TO_INT(s_hazards[i].x);
        s16 hy = FP_TO_INT(s_hazards[i].y);
        u16 htile = (s_hazards[i].anim_frame == 0) ? SPRITE_TILE_HAZARD_1 : SPRITE_TILE_HAZARD_2;
        oam_set(sid++, hx, hy, ATTR0_SQUARE, ATTR1_SIZE_8, htile, 0, false, false);
    }

    // 7. Render Shatter Debris Particles (8x8)
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!s_particles[i].active) continue;
        s16 px_pt = FP_TO_INT(s_particles[i].x);
        s16 py_pt = FP_TO_INT(s_particles[i].y);
        oam_set(sid++, px_pt, py_pt, ATTR0_SQUARE, ATTR1_SIZE_8, SPRITE_TILE_PARTICLE, s_particles[i].pal, false, false);
    }

    // 8. Render Boss Entities (Stage 10 & Stage 20)
    if (s_boss.active) {
        s16 boss_x = FP_TO_INT(s_boss.x);
        s16 boss_y = FP_TO_INT(s_boss.y);

        bool flash = (s_boss.invuln_flash > 0 && (s_boss.invuln_flash % 2 == 0)) ||
                     (s_boss.phase == BOSS_PHASE_DYING && ((s_boss.death_timer / 4) % 2 == 0));

        if (!flash) {
            u8 boss_pal = (s_boss.phase == BOSS_PHASE_ENRAGED) ? 1 : 0;
            oam_set(sid++, boss_x, boss_y, ATTR0_SQUARE, ATTR1_SIZE_32, SPRITE_TILE_BOSS_CORE, boss_pal, false, false);
        }

        // Render Satellite Defense Pods
        for (int p = 0; p < MAX_BOSS_PODS; p++) {
            if (s_boss.pods[p].active) {
                s16 px_pod = FP_TO_INT(s_boss.pods[p].x);
                s16 py_pod = FP_TO_INT(s_boss.pods[p].y);
                oam_set(sid++, px_pod, py_pod, ATTR0_SQUARE, ATTR1_SIZE_16, SPRITE_TILE_BOSS_POD, 0, false, false);
            }
        }

        // Render Boss Plasma Bolts
        for (int b = 0; b < MAX_BOSS_BOLTS; b++) {
            if (s_boss.bolts[b].active) {
                s16 bbx = FP_TO_INT(s_boss.bolts[b].x);
                s16 bby = FP_TO_INT(s_boss.bolts[b].y);
                oam_set(sid++, bbx, bby, ATTR0_SQUARE, ATTR1_SIZE_8, SPRITE_TILE_BOSS_BOLT, 0, false, false);
            }
        }
    }

    // 9. Sidebar Lives Icons
    for (s8 i = 0; i < s_lives && i < 5; i++) {
        oam_set(sid++, 196 + (i * 8), 136, ATTR0_SQUARE, ATTR1_SIZE_8, SPRITE_TILE_MINI_PADDLE, 0, false, false);
    }

    // 10. Sidebar Active Powerup Capsule Icon (16x8)
    PowerUpType disp_pwr = s_paddle.active_pwr;
    if (s_pwr_toast_timer > 0 && s_pwr_toast_type == PWR_LIFE) {
        disp_pwr = PWR_LIFE;
    } else if (s_pwr_toast_timer > 0 && s_pwr_toast_type == PWR_SHIELD) {
        disp_pwr = PWR_SHIELD;
    } else if (s_pwr_toast_timer > 0 && s_pwr_toast_type == PWR_MEGA) {
        disp_pwr = PWR_MEGA;
    } else if (disp_pwr == PWR_NONE) {
        if (s_mega_timer > 0) disp_pwr = PWR_MEGA;
        else if (s_shield_hits > 0) disp_pwr = PWR_SHIELD;
    }

    if (disp_pwr != PWR_NONE) {
        u16 btile = SPRITE_TILE_PWR_WIDE;
        switch (disp_pwr) {
            case PWR_WIDE:   btile = SPRITE_TILE_PWR_WIDE;   break;
            case PWR_LASER:  btile = SPRITE_TILE_PWR_LASER;  break;
            case PWR_MULTI:  btile = SPRITE_TILE_PWR_MULTI;  break;
            case PWR_CATCH:  btile = SPRITE_TILE_PWR_CATCH;  break;
            case PWR_SLOW:   btile = SPRITE_TILE_PWR_SLOW;   break;
            case PWR_LIFE:   btile = SPRITE_TILE_PWR_LIFE;   break;
            case PWR_SHIELD: btile = SPRITE_TILE_PWR_SHIELD; break;
            case PWR_MEGA:   btile = SPRITE_TILE_PWR_MEGA;   break;
            default: break;
        }
        oam_set(sid++, 208, 104, ATTR0_WIDE, ATTR1_SIZE_8, btile, 0, false, false);
    }
}

GameState game_get_state(void) {
    return s_state;
}
