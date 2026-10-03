#include "map.h"
#include "assets.h"
#include "gba.h"

static Room s_rooms[NUM_ROOMS];
static u8 s_current_room_id = 0;

// Helper to fill rectangle in room
static void fill_rect(Room* r, u8 x1, u8 y1, u8 x2, u8 y2, u8 tile) {
    for (u8 y = y1; y <= y2 && y < ROOM_HEIGHT_TILES; y++) {
        for (u8 x = x1; x <= x2 && x < ROOM_WIDTH_TILES; x++) {
            r->tiles[y][x] = tile;
        }
    }
}

// Build room borders with door openings
static void setup_borders(Room* r, bool left, bool right, bool up, bool down) {
    // Top & Bottom walls
    fill_rect(r, 0, 0, ROOM_WIDTH_TILES - 1, 1, TILE_SOLID_HULL);
    fill_rect(r, 0, ROOM_HEIGHT_TILES - 2, ROOM_WIDTH_TILES - 1, ROOM_HEIGHT_TILES - 1, TILE_SOLID_HULL);

    // Left & Right walls
    fill_rect(r, 0, 0, 1, ROOM_HEIGHT_TILES - 1, TILE_SOLID_HULL);
    fill_rect(r, ROOM_WIDTH_TILES - 2, 0, ROOM_WIDTH_TILES - 1, ROOM_HEIGHT_TILES - 1, TILE_SOLID_HULL);

    // Open doors
    if (left) fill_rect(r, 0, 14, 1, 17, TILE_EMPTY);
    if (right) fill_rect(r, ROOM_WIDTH_TILES - 2, 14, ROOM_WIDTH_TILES - 1, 17, TILE_EMPTY);
    if (up) fill_rect(r, 13, 0, 16, 1, TILE_EMPTY);
    if (down) fill_rect(r, 13, ROOM_HEIGHT_TILES - 2, 16, ROOM_HEIGHT_TILES - 1, TILE_EMPTY);
}

void map_init(void) {
    // -------------------------------------------------------------
    // Room 0: Landing Dock (Start) -> Exits: Right (Room 1)
    // -------------------------------------------------------------
    Room* r0 = &s_rooms[0];
    setup_borders(r0, false, true, false, false);
    r0->exit_left = -1;
    r0->exit_right = 1;
    r0->exit_up = -1;
    r0->exit_down = -1;
    // Step platforms
    fill_rect(r0, 8, 15, 13, 15, TILE_GRATE);
    fill_rect(r0, 16, 12, 22, 12, TILE_GRATE);
    fill_rect(r0, 10, 8, 18, 8, TILE_CONDUIT);
    r0->enemy_type = 1; // Crawler
    r0->enemy_x = 120;
    r0->enemy_y = 128; // Floor at 144, crawler height 16 -> 128
    r0->item_type = 0;

    // -------------------------------------------------------------
    // Room 1: Airflow Shaft -> Exits: Left (R0), Right (R2), Down (R3)
    // -------------------------------------------------------------
    Room* r1 = &s_rooms[1];
    setup_borders(r1, true, true, false, true);
    r1->exit_left = 0;
    r1->exit_right = 2;
    r1->exit_up = -1;
    r1->exit_down = 3;
    // Staggered vertical climbing ledges
    fill_rect(r1, 4, 15, 9, 15, TILE_GRATE);
    fill_rect(r1, 20, 13, 26, 13, TILE_GRATE);
    fill_rect(r1, 8, 10, 14, 10, TILE_GRATE);
    fill_rect(r1, 18, 7, 24, 7, TILE_GRATE);
    fill_rect(r1, 10, 4, 16, 4, TILE_CONDUIT);
    r1->enemy_type = 1;
    r1->enemy_x = 180;
    r1->enemy_y = 88; // Grate at y=104 -> crawler height 16 -> 88
    r1->item_type = 0;

    // -------------------------------------------------------------
    // Room 2: Security Armory (Missiles) -> Exits: Left (R1)
    // -------------------------------------------------------------
    Room* r2 = &s_rooms[2];
    setup_borders(r2, true, false, false, false);
    r2->exit_left = 1;
    r2->exit_right = -1;
    r2->exit_up = -1;
    r2->exit_down = -1;
    // Pedestal in center
    fill_rect(r2, 12, 14, 18, 17, TILE_SOLID_HULL);
    fill_rect(r2, 13, 13, 17, 13, TILE_CONDUIT);
    r2->enemy_type = 0;
    r2->item_type = 1; // Missile upgrade
    r2->item_x = 120;
    r2->item_y = 90;

    // -------------------------------------------------------------
    // Room 3: Power Conduit (Morph Tunnel) -> Exits: Up (R1), Down (R4)
    // Vertical descent shaft with platforms and a Morph Drone crawlway
    // -------------------------------------------------------------
    Room* r3 = &s_rooms[3];
    setup_borders(r3, false, false, true, true);
    r3->exit_left = -1;
    r3->exit_right = -1;
    r3->exit_up = 1;
    r3->exit_down = 4;

    // Upper descending platforms (player lands safely from R1 top door at x=13..16, y=0..1)
    fill_rect(r3, 11, 4, 18, 4, TILE_GRATE);  // Safe landing grate right below top door
    fill_rect(r3, 4, 7, 9, 7, TILE_GRATE);    // Left step ledge
    fill_rect(r3, 20, 7, 25, 7, TILE_GRATE);  // Right step ledge
    fill_rect(r3, 12, 9, 17, 9, TILE_GRATE);  // Mid transition platform

    // Mid divider wall (x=2..19) leaving descent shaft on right (x=20..27)
    fill_rect(r3, 2, 10, 19, 11, TILE_SOLID_HULL);

    // Right shaft landing platform
    fill_rect(r3, 20, 13, 27, 13, TILE_SOLID_HULL);

    // 2-tile crawl tunnel (height 16px) requiring Morph Drone to pass to the left!
    // Ceiling of crawlway
    fill_rect(r3, 5, 12, 19, 13, TILE_SOLID_HULL);
    // Tunnel itself is empty at y=14..15, x=5..19
    // Floor of crawlway
    fill_rect(r3, 5, 16, 19, 16, TILE_SOLID_HULL);

    // Platform grate above bottom door (x=12..17, y=16) for jumping up when coming from R4
    fill_rect(r3, 12, 16, 17, 16, TILE_GRATE);

    // Acid hazard pool on floor away from door and drop shaft
    fill_rect(r3, 20, 17, 26, 17, TILE_HAZARD);

    r3->enemy_type = 0;
    r3->item_type = 0;

    // -------------------------------------------------------------
    // Room 4: Hub Corridor -> Exits: Up (R3), Right (R5), Down (R6)
    // Blocked by red security barrier blocks (needs Missiles!)
    // -------------------------------------------------------------
    Room* r4 = &s_rooms[4];
    setup_borders(r4, false, true, true, true);
    r4->exit_left = -1;
    r4->exit_right = 5;
    r4->exit_up = 3;
    r4->exit_down = 6;
    // Central platform
    fill_rect(r4, 8, 12, 21, 12, TILE_SOLID_HULL);
    // Red Security Barrier blocking the right door to boss room!
    fill_rect(r4, 27, 14, 27, 17, TILE_RED_BARRIER);
    fill_rect(r4, 28, 14, 28, 17, TILE_RED_BARRIER);
    r4->enemy_type = 1;
    r4->enemy_x = 100;
    r4->enemy_y = 80; // Platform at 96 -> height 16 -> 80
    r4->item_type = 0;

    // -------------------------------------------------------------
    // Room 5: Boss Chamber (Sentinel Arena) -> Exits: Left (R4)
    // -------------------------------------------------------------
    Room* r5 = &s_rooms[5];
    setup_borders(r5, true, false, false, false);
    r5->exit_left = 4;
    r5->exit_right = -1;
    r5->exit_up = -1;
    r5->exit_down = -1;
    // Large combat arena with dodge platforms
    fill_rect(r5, 5, 13, 10, 13, TILE_GRATE);
    fill_rect(r5, 19, 13, 24, 13, TILE_GRATE);
    fill_rect(r5, 12, 9, 17, 9, TILE_CONDUIT);
    r5->enemy_type = 2; // Boss Sentinel
    r5->enemy_x = 160;
    r5->enemy_y = 60;
    r5->item_type = 0;

    // -------------------------------------------------------------
    // Room 6: Save Station -> Exits: Up (R4)
    // -------------------------------------------------------------
    Room* r6 = &s_rooms[6];
    setup_borders(r6, false, false, true, false);
    r6->exit_left = -1;
    r6->exit_right = -1;
    r6->exit_up = 4;
    r6->exit_down = -1;
    // Save Console in middle of room
    fill_rect(r6, 13, 16, 16, 17, TILE_SAVE_CONSOLE);
    r6->enemy_type = 0;
    r6->item_type = 0;

    // Load initial room
    map_load_room(0);
}

void map_load_room(u8 room_id) {
    if (room_id >= NUM_ROOMS) return;
    s_current_room_id = room_id;

    // ScreenBlock 28 starts at VRAM + (28 * 0x800) = 0x0600E000
    vu16* screenblock = (vu16*)(VRAM_BASE + (28 * 0x800));

    // GBA ScreenBlock 32x32 tiles
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            u16 tile_val = TILE_EMPTY;
            if (y < ROOM_HEIGHT_TILES && x < ROOM_WIDTH_TILES) {
                tile_val = s_rooms[room_id].tiles[y][x];
            }
            screenblock[y * 32 + x] = tile_val;
        }
    }
}

u8 map_get_tile(s16 pixel_x, s16 pixel_y) {
    // Check horizontal doorways
    if (pixel_x >= SCREEN_WIDTH) {
        if (s_rooms[s_current_room_id].exit_right >= 0 && pixel_y >= 112 && pixel_y < 144) {
            return TILE_EMPTY;
        }
        return TILE_SOLID_HULL;
    }
    if (pixel_x < 0) {
        if (s_rooms[s_current_room_id].exit_left >= 0 && pixel_y >= 112 && pixel_y < 144) {
            return TILE_EMPTY;
        }
        return TILE_SOLID_HULL;
    }

    // Check vertical doorways
    if (pixel_y < 0) {
        if (s_rooms[s_current_room_id].exit_up >= 0 && pixel_x >= 104 && pixel_x < 136) {
            return TILE_EMPTY;
        }
        return TILE_SOLID_HULL;
    }
    if (pixel_y >= SCREEN_HEIGHT) {
        if (s_rooms[s_current_room_id].exit_down >= 0 && pixel_x >= 104 && pixel_x < 136) {
            return TILE_EMPTY;
        }
        return TILE_SOLID_HULL;
    }

    u8 tx = pixel_x / 8;
    u8 ty = pixel_y / 8;
    return s_rooms[s_current_room_id].tiles[ty][tx];
}

void map_set_tile(u8 tile_x, u8 tile_y, u8 tile_id) {
    if (tile_x >= ROOM_WIDTH_TILES || tile_y >= ROOM_HEIGHT_TILES) return;
    s_rooms[s_current_room_id].tiles[tile_y][tile_x] = tile_id;

    // Update VRAM ScreenBlock directly
    vu16* screenblock = (vu16*)(VRAM_BASE + (28 * 0x800));
    screenblock[tile_y * 32 + tile_x] = tile_id;
}

void map_clear_room_item(u8 room_id) {
    if (room_id < NUM_ROOMS) {
        s_rooms[room_id].item_type = 0;
    }
}

u8 map_current_room_id(void) {
    return s_current_room_id;
}

const Room* map_get_current_room(void) {
    return &s_rooms[s_current_room_id];
}

const char* map_get_room_name(u8 room_id) {
    switch (room_id) {
        case 0: return "Landing Dock";
        case 1: return "Airflow Shaft";
        case 2: return "Security Armory";
        case 3: return "Power Conduit";
        case 4: return "Hub Corridor";
        case 5: return "Sentinel Boss Arena";
        case 6: return "Save Station";
        default: return "Unknown Sector";
    }
}
