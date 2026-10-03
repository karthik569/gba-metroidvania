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
    // Requires Morph / Nano-Drone crawlspace to pass
    // -------------------------------------------------------------
    Room* r3 = &s_rooms[3];
    setup_borders(r3, false, false, true, true);
    r3->exit_left = -1;
    r3->exit_right = -1;
    r3->exit_up = 1;
    r3->exit_down = 4;
    // Tight 1-tile crawlway across middle
    fill_rect(r3, 2, 6, 27, 13, TILE_SOLID_HULL);
    // 1-tile vertical shaft on left, 1-tile crawlway under block
    fill_rect(r3, 2, 14, 27, 14, TILE_EMPTY); // 1-tile crawl space height!
    // Hazard acid pool on floor
    fill_rect(r3, 8, 17, 21, 17, TILE_HAZARD);
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
    if (pixel_x < 0 || pixel_x >= SCREEN_WIDTH || pixel_y < 0 || pixel_y >= SCREEN_HEIGHT) {
        return TILE_SOLID_HULL; // Out of bounds is solid
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

u8 map_current_room_id(void) {
    return s_current_room_id;
}

const Room* map_get_current_room(void) {
    return &s_rooms[s_current_room_id];
}
