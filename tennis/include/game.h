#ifndef GAME_H
#define GAME_H

#include "gba.h"
#include "assets.h"
#include "audio.h"
#include "save.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    STATE_TITLE,
    STATE_MODE_SELECT,
    STATE_EXHIBITION_SETUP,
    STATE_SURFACE_SELECT,
    STATE_MATCH_SERVE_WAIT,
    STATE_MATCH_RALLY,
    STATE_MATCH_POINT_OVER,
    STATE_MATCH_GAME_OVER,
    STATE_TARGET_PRACTICE,
    STATE_TROPHY_CEREMONY
} GameStateEnum;

typedef enum {
    MODE_TOURNAMENT,
    MODE_EXHIBITION,
    MODE_TARGET_PRACTICE
} TennisMode;

typedef enum {
    AI_BASELINER,
    AI_VOLLEYER,
    AI_POWER_SERVER
} AiStyle;

typedef enum {
    ACTOR_STATE_READY,
    ACTOR_STATE_RUN,
    ACTOR_STATE_SWING_FH,
    ACTOR_STATE_SWING_BH,
    ACTOR_STATE_TOSS,
    ACTOR_STATE_SERVE_HIT,
    ACTOR_STATE_SMASH
} ActorState;

typedef struct {
    fixed_t x, y;
    fixed_t vx, vy;
    ActorState state;
    u8 timer;
    u8 anim_frame;
    u8 run_anim;
    bool is_serving;
    bool side_deuce; // True = serving from right side, False = left
    bool is_charging;
    u8 charge_power;
    ShotType charge_shot;
    bool ai_net_rushing;
} TennisPlayer;

typedef struct {
    fixed_t x, y, z;
    fixed_t vx, vy, vz;
    fixed_t target_x, target_y;
    u16 flight_time;
    u16 flight_elapsed;
    u8 bounce_count;
    u8 last_hitter;   // 0 = player, 1 = opponent
    bool in_air;
    bool first_bounce_evaluated;
    ShotType last_shot;
    u8 trail_timer;
    u8 trail_type;
} TennisBall;

typedef struct {
    s16 x, y;
    u8 timer;
    bool active;
} ChalkPuff;

typedef struct {
    s16 x, y;
    u16 points;
    u8 timer;
    bool active;
    u8 ring_type; // 0=standard, 1=bullseye
} TargetRing;

#define MAX_TARGETS 4

typedef struct {
    GameStateEnum state;
    TennisMode mode;
    CourtSurface surface;
    AiStyle ai_style;

    TennisPlayer player;
    TennisPlayer opponent;
    TennisBall ball;
    ChalkPuff chalk;
    TargetRing targets[MAX_TARGETS];

    // Scoring
    u8 player_points;    // 0=0, 1=15, 2=30, 3=40, 4=AD
    u8 opponent_points;  // 0=0, 1=15, 2=30, 3=40, 4=AD
    u8 player_games;
    u8 opponent_games;
    u8 server;           // 0 = player, 1 = opponent
    u8 fault_count;      // 0 = none, 1 = second serve, 2 = double fault

    // Match Stats
    u16 current_rally;
    u16 longest_rally_match;
    u16 last_serve_mph;
    u32 target_score;
    u16 target_timer;
    u8 target_combo;
    u16 ball_machine_timer;

    // Menu Navigation
    u8 menu_mode;        // 0 = Tournament, 1 = Exhibition, 2 = Target Practice
    u8 menu_opp;         // 0 = Vance, 1 = Sato, 2 = Thorne
    u8 menu_surface;     // 0 = Grass, 1 = Clay, 2 = Hard
    u8 menu_setup_step;  // 0 = Surface, 1 = Opponent

    // Tournament
    u8 tournament_round; // 0 = Quarter, 1 = Semi, 2 = Final
    const char* announcement_str;
    u8 announcement_timer;

    TennisSaveData save_data;
    u16 game_frame;
} GameState;

extern GameState g_tennis;

void game_init(void);
void game_update(void);
void game_draw(void);

void game_start_match(CourtSurface surface, AiStyle ai, TennisMode mode);
void game_award_point(u8 winner);

#ifdef __cplusplus
}
#endif

#endif // GAME_H
