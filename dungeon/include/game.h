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
    STATE_PLAY,
    STATE_DIALOG,
    STATE_INVENTORY,
    STATE_GAMEOVER,
    STATE_VICTORY
} GameStateEnum;

typedef enum {
    ROOM_CAMP           = 0,
    ROOM_UPPER_CRYPT    = 1,
    ROOM_AQUEDUCT       = 2,
    ROOM_SANCTUM        = 3,
    ROOM_SECRET_VAULT   = 4,
    ROOM_COUNT          = 5
} DungeonRoomId;

typedef enum {
    PLAYER_IDLE,
    PLAYER_WALK,
    PLAYER_SLASH,
    PLAYER_ROLL,
    PLAYER_HURT
} PlayerState;

typedef struct {
    fixed_t x, y;
    fixed_t vx, vy;
    Direction dir;
    u8 hearts;          // Current health (half-hearts: 6 = 3 full hearts)
    u8 max_hearts;      // Max health (6..16)
    u8 mp;              // Magic points (0..100)
    u16 gold;           // Coins
    u8 small_keys;      // Small dungeon keys
    bool has_boss_key;  // Big Boss Key
    u8 unlocked_tools;  // Bitflags of SubWeaponType
    SubWeaponType active_tool;
    PlayerState state;
    u8 state_timer;
    u8 invuln_timer;
    u8 walk_frame;
    u8 anim_timer;
} Player;

typedef enum {
    ACTOR_NONE,
    ACTOR_SLIME,
    ACTOR_SKELETON,
    ACTOR_BAT,
    ACTOR_BLADE_TRAP,
    ACTOR_GOLEM_BODY,
    ACTOR_GOLEM_FIST,
    ACTOR_STALACTITE,
    ACTOR_NPC_MERCHANT
} ActorType;

typedef struct {
    ActorType type;
    fixed_t x, y;
    fixed_t vx, vy;
    Direction dir;
    s8 hp;
    u8 max_hp;
    u8 state;
    u16 timer;
    u8 invuln_timer;
    bool active;
    u8 extra; // Custom parameter (e.g. fist index, trap axis)
} Actor;

#define MAX_ACTORS 16

typedef enum {
    PROJ_NONE,
    PROJ_SWORD_ARC,
    PROJ_BOOMERANG,
    PROJ_BOMB,
    PROJ_EXPLOSION,
    PROJ_FIREBALL,
    PROJ_GOLEM_LASER
} ProjType;

typedef struct {
    ProjType type;
    fixed_t x, y;
    fixed_t vx, vy;
    Direction dir;
    u16 timer;
    u8 damage;
    bool active;
} Projectile;

#define MAX_PROJECTILES 8

typedef enum {
    PICKUP_NONE,
    PICKUP_HEART,
    PICKUP_MP,
    PICKUP_COIN,
    PICKUP_KEY,
    PICKUP_BOSS_KEY,
    PICKUP_HEART_CONTAINER
} PickupType;

typedef struct {
    PickupType type;
    fixed_t x, y;
    u16 timer;
    bool active;
} Pickup;

#define MAX_PICKUPS 8

typedef struct {
    MetatileId tiles[ROOM_HEIGHT][ROOM_WIDTH];
} RoomMap;

// Global Game State
typedef struct {
    GameStateEnum state;
    DungeonRoomId current_room;
    Player player;
    Actor actors[MAX_ACTORS];
    Projectile projectiles[MAX_PROJECTILES];
    Pickup pickups[MAX_PICKUPS];
    RoomMap room;
    u32 room_cleared_flags;
    u8 golem_phase;
    s8 golem_core_hp;
    bool boss_defeated;
    const char* dialog_lines[3];
    u8 dialog_count;
    u8 dialog_line_idx;
    u8 save_slot;
    u16 game_frame;
} GameState;

extern GameState g_game;

void game_init(void);
void game_update(void);
void game_draw(void);

void game_load_room(DungeonRoomId room_id, fixed_t spawn_x, fixed_t spawn_y);
void game_show_dialog(const char* l1, const char* l2, const char* l3);

#ifdef __cplusplus
}
#endif

#endif // GAME_H
