#ifndef GAME_H
#define GAME_H

#include "types.h"

#define MAX_UNITS 16

// Terrain Types
typedef enum {
    TERR_PLAINS = 0,
    TERR_FOREST,
    TERR_MOUNTAIN,
    TERR_RIVER,
    TERR_ROAD,
    TERR_BRIDGE,
    TERR_CITY,
    TERR_HQ_PLAYER,
    TERR_HQ_ENEMY
} TerrainType;

// Unit Types
typedef enum {
    UNIT_WALKER = 0,    // Assault Walker Mech (Move: 4, Captures HQ)
    UNIT_RECON,         // Recon Drone Buggy (Move: 6, High Vision)
    UNIT_TANK,          // Heavy Hover Tank (Move: 5, Heavy Cannon)
    UNIT_ARTILLERY,     // Siege Artillery (Move: 3, Range: 2-3 Indirect)
    UNIT_VTOL           // Airborne Gunship (Move: 6, Air Flight)
} UnitClass;

// Teams
typedef enum {
    TEAM_PLAYER = 0,
    TEAM_ENEMY  = 1
} Team;

// Turn Phase
typedef enum {
    PHASE_PLAYER = 0,
    PHASE_ENEMY  = 1
} TurnPhase;

// Game State
typedef enum {
    STATE_TITLE = 0,
    STATE_MISSION_INTRO,
    STATE_BATTLE,
    STATE_MISSION_CLEAR,
    STATE_GAME_OVER,
    STATE_VICTORY
} GameState;

// Player Interaction Sub-States
typedef enum {
    SUB_IDLE = 0,
    SUB_UNIT_SELECTED,
    SUB_ACTION_MENU,
    SUB_TARGET_SELECT,
    SUB_COMBAT_ANIM,
    SUB_CO_TARGET,
    SUB_FIELD_MENU
} SubState;

// Unit Struct
typedef struct {
    bool active;
    u8 id;
    UnitClass type;
    Team team;
    u8 hp;              // 1..10
    u8 x, y;            // Grid coords (0..19, 0..14)
    bool has_moved;
    bool has_acted;
    u8 capture_progress;// 0..20 (20 = captured)
    bool emp_disabled;  // Disabled by Orbital EMP for 1 turn
} TacticalUnit;

// Combat Animation Sequence Data
typedef struct {
    bool active;
    u8 timer;
    u8 attacker_id;
    u8 defender_id;
    u8 damage_dealt;
    u8 counter_damage;
    bool has_counter;
    s16 projectile_x, projectile_y;
    s16 target_x, target_y;
    u8 phase;           // 0: fire, 1: flight, 2: impact/damage, 3: counter
} CombatAnim;

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
