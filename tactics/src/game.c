#include "game.h"
#include "graphics.h"
#include "input.h"
#include "audio.h"
#include "assets.h"
#include "save.h"

// =========================================================================
// Terrain Data & Movement Costs
// =========================================================================

// Defense cover percentage provided by terrain
static const u8 s_terrain_defense[9] = {
    10, // Plains: 10%
    20, // Forest: 20%
    40, // Mountain: 40%
     0, // River: 0%
     0, // Road: 0%
     0, // Bridge: 0%
    30, // City: 30%
    40, // HQ Player: 40%
    40  // HQ Enemy: 40%
};

// Movement costs: [UnitClass][TerrainType] (99 = Impassable)
static const u8 s_move_costs[5][9] = {
    // Walker (Mech legs)
    { 1, 1, 2, 2, 1, 1, 1, 1, 1 },
    // Recon (Wheeled buggy)
    { 2, 3, 99, 99, 1, 1, 1, 1, 1 },
    // Tank (Hover tank - glides over rivers!)
    { 1, 2, 99,  1, 1, 1, 1, 1, 1 },
    // Artillery (Tracked siege)
    { 1, 2, 99, 99, 1, 1, 1, 1, 1 },
    // VTOL (Airborne gunship - flies over everything!)
    { 1, 1,  1,  1, 1, 1, 1, 1, 1 }
};

// Base Movement points per unit
static const u8 s_unit_base_move[5] = {
    4, // Walker
    6, // Recon
    5, // Tank
    3, // Artillery
    6  // VTOL
};

// Base Damage Matrix: [Attacker][Defender] (Scale: 0..100)
static const u8 s_base_damage[5][5] = {
    //           Walker  Recon   Tank   Artillery  VTOL
    /* Walker */ { 55,    65,     40,      50,      30 },
    /* Recon  */ { 70,    55,     20,      45,      15 },
    /* Tank   */ { 75,    85,     55,      70,      35 },
    /* Artill */ { 85,    90,     75,      75,       0 }, // Can't hit air
    /* VTOL   */ { 60,    70,     85,      80,      50 }
};

// =========================================================================
// 3 Handcrafted Campaign Maps (20 cols x 15 rows)
// =========================================================================

// Mission 1: River Bridgehead (Central river with bridges, forests, and city capture)
static const u8 s_mission1_map[GRID_HEIGHT][GRID_WIDTH] = {
    { 0, 0, 1, 1, 0, 0, 0, 3, 0, 0, 0, 0, 0, 3, 0, 0, 1, 1, 0, 8 },
    { 0, 7, 0, 1, 0, 0, 0, 3, 0, 0, 1, 1, 0, 3, 0, 0, 0, 1, 0, 0 },
    { 0, 0, 0, 0, 4, 4, 4, 5, 4, 4, 4, 4, 4, 5, 4, 4, 0, 0, 0, 0 },
    { 0, 0, 1, 0, 4, 0, 0, 3, 0, 6, 0, 0, 0, 3, 0, 4, 0, 1, 0, 0 },
    { 1, 1, 1, 0, 4, 0, 0, 3, 0, 0, 0, 0, 0, 3, 0, 4, 0, 1, 1, 1 },
    { 0, 0, 0, 0, 4, 0, 0, 3, 0, 0, 1, 1, 0, 3, 0, 4, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 4, 4, 4, 5, 4, 4, 4, 4, 4, 5, 4, 4, 0, 0, 0, 0 },
    { 0, 2, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 3, 0, 0, 0, 2, 2, 0 },
    { 2, 2, 2, 2, 0, 0, 0, 3, 0, 6, 0, 0, 0, 3, 0, 0, 2, 2, 2, 2 },
    { 0, 2, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 3, 0, 0, 0, 2, 2, 0 },
    { 0, 0, 0, 0, 4, 4, 4, 5, 4, 4, 4, 4, 4, 5, 4, 4, 0, 0, 0, 0 },
    { 1, 1, 0, 0, 4, 0, 0, 3, 0, 0, 0, 0, 0, 3, 0, 4, 0, 0, 1, 1 },
    { 0, 0, 0, 0, 4, 0, 0, 3, 0, 0, 1, 1, 0, 3, 0, 4, 0, 0, 0, 0 },
    { 0, 0, 1, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 3, 0, 0, 0, 1, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0 }
};

// Mission 2: Canyon Artillery Ambush (Mountain pass chokepoints & artillery bastions)
static const u8 s_mission2_map[GRID_HEIGHT][GRID_WIDTH] = {
    { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 },
    { 2, 7, 0, 0, 4, 4, 4, 0, 2, 2, 0, 4, 4, 4, 0, 0, 0, 6, 8, 2 },
    { 2, 0, 1, 0, 4, 2, 2, 0, 2, 2, 0, 2, 2, 4, 0, 1, 0, 0, 0, 2 },
    { 2, 0, 1, 0, 4, 2, 2, 0, 0, 0, 0, 2, 2, 4, 0, 1, 0, 0, 0, 2 },
    { 2, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 2 },
    { 2, 2, 2, 0, 2, 2, 2, 2, 0, 6, 0, 2, 2, 2, 2, 0, 2, 2, 2, 2 },
    { 2, 2, 2, 0, 2, 2, 2, 2, 0, 0, 0, 2, 2, 2, 2, 0, 2, 2, 2, 2 },
    { 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2 },
    { 2, 2, 2, 0, 2, 2, 2, 2, 0, 0, 0, 2, 2, 2, 2, 0, 2, 2, 2, 2 },
    { 2, 2, 2, 0, 2, 2, 2, 2, 0, 6, 0, 2, 2, 2, 2, 0, 2, 2, 2, 2 },
    { 2, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 2 },
    { 2, 0, 1, 0, 4, 2, 2, 0, 0, 0, 0, 2, 2, 4, 0, 1, 0, 0, 0, 2 },
    { 2, 0, 1, 0, 4, 2, 2, 0, 2, 2, 0, 2, 2, 4, 0, 1, 0, 0, 0, 2 },
    { 2, 0, 0, 0, 4, 4, 4, 0, 2, 2, 0, 4, 4, 4, 0, 0, 0, 0, 0, 2 },
    { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 }
};

// Mission 3: The Iron Citadel (General Vex's cyber fortress with city networks)
static const u8 s_mission3_map[GRID_HEIGHT][GRID_WIDTH] = {
    { 0, 0, 1, 0, 4, 0, 0, 2, 2, 2, 2, 2, 2, 0, 0, 4, 0, 1, 0, 0 },
    { 0, 7, 1, 0, 4, 0, 0, 2, 6, 8, 6, 2, 2, 0, 0, 4, 0, 1, 0, 0 },
    { 0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0 },
    { 1, 1, 0, 0, 4, 6, 0, 2, 6, 6, 6, 2, 0, 6, 4, 4, 0, 0, 1, 1 },
    { 0, 0, 0, 0, 4, 0, 0, 2, 2, 2, 2, 2, 0, 0, 4, 0, 0, 0, 0, 0 },
    { 3, 3, 3, 5, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 5, 3, 3, 3, 3 },
    { 0, 0, 0, 0, 4, 0, 0, 1, 1, 1, 1, 1, 0, 0, 4, 0, 0, 0, 0, 0 },
    { 0, 6, 0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 6, 0, 0 },
    { 0, 0, 0, 0, 4, 0, 0, 1, 1, 1, 1, 1, 0, 0, 4, 0, 0, 0, 0, 0 },
    { 3, 3, 3, 5, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 5, 3, 3, 3, 3 },
    { 0, 0, 0, 0, 4, 0, 0, 2, 2, 2, 2, 2, 0, 0, 4, 0, 0, 0, 0, 0 },
    { 1, 1, 0, 0, 4, 6, 0, 2, 6, 6, 6, 2, 0, 6, 4, 4, 0, 0, 1, 1 },
    { 0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0 },
    { 0, 0, 1, 0, 4, 0, 0, 2, 6, 6, 6, 2, 2, 0, 0, 4, 0, 1, 0, 0 },
    { 0, 0, 1, 0, 4, 0, 0, 2, 2, 2, 2, 2, 2, 0, 0, 4, 0, 1, 0, 0 }
};

// =========================================================================
// Game Engine Variables
// =========================================================================

static GameState s_state = STATE_TITLE;
static SubState  s_sub_state = SUB_IDLE;
static TurnPhase s_turn_phase = PHASE_PLAYER;

static u8 s_current_mission = 1;
static u8 s_max_mission = 1;
static u16 s_turn_counter = 1;
static u16 s_state_timer = 0;

static u8 s_cursor_x = 1;
static u8 s_cursor_y = 1;

static u8 s_map[GRID_HEIGHT][GRID_WIDTH];
static u8 s_reach[GRID_HEIGHT][GRID_WIDTH]; // 0=none, 1=move, 2=attack, 3=emp

static TacticalUnit s_units[MAX_UNITS];
static s8 s_selected_unit_idx = -1;
static u8 s_action_menu_sel = 0; // 0=ATTACK, 1=CAPTURE, 2=WAIT
static bool s_can_attack = false;
static bool s_can_capture = false;

static u8 s_co_meter_player = 0; // 0..4
static u8 s_co_meter_enemy = 0;
static bool s_overdrive_active = false;

static CombatAnim s_combat;

// Forward declarations
static void load_mission(u8 mission);
static void calculate_reach(u8 unit_idx);
static void execute_combat(u8 attacker_idx, u8 defender_idx);
static void update_ai_turn(void);
static void check_victory_conditions(void);
static void update_camera(void);

// =========================================================================
// Initialization
// =========================================================================

void game_init(void) {
    save_init();
    s_max_mission = save_get_max_mission();
    s_current_mission = 1;
    s_state = STATE_TITLE;
    s_sub_state = SUB_IDLE;
    s_state_timer = 0;

    audio_play_bgm(BGM_TITLE);
}

static void load_mission(u8 mission) {
    s_current_mission = mission;
    s_turn_counter = 1;
    s_turn_phase = PHASE_PLAYER;
    s_state = STATE_BATTLE;
    s_sub_state = SUB_IDLE;
    s_selected_unit_idx = -1;
    s_co_meter_player = 0;
    s_co_meter_enemy = 0;
    s_overdrive_active = false;
    s_combat.active = false;

    // 1. Copy Map Data
    const u8 (*src_map)[GRID_WIDTH] = s_mission1_map;
    if (mission == 2) src_map = s_mission2_map;
    else if (mission == 3) src_map = s_mission3_map;

    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            s_map[y][x] = src_map[y][x];
            s_reach[y][x] = 0;
        }
    }

    // 2. Clear Units
    for (int i = 0; i < MAX_UNITS; i++) {
        s_units[i].active = false;
    }

    // 3. Deploy Mission Units
    if (mission == 1) {
        // Player Forces (West Bridgehead)
        // Unit 0: Assault Walker
        s_units[0].active = true; s_units[0].id = 0; s_units[0].type = UNIT_WALKER; s_units[0].team = TEAM_PLAYER; s_units[0].hp = 10; s_units[0].x = 1; s_units[0].y = 2;
        // Unit 1: Recon Drone
        s_units[1].active = true; s_units[1].id = 1; s_units[1].type = UNIT_RECON;  s_units[1].team = TEAM_PLAYER; s_units[1].hp = 10; s_units[1].x = 2; s_units[1].y = 1;
        // Unit 2: Hover Tank
        s_units[2].active = true; s_units[2].id = 2; s_units[2].type = UNIT_TANK;   s_units[2].team = TEAM_PLAYER; s_units[2].hp = 10; s_units[2].x = 1; s_units[2].y = 3;
        // Unit 3: Walker 2
        s_units[3].active = true; s_units[3].id = 3; s_units[3].type = UNIT_WALKER; s_units[3].team = TEAM_PLAYER; s_units[3].hp = 10; s_units[3].x = 0; s_units[3].y = 2;

        // Enemy Forces (East Bridgehead)
        // Unit 4: Enemy Walker
        s_units[4].active = true; s_units[4].id = 4; s_units[4].type = UNIT_WALKER; s_units[4].team = TEAM_ENEMY;  s_units[4].hp = 10; s_units[4].x = 18; s_units[4].y = 2;
        // Unit 5: Enemy Recon
        s_units[5].active = true; s_units[5].id = 5; s_units[5].type = UNIT_RECON;  s_units[5].team = TEAM_ENEMY;  s_units[5].hp = 10; s_units[5].x = 17; s_units[5].y = 1;
        // Unit 6: Enemy Tank
        s_units[6].active = true; s_units[6].id = 6; s_units[6].type = UNIT_TANK;   s_units[6].team = TEAM_ENEMY;  s_units[6].hp = 10; s_units[6].x = 18; s_units[6].y = 3;
        // Unit 7: Enemy Walker 2
        s_units[7].active = true; s_units[7].id = 7; s_units[7].type = UNIT_WALKER; s_units[7].team = TEAM_ENEMY;  s_units[7].hp = 10; s_units[7].x = 19; s_units[7].y = 2;
    } else if (mission == 2) {
        // Mission 2: Canyon Artillery Ambush
        // Player Units (West)
        s_units[0].active = true; s_units[0].id = 0; s_units[0].type = UNIT_WALKER;    s_units[0].team = TEAM_PLAYER; s_units[0].hp = 10; s_units[0].x = 2; s_units[0].y = 1;
        s_units[1].active = true; s_units[1].id = 1; s_units[1].type = UNIT_TANK;      s_units[1].team = TEAM_PLAYER; s_units[1].hp = 10; s_units[1].x = 1; s_units[1].y = 3;
        s_units[2].active = true; s_units[2].id = 2; s_units[2].type = UNIT_ARTILLERY; s_units[2].team = TEAM_PLAYER; s_units[2].hp = 10; s_units[2].x = 1; s_units[2].y = 2;
        s_units[3].active = true; s_units[3].id = 3; s_units[3].type = UNIT_TANK;      s_units[3].team = TEAM_PLAYER; s_units[3].hp = 10; s_units[3].x = 2; s_units[3].y = 4;

        // Enemy Units (East Fortress)
        s_units[4].active = true; s_units[4].id = 4; s_units[4].type = UNIT_ARTILLERY; s_units[4].team = TEAM_ENEMY;  s_units[4].hp = 10; s_units[4].x = 17; s_units[4].y = 2;
        s_units[5].active = true; s_units[5].id = 5; s_units[5].type = UNIT_TANK;      s_units[5].team = TEAM_ENEMY;  s_units[5].hp = 10; s_units[5].x = 16; s_units[5].y = 1;
        s_units[6].active = true; s_units[6].id = 6; s_units[6].type = UNIT_TANK;      s_units[6].team = TEAM_ENEMY;  s_units[6].hp = 10; s_units[6].x = 16; s_units[6].y = 3;
        s_units[7].active = true; s_units[7].id = 7; s_units[7].type = UNIT_WALKER;    s_units[7].team = TEAM_ENEMY;  s_units[7].hp = 10; s_units[7].x = 18; s_units[7].y = 1;
    } else {
        // Mission 3: The Iron Citadel
        // Player Combined Arms
        s_units[0].active = true; s_units[0].id = 0; s_units[0].type = UNIT_WALKER;    s_units[0].team = TEAM_PLAYER; s_units[0].hp = 10; s_units[0].x = 1; s_units[0].y = 1;
        s_units[1].active = true; s_units[1].id = 1; s_units[1].type = UNIT_TANK;      s_units[1].team = TEAM_PLAYER; s_units[1].hp = 10; s_units[1].x = 1; s_units[1].y = 2;
        s_units[2].active = true; s_units[2].id = 2; s_units[2].type = UNIT_ARTILLERY; s_units[2].team = TEAM_PLAYER; s_units[2].hp = 10; s_units[2].x = 0; s_units[2].y = 1;
        s_units[3].active = true; s_units[3].id = 3; s_units[3].type = UNIT_VTOL;      s_units[3].team = TEAM_PLAYER; s_units[3].hp = 10; s_units[3].x = 2; s_units[3].y = 1;
        s_units[4].active = true; s_units[4].id = 4; s_units[4].type = UNIT_RECON;     s_units[4].team = TEAM_PLAYER; s_units[4].hp = 10; s_units[4].x = 1; s_units[4].y = 3;

        // Enemy Citadel Forces
        s_units[5].active = true; s_units[5].id = 5; s_units[5].type = UNIT_TANK;      s_units[5].team = TEAM_ENEMY;  s_units[5].hp = 10; s_units[5].x = 10; s_units[5].y = 2;
        s_units[6].active = true; s_units[6].id = 6; s_units[6].type = UNIT_ARTILLERY; s_units[6].team = TEAM_ENEMY;  s_units[6].hp = 10; s_units[6].x = 9;  s_units[6].y = 1;
        s_units[7].active = true; s_units[7].id = 7; s_units[7].type = UNIT_VTOL;      s_units[7].team = TEAM_ENEMY;  s_units[7].hp = 10; s_units[7].x = 11; s_units[7].y = 1;
        s_units[8].active = true; s_units[8].id = 8; s_units[8].type = UNIT_WALKER;    s_units[8].team = TEAM_ENEMY;  s_units[8].hp = 10; s_units[8].x = 8;  s_units[8].y = 2;
    }

    for (int i = 0; i < MAX_UNITS; i++) {
        if (s_units[i].active) {
            s_units[i].has_moved = false;
            s_units[i].has_acted = false;
            s_units[i].capture_progress = 0;
            s_units[i].emp_disabled = false;
        }
    }

    // Set cursor to player unit 0
    s_cursor_x = s_units[0].x;
    s_cursor_y = s_units[0].y;
    update_camera();

    // 4. Render Terrain to BG2 (Each 16x16 macro block is 2x2 8x8 tiles)
    for (int gy = 0; gy < GRID_HEIGHT; gy++) {
        for (int gx = 0; gx < GRID_WIDTH; gx++) {
            u8 t = s_map[gy][gx];
            u16 base_tile = TILE_PLAINS_TL;
            if (t == TERR_FOREST)      base_tile = TILE_FOREST_TL;
            else if (t == TERR_MOUNTAIN) base_tile = TILE_MOUNTAIN_TL;
            else if (t == TERR_RIVER)    base_tile = TILE_RIVER_TL;
            else if (t == TERR_ROAD || t == TERR_BRIDGE) base_tile = TILE_ROAD_TL;
            else if (t == TERR_CITY)     base_tile = TILE_CITY_TL;
            else if (t == TERR_HQ_PLAYER) base_tile = TILE_HQ_P_TL;
            else if (t == TERR_HQ_ENEMY)  base_tile = TILE_HQ_E_TL;

            u8 bx = gx * 2;
            u8 by = gy * 2;
            graphics_set_bg2_tile(bx,     by,     base_tile + 0, 2);
            graphics_set_bg2_tile(bx + 1, by,     base_tile + 1, 2);
            graphics_set_bg2_tile(bx,     by + 1, base_tile + 2, 2);
            graphics_set_bg2_tile(bx + 1, by + 1, base_tile + 3, 2);
        }
    }

    graphics_clear_bg0();
    graphics_clear_bg1();
    audio_play_bgm(BGM_PLAYER_PHASE);
}

// =========================================================================
// Camera Auto-Tracking
// =========================================================================

static void update_camera(void) {
    s16 cursor_px = s_cursor_x * 16;
    s16 cursor_py = s_cursor_y * 16;

    s16 cur_cam_x = graphics_get_camera_x();
    s16 cur_cam_y = graphics_get_camera_y();

    // Keep cursor within 3-tile margin of screen borders
    if (cursor_px - cur_cam_x > 180) cur_cam_x = cursor_px - 180;
    if (cursor_px - cur_cam_x < 48)  cur_cam_x = cursor_px - 48;
    if (cursor_py - cur_cam_y > 112) cur_cam_y = cursor_py - 112;
    if (cursor_py - cur_cam_y < 32)  cur_cam_y = cursor_py - 32;

    graphics_set_camera(cur_cam_x, cur_cam_y);
}

// =========================================================================
// Reach Calculation (BFS Flood Fill)
// =========================================================================

static void calculate_reach(u8 unit_idx) {
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            s_reach[y][x] = 0;
        }
    }

    TacticalUnit* u = &s_units[unit_idx];
    u8 move_pts = s_unit_base_move[u->type];
    if (s_overdrive_active && u->team == TEAM_PLAYER) move_pts += 2;

    // Distance matrix initialized to 255
    u8 dist[GRID_HEIGHT][GRID_WIDTH];
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            dist[y][x] = 255;
        }
    }

    // BFS Queue
    u8 qx[64], qy[64];
    u8 qhead = 0, qtail = 0;

    dist[u->y][u->x] = 0;
    qx[qtail] = u->x;
    qy[qtail] = u->y;
    qtail++;

    const s8 dx[4] = { 1, -1, 0, 0 };
    const s8 dy[4] = { 0, 0, 1, -1 };

    while (qhead != qtail) {
        u8 cx = qx[qhead];
        u8 cy = qy[qhead];
        qhead = (qhead + 1) & 63;
        u8 cur_d = dist[cy][cx];

        for (int d = 0; d < 4; d++) {
            s16 nx = cx + dx[d];
            s16 ny = cy + dy[d];

            if (nx >= 0 && nx < GRID_WIDTH && ny >= 0 && ny < GRID_HEIGHT) {
                u8 terr = s_map[ny][nx];
                u8 cost = s_move_costs[u->type][terr];

                // Blocked by enemy units
                bool enemy_blocked = false;
                for (int i = 0; i < MAX_UNITS; i++) {
                    if (s_units[i].active && s_units[i].team != u->team && s_units[i].x == nx && s_units[i].y == ny) {
                        enemy_blocked = true;
                        break;
                    }
                }

                if (!enemy_blocked && cost < 99) {
                    if (cur_d + cost <= move_pts && cur_d + cost < dist[ny][nx]) {
                        dist[ny][nx] = cur_d + cost;
                        qx[qtail] = (u8)nx;
                        qy[qtail] = (u8)ny;
                        qtail = (qtail + 1) & 63;
                    }
                }
            }
        }
    }

    // Populate Movement Reach on BG1
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            if (dist[y][x] <= move_pts) {
                // Check if tile already occupied by friendly unit
                bool occupied = false;
                for (int i = 0; i < MAX_UNITS; i++) {
                    if (s_units[i].active && i != unit_idx && s_units[i].x == x && s_units[i].y == y) {
                        occupied = true;
                        break;
                    }
                }
                if (!occupied || (x == u->x && y == u->y)) {
                    s_reach[y][x] = 1; // Reachable movement tile
                }
            }
        }
    }

    // Render BG1 Movement Highlight Grid
    for (int gy = 0; gy < GRID_HEIGHT; gy++) {
        for (int gx = 0; gx < GRID_WIDTH; gx++) {
            u16 tile = (s_reach[gy][gx] == 1) ? TILE_MOVE_GRID : TILE_EMPTY;
            u8 bx = gx * 2;
            u8 by = gy * 2;
            graphics_set_bg1_tile(bx,     by,     tile, 1);
            graphics_set_bg1_tile(bx + 1, by,     tile, 1);
            graphics_set_bg1_tile(bx,     by + 1, tile, 1);
            graphics_set_bg1_tile(bx + 1, by + 1, tile, 1);
        }
    }
}

// =========================================================================
// Combat & Damage Resolution
// =========================================================================

static void execute_combat(u8 attacker_idx, u8 defender_idx) {
    TacticalUnit* atk = &s_units[attacker_idx];
    TacticalUnit* def = &s_units[defender_idx];

    // Base damage lookup
    u8 base_atk = s_base_damage[atk->type][def->type];
    u8 def_cover = s_terrain_defense[s_map[def->y][def->x]];

    // Attacker damage calculation
    u32 dmg = (base_atk * atk->hp * (100 - def_cover)) / 1000;
    if (dmg < 1 && base_atk > 0) dmg = 1;
    if (s_overdrive_active && atk->team == TEAM_PLAYER) dmg = (dmg * 13) / 10; // +30%

    // Counter-attack damage calculation (if defender survives and direct Range 1)
    s16 dist = (atk->x > def->x ? atk->x - def->x : def->x - atk->x) +
               (atk->y > def->y ? atk->y - def->y : def->y - atk->y);

    bool has_counter = (dist == 1 && def->hp > dmg && !def->emp_disabled && def->type != UNIT_ARTILLERY);
    u8 counter_dmg = 0;

    if (has_counter) {
        u8 rem_hp = def->hp - dmg;
        u8 base_def = s_base_damage[def->type][atk->type];
        u8 atk_cover = s_terrain_defense[s_map[atk->y][atk->x]];
        counter_dmg = (base_def * rem_hp * (100 - atk_cover)) / 1000;
        if (counter_dmg < 1 && base_def > 0) counter_dmg = 1;
    }

    // Set Combat Animation
    s_combat.active = true;
    s_combat.timer = 0;
    s_combat.attacker_id = attacker_idx;
    s_combat.defender_id = defender_idx;
    s_combat.damage_dealt = (u8)dmg;
    s_combat.counter_damage = counter_dmg;
    s_combat.has_counter = has_counter;
    s_combat.phase = 0;

    s_sub_state = SUB_COMBAT_ANIM;

    if (atk->type == UNIT_TANK) sfx_cannon_fire();
    else if (atk->type == UNIT_ARTILLERY) sfx_rocket_launch();
    else sfx_autocannon_burst();

    graphics_trigger_shake(3, 10);
}

// =========================================================================
// Enemy AI
// =========================================================================

static void update_ai_turn(void) {
    bool all_done = true;

    for (int i = 0; i < MAX_UNITS; i++) {
        if (s_units[i].active && s_units[i].team == TEAM_ENEMY && !s_units[i].has_acted) {
            all_done = false;
            TacticalUnit* u = &s_units[i];

            if (u->emp_disabled) {
                u->emp_disabled = false;
                u->has_acted = true;
                continue;
            }

            // 1. Check if can attack any adjacent player unit immediately
            int best_target = -1;
            u8 min_target_hp = 255;

            for (int p = 0; p < MAX_UNITS; p++) {
                if (s_units[p].active && s_units[p].team == TEAM_PLAYER) {
                    s16 dist = (u->x > s_units[p].x ? u->x - s_units[p].x : s_units[p].x - u->x) +
                               (u->y > s_units[p].y ? u->y - s_units[p].y : s_units[p].y - u->y);

                    if (u->type == UNIT_ARTILLERY) {
                        if (dist >= 2 && dist <= 3 && s_units[p].hp < min_target_hp) {
                            min_target_hp = s_units[p].hp;
                            best_target = p;
                        }
                    } else {
                        if (dist == 1 && s_units[p].hp < min_target_hp) {
                            min_target_hp = s_units[p].hp;
                            best_target = p;
                        }
                    }
                }
            }

            if (best_target != -1) {
                u->has_acted = true;
                execute_combat(i, best_target);
                return;
            }

            // 2. If no direct attack, move towards nearest Player unit
            int target_player = -1;
            s16 min_dist = 999;
            for (int p = 0; p < MAX_UNITS; p++) {
                if (s_units[p].active && s_units[p].team == TEAM_PLAYER) {
                    s16 d = (u->x > s_units[p].x ? u->x - s_units[p].x : s_units[p].x - u->x) +
                            (u->y > s_units[p].y ? u->y - s_units[p].y : s_units[p].y - u->y);
                    if (d < min_dist) {
                        min_dist = d;
                        target_player = p;
                    }
                }
            }

            if (target_player != -1) {
                TacticalUnit* tgt = &s_units[target_player];
                // Step towards target along best axis
                s8 step_x = 0, step_y = 0;
                if (tgt->x > u->x) step_x = 1;
                else if (tgt->x < u->x) step_x = -1;
                else if (tgt->y > u->y) step_y = 1;
                else if (tgt->y < u->y) step_y = -1;

                u8 nx = u->x + step_x;
                u8 ny = u->y + step_y;

                if (nx < GRID_WIDTH && ny < GRID_HEIGHT && s_move_costs[u->type][s_map[ny][nx]] < 99) {
                    bool occupied = false;
                    for (int o = 0; o < MAX_UNITS; o++) {
                        if (s_units[o].active && s_units[o].x == nx && s_units[o].y == ny) {
                            occupied = true;
                            break;
                        }
                    }
                    if (!occupied) {
                        u->x = nx;
                        u->y = ny;
                    }
                }
            }

            u->has_acted = true;
            return;
        }
    }

    if (all_done) {
        // End Enemy Turn -> Switch to Player Turn
        s_turn_phase = PHASE_PLAYER;
        s_turn_counter++;
        s_overdrive_active = false;

        for (int i = 0; i < MAX_UNITS; i++) {
            if (s_units[i].active) {
                s_units[i].has_moved = false;
                s_units[i].has_acted = false;
            }
        }

        audio_play_bgm(BGM_PLAYER_PHASE);
    }
}

// =========================================================================
// Victory & Defeat Check
// =========================================================================

static void check_victory_conditions(void) {
    bool any_player_alive = false;
    bool any_enemy_alive = false;

    for (int i = 0; i < MAX_UNITS; i++) {
        if (s_units[i].active) {
            if (s_units[i].team == TEAM_PLAYER) any_player_alive = true;
            if (s_units[i].team == TEAM_ENEMY)  any_enemy_alive = true;
        }
    }

    // Check HQ Captures
    for (int i = 0; i < MAX_UNITS; i++) {
        if (s_units[i].active && s_units[i].type == UNIT_WALKER) {
            if (s_units[i].team == TEAM_PLAYER && s_map[s_units[i].y][s_units[i].x] == TERR_HQ_ENEMY && s_units[i].capture_progress >= 20) {
                any_enemy_alive = false; // HQ captured!
            }
            if (s_units[i].team == TEAM_ENEMY && s_map[s_units[i].y][s_units[i].x] == TERR_HQ_PLAYER && s_units[i].capture_progress >= 20) {
                any_player_alive = false; // Player HQ lost!
            }
        }
    }

    if (!any_enemy_alive) {
        s_state = STATE_MISSION_CLEAR;
        s_state_timer = 0;
        audio_play_bgm(BGM_VICTORY);

        // Calculate Rank based on turns
        u8 rank = 2; // B rank
        if (s_turn_counter <= 10) rank = 4; // S rank
        else if (s_turn_counter <= 15) rank = 3; // A rank

        save_set_mission_rank(s_current_mission, rank);
        save_add_career_score(10000 - s_turn_counter * 200);

        if (s_current_mission < 3) {
            if (s_current_mission + 1 > s_max_mission) {
                s_max_mission = s_current_mission + 1;
                save_set_max_mission(s_max_mission);
            }
        } else {
            s_state = STATE_VICTORY;
        }
    } else if (!any_player_alive) {
        s_state = STATE_GAME_OVER;
        s_state_timer = 0;
    }
}

// =========================================================================
// Game Update Loop
// =========================================================================

void game_update(void) {
    audio_update();
    graphics_update_shake();

    // -------------------------------------------------------------
    // Title Screen
    // -------------------------------------------------------------
    if (s_state == STATE_TITLE) {
        s_state_timer++;

        if (key_was_pressed(KEY_LEFT)) {
            if (s_current_mission > 1) {
                s_current_mission--;
                sfx_cursor_move();
            }
        }
        if (key_was_pressed(KEY_RIGHT)) {
            if (s_current_mission < s_max_mission) {
                s_current_mission++;
                sfx_cursor_move();
            }
        }

        if (key_was_pressed(KEY_START) || key_was_pressed(KEY_A)) {
            sfx_select();
            load_mission(s_current_mission);
        }
        return;
    }

    // -------------------------------------------------------------
    // Mission Clear / Game Over / Victory Screens
    // -------------------------------------------------------------
    if (s_state == STATE_MISSION_CLEAR) {
        s_state_timer++;
        if (s_state_timer >= 120 && (key_was_pressed(KEY_A) || key_was_pressed(KEY_START))) {
            load_mission(s_current_mission + 1);
        }
        return;
    }

    if (s_state == STATE_GAME_OVER || s_state == STATE_VICTORY) {
        s_state_timer++;
        if (s_state_timer >= 120 && (key_was_pressed(KEY_A) || key_was_pressed(KEY_START))) {
            s_state = STATE_TITLE;
            audio_play_bgm(BGM_TITLE);
        }
        return;
    }

    // -------------------------------------------------------------
    // Combat Animation Sequence
    // -------------------------------------------------------------
    if (s_sub_state == SUB_COMBAT_ANIM) {
        s_combat.timer++;

        if (s_combat.phase == 0 && s_combat.timer >= 20) {
            // Apply damage to defender
            TacticalUnit* def = &s_units[s_combat.defender_id];
            if (def->hp <= s_combat.damage_dealt) {
                def->hp = 0;
                def->active = false;
                sfx_explosion();
                graphics_trigger_shake(5, 12);
            } else {
                def->hp -= s_combat.damage_dealt;
            }

            // Charge CO Meters
            if (s_co_meter_player < 4) s_co_meter_player++;

            if (s_combat.has_counter && def->active) {
                s_combat.phase = 1;
                s_combat.timer = 0;
                sfx_cannon_fire();
            } else {
                s_combat.active = false;
                s_sub_state = SUB_IDLE;
                s_selected_unit_idx = -1;
                graphics_clear_bg0();
                graphics_clear_bg1();
                check_victory_conditions();
            }
        } else if (s_combat.phase == 1 && s_combat.timer >= 20) {
            // Apply counter damage to attacker
            TacticalUnit* atk = &s_units[s_combat.attacker_id];
            if (atk->hp <= s_combat.counter_damage) {
                atk->hp = 0;
                atk->active = false;
                sfx_explosion();
                graphics_trigger_shake(5, 12);
            } else {
                atk->hp -= s_combat.counter_damage;
            }

            s_combat.active = false;
            s_sub_state = SUB_IDLE;
            s_selected_unit_idx = -1;
            graphics_clear_bg0();
            graphics_clear_bg1();
            check_victory_conditions();
        }
        return;
    }

    // -------------------------------------------------------------
    // Enemy Phase Processing
    // -------------------------------------------------------------
    if (s_turn_phase == PHASE_ENEMY) {
        s_state_timer++;
        if (s_state_timer % 30 == 0) {
            update_ai_turn();
        }
        return;
    }

    // -------------------------------------------------------------
    // Player Turn Interactive Input
    // -------------------------------------------------------------

    // Cursor Movement
    bool moved_cursor = false;
    if (key_was_pressed(KEY_LEFT) && s_cursor_x > 0) {
        s_cursor_x--; moved_cursor = true;
    }
    if (key_was_pressed(KEY_RIGHT) && s_cursor_x < GRID_WIDTH - 1) {
        s_cursor_x++; moved_cursor = true;
    }
    if (key_was_pressed(KEY_UP) && s_cursor_y > 0) {
        s_cursor_y--; moved_cursor = true;
    }
    if (key_was_pressed(KEY_DOWN) && s_cursor_y < GRID_HEIGHT - 1) {
        s_cursor_y++; moved_cursor = true;
    }

    if (moved_cursor) {
        sfx_cursor_move();
        update_camera();
    }

    // Sub-State Handling
    if (s_sub_state == SUB_IDLE) {
        if (key_was_pressed(KEY_A)) {
            // Check if cursor is on player unit
            int clicked_unit = -1;
            for (int i = 0; i < MAX_UNITS; i++) {
                if (s_units[i].active && s_units[i].team == TEAM_PLAYER && s_units[i].x == s_cursor_x && s_units[i].y == s_cursor_y) {
                    clicked_unit = i;
                    break;
                }
            }

            if (clicked_unit != -1 && !s_units[clicked_unit].has_acted) {
                s_selected_unit_idx = (s8)clicked_unit;
                s_sub_state = SUB_UNIT_SELECTED;
                sfx_select();
                calculate_reach(clicked_unit);
            } else {
                // Open Field Options Menu on empty tile
                s_sub_state = SUB_FIELD_MENU;
                sfx_select();
            }
        }
    } else if (s_sub_state == SUB_UNIT_SELECTED) {
        if (key_was_pressed(KEY_B)) {
            // Cancel selection
            s_selected_unit_idx = -1;
            s_sub_state = SUB_IDLE;
            graphics_clear_bg1();
            sfx_cancel();
        } else if (key_was_pressed(KEY_A)) {
            // Confirm Move if destination is reachable
            if (s_reach[s_cursor_y][s_cursor_x] == 1) {
                TacticalUnit* u = &s_units[s_selected_unit_idx];
                u->x = s_cursor_x;
                u->y = s_cursor_y;
                u->has_moved = true;
                sfx_unit_move();
                graphics_clear_bg1();

                // Check for action choices (Attack, Capture, Wait)
                s_can_attack = false;
                for (int e = 0; e < MAX_UNITS; e++) {
                    if (s_units[e].active && s_units[e].team == TEAM_ENEMY) {
                        s16 dist = (u->x > s_units[e].x ? u->x - s_units[e].x : s_units[e].x - u->x) +
                                   (u->y > s_units[e].y ? u->y - s_units[e].y : s_units[e].y - u->y);
                        if (u->type == UNIT_ARTILLERY) {
                            if (dist >= 2 && dist <= 3) s_can_attack = true;
                        } else {
                            if (dist == 1) s_can_attack = true;
                        }
                    }
                }

                u8 terr = s_map[u->y][u->x];
                s_can_capture = (u->type == UNIT_WALKER && (terr == TERR_CITY || terr == TERR_HQ_ENEMY));

                s_sub_state = SUB_ACTION_MENU;
                s_action_menu_sel = s_can_attack ? 0 : 2; // Default to Attack if available, else Wait
            }
        }
    } else if (s_sub_state == SUB_ACTION_MENU) {
        if (key_was_pressed(KEY_UP)) {
            if (s_action_menu_sel > 0) s_action_menu_sel--;
            sfx_cursor_move();
        }
        if (key_was_pressed(KEY_DOWN)) {
            if (s_action_menu_sel < 2) s_action_menu_sel++;
            sfx_cursor_move();
        }

        if (key_was_pressed(KEY_B)) {
            // Cancel and re-select move
            s_sub_state = SUB_UNIT_SELECTED;
            graphics_clear_bg0();
            calculate_reach(s_selected_unit_idx);
            sfx_cancel();
        } else if (key_was_pressed(KEY_A)) {
            TacticalUnit* u = &s_units[s_selected_unit_idx];

            if (s_action_menu_sel == 0 && s_can_attack) {
                // Target Select
                s_sub_state = SUB_TARGET_SELECT;
                sfx_select();
                graphics_clear_bg0();
                // Mark attackable enemies
                for (int e = 0; e < MAX_UNITS; e++) {
                    if (s_units[e].active && s_units[e].team == TEAM_ENEMY) {
                        s16 dist = (u->x > s_units[e].x ? u->x - s_units[e].x : s_units[e].x - u->x) +
                                   (u->y > s_units[e].y ? u->y - s_units[e].y : s_units[e].y - u->y);
                        if ((u->type == UNIT_ARTILLERY && dist >= 2 && dist <= 3) ||
                            (u->type != UNIT_ARTILLERY && dist == 1)) {
                            s_reach[s_units[e].y][s_units[e].x] = 2; // Target
                        }
                    }
                }
            } else if (s_action_menu_sel == 1 && s_can_capture) {
                // Capture Building
                u->capture_progress += 10;
                sfx_capture();
                u->has_acted = true;
                s_sub_state = SUB_IDLE;
                s_selected_unit_idx = -1;
                graphics_clear_bg0();
                check_victory_conditions();
            } else {
                // Wait
                u->has_acted = true;
                s_sub_state = SUB_IDLE;
                s_selected_unit_idx = -1;
                graphics_clear_bg0();
                sfx_select();
            }
        }
    } else if (s_sub_state == SUB_TARGET_SELECT) {
        if (key_was_pressed(KEY_B)) {
            s_sub_state = SUB_ACTION_MENU;
            graphics_clear_bg1();
            sfx_cancel();
        } else if (key_was_pressed(KEY_A)) {
            // Find targeted enemy
            for (int e = 0; e < MAX_UNITS; e++) {
                if (s_units[e].active && s_units[e].team == TEAM_ENEMY && s_units[e].x == s_cursor_x && s_units[e].y == s_cursor_y) {
                    if (s_reach[s_cursor_y][s_cursor_x] == 2) {
                        TacticalUnit* u = &s_units[s_selected_unit_idx];
                        u->has_acted = true;
                        execute_combat(s_selected_unit_idx, e);
                        break;
                    }
                }
            }
        }
    } else if (s_sub_state == SUB_FIELD_MENU) {
        if (key_was_pressed(KEY_B)) {
            s_sub_state = SUB_IDLE;
            graphics_clear_bg0();
            sfx_cancel();
        } else if (key_was_pressed(KEY_A)) {
            // Select End Turn or CO Power
            if (s_co_meter_player >= 4) {
                // Trigger CO Power: Overdrive Blitz
                s_overdrive_active = true;
                s_co_meter_player = 0;
                sfx_co_power();
                graphics_trigger_shake(5, 20);
                audio_play_bgm(BGM_CO_POWER);
                s_sub_state = SUB_IDLE;
                graphics_clear_bg0();
            } else {
                // End Turn
                s_turn_phase = PHASE_ENEMY;
                s_state_timer = 0;
                s_sub_state = SUB_IDLE;
                graphics_clear_bg0();
                audio_play_bgm(BGM_ENEMY_PHASE);
            }
        }
    }
}

// =========================================================================
// Render Subsystem
// =========================================================================

void game_render(void) {
    graphics_oam_hide_all();

    // -------------------------------------------------------------
    // Title Screen
    // -------------------------------------------------------------
    if (s_state == STATE_TITLE) {
        graphics_print_text(6, 3, "IRON PROTOCOL", 0);
        graphics_print_text(7, 5, "MICRO-TACTICS", 0);

        graphics_print_text(5, 8, "SELECT OPERATION:", 0);

        char mis_str[16];
        mis_str[0] = '<';
        mis_str[1] = ' ';
        mis_str[2] = 'M';
        mis_str[3] = 'I';
        mis_str[4] = 'S';
        mis_str[5] = 'S';
        mis_str[6] = 'I';
        mis_str[7] = 'O';
        mis_str[8] = 'N';
        mis_str[9] = ' ';
        mis_str[10] = '0' + s_current_mission;
        mis_str[11] = ' ';
        mis_str[12] = '>';
        mis_str[13] = '\0';
        graphics_print_text(8, 10, mis_str, 0);

        if (s_current_mission == 1) graphics_print_text(6, 12, "[RIVER BRIDGEHEAD]", 0);
        else if (s_current_mission == 2) graphics_print_text(5, 12, "[CANYON ARTILLERY]", 0);
        else graphics_print_text(6, 12, "[THE IRON CITADEL]", 0);

        if ((s_state_timer & 32) == 0) {
            graphics_print_text(8, 15, "PRESS START", 0);
        } else {
            graphics_print_text(8, 15, "           ", 0);
        }

        graphics_oam_copy();
        return;
    }

    // -------------------------------------------------------------
    // Render Units (Slots 0..15)
    // -------------------------------------------------------------
    s16 cam_x = graphics_get_camera_x();
    s16 cam_y = graphics_get_camera_y();

    for (int i = 0; i < MAX_UNITS; i++) {
        if (s_units[i].active) {
            s16 sx = (s_units[i].x * 16) - cam_x;
            s16 sy = (s_units[i].y * 16) - cam_y;

            u16 tile = SPRITE_TILE_WALKER;
            if (s_units[i].type == UNIT_RECON) tile = SPRITE_TILE_RECON;
            else if (s_units[i].type == UNIT_TANK) tile = SPRITE_TILE_TANK;
            else if (s_units[i].type == UNIT_ARTILLERY) tile = SPRITE_TILE_ARTILLERY;
            else if (s_units[i].type == UNIT_VTOL) tile = SPRITE_TILE_VTOL;

            u8 pal = (s_units[i].team == TEAM_PLAYER) ? 0 : 1;
            // Greyscale / dim if already acted
            if (s_units[i].has_acted) pal = 3;

            graphics_set_sprite(i, sx, sy, tile, 0, 1, pal, false, false); // 16x16

            // HP Badge in bottom-right corner of unit (Slots 16..31)
            if (s_units[i].hp < 10) {
                u16 hp_tile = SPRITE_TILE_HP_BADGE + s_units[i].hp;
                graphics_set_sprite(16 + i, sx + 8, sy + 8, hp_tile, 0, 0, 3, false, false);
            }
        }
    }

    // -------------------------------------------------------------
    // Selection Cursor (Slot 32)
    // -------------------------------------------------------------
    if (s_state == STATE_BATTLE) {
        s16 csx = (s_cursor_x * 16) - cam_x;
        s16 csy = (s_cursor_y * 16) - cam_y;
        graphics_set_sprite(32, csx, csy, SPRITE_TILE_CURSOR, 0, 1, 2, false, false);
    }

    // -------------------------------------------------------------
    // Combat Animation VFX (Slot 33)
    // -------------------------------------------------------------
    if (s_combat.active) {
        TacticalUnit* def = &s_units[s_combat.defender_id];
        s16 dsx = (def->x * 16) - cam_x;
        s16 dsy = (def->y * 16) - cam_y;
        graphics_set_sprite(33, dsx, dsy, SPRITE_TILE_EXPLOSION, 0, 1, 3, false, false);
    }

    // -------------------------------------------------------------
    // HUD Overlays on BG0
    // -------------------------------------------------------------
    if (s_state == STATE_BATTLE) {
        // Top status bar
        graphics_print_text(1, 0, "TURN:", 0);
        graphics_print_num(6, 0, s_turn_counter, 2, 0);

        if (s_turn_phase == PHASE_PLAYER) {
            graphics_print_text(10, 0, "[PLAYER PHASE]", 0);
        } else {
            graphics_print_text(10, 0, "[ENEMY PHASE ]", 0);
        }

        // CO Power Meter Stars (Top Right)
        char co_str[8];
        co_str[0] = 'C'; co_str[1] = 'O'; co_str[2] = ':';
        for (int s = 0; s < 4; s++) {
            co_str[3 + s] = (s < s_co_meter_player) ? '!' : '-';
        }
        co_str[7] = '\0';
        graphics_print_text(22, 0, co_str, 0);

        // Action Menu Dialog
        if (s_sub_state == SUB_ACTION_MENU) {
            // Draw window box
            for (u8 y = 4; y <= 8; y++) {
                for (u8 x = 18; x <= 28; x++) {
                    u16 tile = TILE_BOX_FILL;
                    if (y == 4 && x == 18) tile = TILE_BOX_TL;
                    else if (y == 4 && x == 28) tile = TILE_BOX_TR;
                    else if (y == 8 && x == 18) tile = TILE_BOX_BL;
                    else if (y == 8 && x == 28) tile = TILE_BOX_BR;
                    else if (y == 4 || y == 8) tile = TILE_BOX_H;
                    else if (x == 18 || x == 28) tile = TILE_BOX_V;
                    graphics_set_bg0_tile(x, y, tile, 0);
                }
            }

            graphics_print_text(20, 5, (s_action_menu_sel == 0 ? ">ATTACK" : " ATTACK"), 0);
            graphics_print_text(20, 6, (s_action_menu_sel == 1 ? ">CAPTURE" : " CAPTURE"), 0);
            graphics_print_text(20, 7, (s_action_menu_sel == 2 ? ">WAIT  " : " WAIT  "), 0);
        } else if (s_sub_state == SUB_FIELD_MENU) {
            // Field Menu (End Turn / CO Power)
            for (u8 y = 5; y <= 8; y++) {
                for (u8 x = 18; x <= 28; x++) {
                    u16 tile = TILE_BOX_FILL;
                    if (y == 5 && x == 18) tile = TILE_BOX_TL;
                    else if (y == 5 && x == 28) tile = TILE_BOX_TR;
                    else if (y == 8 && x == 18) tile = TILE_BOX_BL;
                    else if (y == 8 && x == 28) tile = TILE_BOX_BR;
                    else if (y == 5 || y == 8) tile = TILE_BOX_H;
                    else if (x == 18 || x == 28) tile = TILE_BOX_V;
                    graphics_set_bg0_tile(x, y, tile, 0);
                }
            }

            if (s_co_meter_player >= 4) {
                graphics_print_text(19, 6, ">CO POWER!", 0);
            } else {
                graphics_print_text(19, 6, ">END TURN", 0);
            }
            graphics_print_text(19, 7, " CANCEL  ", 0);
        }
    } else if (s_state == STATE_MISSION_CLEAR) {
        graphics_print_text(8, 7, "MISSION COMPLETE!", 0);
        graphics_print_text(6, 9, "ENEMY FORCES ROUTED", 0);
        graphics_print_text(7, 11, "PRESS A TO ADVANCE", 0);
    } else if (s_state == STATE_VICTORY) {
        graphics_print_text(7, 6, "CAMPAIGN VICTORY!", 0);
        graphics_print_text(5, 8, "THE IRON CITADEL HAS FALLEN", 0);
        graphics_print_text(7, 10, "TACTICAL RANK: S-RANK", 0);
        graphics_print_text(7, 12, "PRESS START TO RETRY", 0);
    } else if (s_state == STATE_GAME_OVER) {
        graphics_print_text(8, 7, "HEADQUARTERS LOST", 0);
        graphics_print_text(7, 9, "OPERATION FAILED", 0);
        graphics_print_text(7, 11, "PRESS START TO RETRY", 0);
    }

    graphics_oam_copy();
}
