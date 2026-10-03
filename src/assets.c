#include "assets.h"
#include "graphics.h"

// 16-color Background Palette
static const u16 s_bg_palette[16] = {
    RGB15(1, 1, 2),     // 0: Deep Space Void
    RGB15(3, 5, 10),    // 1: Steel Blue Hull Shadow
    RGB15(7, 12, 18),   // 2: Hull Midtone
    RGB15(14, 22, 28),  // 3: Hull Highlight
    RGB15(6, 20, 24),   // 4: Neon Conduit Glow
    RGB15(5, 5, 7),     // 5: Dark Grate Plate
    RGB15(10, 10, 12),  // 6: Light Grate Plate
    RGB15(18, 18, 20),  // 7: Metallic Edge
    RGB15(20, 8, 2),    // 8: Hazard Orange Base
    RGB15(31, 24, 4),   // 9: Hazard Stripe Yellow
    RGB15(16, 2, 2),    // 10: Security Red Base
    RGB15(30, 4, 4),    // 11: Security Red Glow
    RGB15(3, 24, 8),    // 12: Bio Hazard Acid Glow
    RGB15(1, 10, 3),    // 13: Bio Slime Deep
    RGB15(31, 26, 6),   // 14: Save Station Gold
    RGB15(31, 31, 31)   // 15: Pure Energy White
};

// 16-color Object / Sprite Palette (Classic Metroid Power Suit aesthetic)
static const u16 s_obj_palette[16] = {
    0,                  // 0: Transparent
    RGB15(2, 2, 3),     // 1: Armor Black Shadow / Joint Liner
    RGB15(24, 4, 4),    // 2: Crimson Red Helmet & Torso Plate
    RGB15(31, 8, 8),    // 3: Bright Crimson Flare
    RGB15(28, 14, 2),   // 4: Varia Shoulder Amber / Orange
    RGB15(31, 22, 4),   // 5: Armor Gold / Yellow Primary
    RGB15(31, 28, 10),  // 6: Armor Bright Yellow Highlight
    RGB15(4, 28, 8),    // 7: Visor Emerald Green
    RGB15(16, 31, 16),  // 8: Visor Neon Lime Glint
    RGB15(4, 18, 10),   // 9: Arm Cannon Forest Green
    RGB15(8, 28, 31),   // 10: Energy Blaster Cyan
    RGB15(28, 2, 2),    // 11: Concussion Missile Red
    RGB15(18, 4, 22),   // 12: Alien Violet Carapace
    RGB15(22, 14, 4),   // 13: Sentinel Boss Bronze
    RGB15(31, 2, 2),    // 14: Sentinel Laser Red
    RGB15(31, 31, 31)   // 15: Pure Energy White
};

// Background Tiles (8 tiles * 8 words each = 64 words)
static const u32 s_bg_tiles[64] = {
    // Tile 0: Empty / Void space (Color 0)
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,

    // Tile 1: Solid Metal Block (Riveted Plate with bevel)
    0x33333333, 0x32222221, 0x32722271, 0x32222221,
    0x32222221, 0x32722271, 0x32222221, 0x11111111,

    // Tile 2: Platform Grate (One-way jump-through)
    0x77777777, 0x65656565, 0x56565656, 0x65656565,
    0x56565656, 0x65656565, 0x56565656, 0x11111111,

    // Tile 3: Glowing Energy Conduit Pipe
    0x22222222, 0x44444444, 0x4FFFFF44, 0x44444444,
    0x44444444, 0x4FFFFF44, 0x44444444, 0x22222222,

    // Tile 4: Hazard Bio-Acid Pool
    0x00000000, 0x00000000, 0x0C0C0C0C, 0xCCCCCCCC,
    0xCDCCCCCC, 0xDCDCCDCD, 0xDDDDDDDD, 0xDDDDDDDD,

    // Tile 5: Red Security Barrier Block (Missile destructible)
    0xBBBBBBBB, 0xBAAAAAAB, 0xBAFAAFAB, 0xBAFFAFAB,
    0xBAFFAFAB, 0xBAFAAFAB, 0xBAAAAAAB, 0xBBBBBBBB,

    // Tile 6: Energy Airlock Door
    0x33444433, 0x34FFFF43, 0x34FFFF43, 0x34FFFF43,
    0x34FFFF43, 0x34FFFF43, 0x34FFFF43, 0x33444433,

    // Tile 7: Computer Terminal / Save Console
    0x77777777, 0x7EEEEEE7, 0x7E3333E7, 0x7E3F33E7,
    0x7EEEEEE7, 0x72222227, 0x72626227, 0x11111111
};

// Sprite Tiles: strictly 8 words (32 bytes) per 8x8 tile
static const u32 s_obj_tiles[] = {
    // -------------------------------------------------------------
    // Tile 0..7: Player Humanoid Idle (16x32 = 8 tiles)
    // -------------------------------------------------------------
    // Tile 0: Top-Left (Helmet dome & green visor left)
    0x00000111, 0x00012333, 0x00133222, 0x01337888,
    0x01227888, 0x00122222, 0x00011111, 0x00014444,
    // Tile 1: Top-Right (Helmet dome & green visor right)
    0x11100000, 0x33321000, 0x22233100, 0x88873310,
    0x88872210, 0x22222100, 0x11111000, 0x44441000,
    // Tile 2: Chest-Left (Orange Varia shoulder & chest core)
    0x00144444, 0x01466644, 0x01466644, 0x00144422,
    0x00122233, 0x001223FF, 0x00122233, 0x00011122,
    // Tile 3: Chest-Right (Right shoulder & Arm Cannon!)
    0x44444100, 0x44666410, 0x44666410, 0x22444100,
    0x33221999, 0xFF32199A, 0x33221999, 0x22111100,
    // Tile 4: Waist-Left (Liner & gold thigh left)
    0x00011111, 0x00155555, 0x01566655, 0x01566655,
    0x00155555, 0x00155555, 0x00111111, 0x00155555,
    // Tile 5: Waist-Right (Liner & gold thigh right)
    0x11111000, 0x55555100, 0x55666510, 0x55666510,
    0x55555100, 0x55555100, 0x11111100, 0x55555100,
    // Tile 6: Boots-Left (Gold shin & armored boot left)
    0x00156655, 0x00156655, 0x00155555, 0x00155555,
    0x01555555, 0x01666665, 0x01111111, 0x00000000,
    // Tile 7: Boots-Right (Gold shin & armored boot right)
    0x55665100, 0x55665100, 0x55555100, 0x55555100,
    0x55555510, 0x56666610, 0x11111110, 0x00000000,

    // -------------------------------------------------------------
    // Tile 8..15: Player Humanoid Run Frame (16x32 = 8 tiles)
    // -------------------------------------------------------------
    // Tile 8: Top-Left (Forward leaning helmet)
    0x00000011, 0x00001233, 0x00013322, 0x00133788,
    0x00122788, 0x00012222, 0x00001111, 0x00014444,
    // Tile 9: Top-Right
    0x11110000, 0x33210000, 0x22331000, 0x88733100,
    0x88722100, 0x22221000, 0x11110000, 0x44410000,
    // Tile 10: Chest-Left (Armored Torso running)
    0x00144664, 0x01466644, 0x01466644, 0x00144422,
    0x00122233, 0x001223FF, 0x00122233, 0x00011122,
    // Tile 11: Chest-Right (Arm Cannon leveled forward)
    0x44444100, 0x44666410, 0x44666410, 0x22444100,
    0x33221999, 0xFF3219AA, 0x33221999, 0x22111100,
    // Tile 12: Legs-Left (Forward stride)
    0x00011111, 0x00155555, 0x01566655, 0x01566655,
    0x00155555, 0x00015555, 0x00001555, 0x00000155,
    // Tile 13: Legs-Right (Back stride)
    0x11111000, 0x55555100, 0x55666510, 0x55666510,
    0x55555100, 0x55551000, 0x55510000, 0x55100000,
    // Tile 14: Foot-Left (Planted forward boot)
    0x00000155, 0x00001565, 0x00015555, 0x00155555,
    0x01555555, 0x01666665, 0x01111111, 0x00000000,
    // Tile 15: Foot-Right (Trailing heel kick)
    0x55100000, 0x56510000, 0x55551000, 0x55551000,
    0x00055510, 0x00056610, 0x00011110, 0x00000000,

    // -------------------------------------------------------------
    // Tile 16..23: Player Humanoid Jump Frame (16x32 = 8 tiles)
    // -------------------------------------------------------------
    // Tile 16: Top-Left
    0x00000111, 0x00012333, 0x00133222, 0x01337888,
    0x01227888, 0x00122222, 0x00011111, 0x00014444,
    // Tile 17: Top-Right
    0x11100000, 0x33321000, 0x22233100, 0x88873310,
    0x88872210, 0x22222100, 0x11111000, 0x44441000,
    // Tile 18: Chest-Left
    0x00144444, 0x01466644, 0x01466644, 0x00144422,
    0x00122233, 0x001223FF, 0x00122233, 0x00011122,
    // Tile 19: Chest-Right (Cannon ready)
    0x44444100, 0x44666410, 0x44666410, 0x22444100,
    0x33221999, 0xFF32199A, 0x33221999, 0x22111100,
    // Tile 20: Tucked Legs-Left
    0x00011111, 0x00155555, 0x01566655, 0x01566655,
    0x01555555, 0x00155555, 0x00011111, 0x00000000,
    // Tile 21: Tucked Legs-Right
    0x11111000, 0x55555100, 0x55666510, 0x55666510,
    0x55555510, 0x55555100, 0x11111000, 0x00000000,
    // Tile 22: Airborne Boots-Left
    0x00015555, 0x00156665, 0x01555555, 0x01111111,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    // Tile 23: Airborne Boots-Right
    0x55551000, 0x56665100, 0x55555510, 0x11111110,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,

    // -------------------------------------------------------------
    // Tile 24..27: Player Morph Ball / Nano-Drone (16x16 = 4 tiles)
    // -------------------------------------------------------------
    // Tile 24: Top-Left (Glowing round sphere)
    0x00000001, 0x00001441, 0x00014554, 0x00145785,
    0x00155885, 0x00014554, 0x00001441, 0x00000001,
    // Tile 25: Top-Right
    0x10000000, 0x14410000, 0x45541000, 0x58754100,
    0x58855100, 0x45541000, 0x14410000, 0x10000000,
    // Tile 26: Bottom-Left
    0x00000001, 0x00001441, 0x00014554, 0x00145785,
    0x00155885, 0x00014554, 0x00001441, 0x00000001,
    // Tile 27: Bottom-Right
    0x10000000, 0x14410000, 0x45541000, 0x58754100,
    0x58855100, 0x45541000, 0x14410000, 0x10000000,

    // -------------------------------------------------------------
    // Tile 28: Blaster Beam (8x8 = 1 tile)
    // -------------------------------------------------------------
    0x00000000, 0x00AAAA00, 0x0AFFFA0, 0x0AFFFFA0,
    0x0AFFFFA0, 0x00AAAA00, 0x00000000, 0x00000000,

    // -------------------------------------------------------------
    // Tile 29: Concussion Missile (8x8 = 1 tile)
    // -------------------------------------------------------------
    0x000BB000, 0x00BBBB00, 0x0BBFFBB0, 0xBBFFFFBB,
    0x0BBFFBB0, 0x00666600, 0x00066000, 0x00000000,

    // Tile 30..31: Empty padding (2 tiles)
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,

    // -------------------------------------------------------------
    // Tile 32..35: Alien Crawler (16x16 = 4 tiles)
    // -------------------------------------------------------------
    // Tile 32: Top-Left
    0x00000001, 0x00001CC1, 0x0001CDDC, 0x001CDDDC,
    0x01CDDDDC, 0x0001CCCC, 0x00000100, 0x00001000,
    // Tile 33: Top-Right
    0x10000000, 0x1CC10000, 0xCDDC1000, 0xCDDDC100,
    0xCDDDDC10, 0xCCCC1000, 0x00100000, 0x00010000,
    // Tile 34: Bottom-Left (Legs)
    0x01000000, 0x10000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    // Tile 35: Bottom-Right (Legs)
    0x00000010, 0x00000001, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,

    // -------------------------------------------------------------
    // Tile 36..39: Missile Item Upgrade Pod (16x16 = 4 tiles)
    // -------------------------------------------------------------
    // Tile 36: Top-Left
    0x0000000B, 0x00000BFF, 0x0000BFFF, 0x000BFFFF,
    0x000BFFFF, 0x0000BFFF, 0x00000BFF, 0x0000000B,
    // Tile 37: Top-Right
    0xB0000000, 0xFFB00000, 0xFFFB0000, 0xFFFFB000,
    0xFFFFB000, 0xFFFB0000, 0xFFB00000, 0xB0000000,
    // Tile 38: Bottom-Left
    0x0000000B, 0x00000BFF, 0x0000BFFF, 0x000BFFFF,
    0x000BFFFF, 0x0000BFFF, 0x00000BFF, 0x0000000B,
    // Tile 39: Bottom-Right
    0xB0000000, 0xFFB00000, 0xFFFB0000, 0xFFFFB000,
    0xFFFFB000, 0xFFFB0000, 0xFFB00000, 0xB0000000,

    // -------------------------------------------------------------
    // Tile 40..55: Boss Sentinel (32x32 = 16 tiles)
    // -------------------------------------------------------------
    0x00011000, 0x001DD100, 0x01DEED10, 0x1DEEEED1, 0x1DEEED10, 0x01DDDD10, 0x00111000, 0x00000000,
    0x00011000, 0x001DD100, 0x01DEED10, 0x1DEEEED1, 0x1DEEED10, 0x01DDDD10, 0x00111000, 0x00000000,
    0x00011000, 0x001DD100, 0x01DEED10, 0x1DEEEED1, 0x1DEEED10, 0x01DDDD10, 0x00111000, 0x00000000,
    0x00011000, 0x001DD100, 0x01DEED10, 0x1DEEEED1, 0x1DEEED10, 0x01DDDD10, 0x00111000, 0x00000000,
    0x00011000, 0x001DD100, 0x01DEED10, 0x1DEEEED1, 0x1DEEED10, 0x01DDDD10, 0x00111000, 0x00000000,
    0x00011000, 0x001DD100, 0x01DEED10, 0x1DEEEED1, 0x1DEEED10, 0x01DDDD10, 0x00111000, 0x00000000,
    0x00011000, 0x001DD100, 0x01DEED10, 0x1DEEEED1, 0x1DEEED10, 0x01DDDD10, 0x00111000, 0x00000000,
    0x00011000, 0x001DD100, 0x01DEED10, 0x1DEEEED1, 0x1DEEED10, 0x01DDDD10, 0x00111000, 0x00000000,
    0x00011000, 0x001DD100, 0x01DEED10, 0x1DEEEED1, 0x1DEEED10, 0x01DDDD10, 0x00111000, 0x00000000,
    0x00011000, 0x001DD100, 0x01DEED10, 0x1DEEEED1, 0x1DEEED10, 0x01DDDD10, 0x00111000, 0x00000000,
    0x00011000, 0x001DD100, 0x01DEED10, 0x1DEEEED1, 0x1DEEED10, 0x01DDDD10, 0x00111000, 0x00000000,
    0x00011000, 0x001DD100, 0x01DEED10, 0x1DEEEED1, 0x1DEEED10, 0x01DDDD10, 0x00111000, 0x00000000,
    0x00011000, 0x001DD100, 0x01DEED10, 0x1DEEEED1, 0x1DEEED10, 0x01DDDD10, 0x00111000, 0x00000000,
    0x00011000, 0x001DD100, 0x01DEED10, 0x1DEEEED1, 0x1DEEED10, 0x01DDDD10, 0x00111000, 0x00000000,
    0x00011000, 0x001DD100, 0x01DEED10, 0x1DEEEED1, 0x1DEEED10, 0x01DDDD10, 0x00111000, 0x00000000,
    0x00011000, 0x001DD100, 0x01DEED10, 0x1DEEEED1, 0x1DEEED10, 0x01DDDD10, 0x00111000, 0x00000000,

    // -------------------------------------------------------------
    // Tile 56: HUD Icon (8x8 = 1 tile) - Energy Heart / E-Tank
    // -------------------------------------------------------------
    0x00330330, 0x03FF3FF3, 0x3FFFFFF3, 0x3FFFFFF3,
    0x03FFFF30, 0x003FF300, 0x00033000, 0x00000000,

    // -------------------------------------------------------------
    // Tile 57: HUD Missile Icon (8x8 = 1 tile)
    // -------------------------------------------------------------
    0x000BB000, 0x00BFB000, 0x0BBFBB00, 0x0BBFBB00,
    0x0BBFBB00, 0x00666000, 0x00060000, 0x00000000,

    // -------------------------------------------------------------
    // Tile 58: HUD Selector Arrow (8x8 = 1 tile)
    // -------------------------------------------------------------
    0x00000000, 0x000AA000, 0x0000AA00, 0x00000AA0,
    0x0000AA00, 0x000AA000, 0x00000000, 0x00000000,

    // Tile 59: Padding
    0, 0, 0, 0, 0, 0, 0, 0,

    // -------------------------------------------------------------
    // Tile 60..69: HUD Digits '0'..'9' (8x8 = 10 tiles)
    // -------------------------------------------------------------
    // Tile 60: '0'
    0x00000000, 0x00FFFF00, 0x00F00F00, 0x00F00F00,
    0x00F00F00, 0x00F00F00, 0x00FFFF00, 0x00000000,
    // Tile 61: '1'
    0x00000000, 0x0000F000, 0x000FF000, 0x000FF000,
    0x000FF000, 0x000FF000, 0x00FFFF00, 0x00000000,
    // Tile 62: '2'
    0x00000000, 0x00FFFF00, 0x00F00000, 0x00FFFF00,
    0x00000F00, 0x00000F00, 0x00FFFF00, 0x00000000,
    // Tile 63: '3'
    0x00000000, 0x00FFFF00, 0x00F00000, 0x00FFFF00,
    0x00F00000, 0x00F00000, 0x00FFFF00, 0x00000000,
    // Tile 64: '4'
    0x00000000, 0x00F00F00, 0x00F00F00, 0x00FFFF00,
    0x00F00000, 0x00F00000, 0x00F00000, 0x00000000,
    // Tile 65: '5'
    0x00000000, 0x00FFFF00, 0x00000F00, 0x00FFFF00,
    0x00F00000, 0x00F00000, 0x00FFFF00, 0x00000000,
    // Tile 66: '6'
    0x00000000, 0x00FFFF00, 0x00000F00, 0x00FFFF00,
    0x00F00F00, 0x00F00F00, 0x00FFFF00, 0x00000000,
    // Tile 67: '7'
    0x00000000, 0x00FFFF00, 0x00F00000, 0x00F00000,
    0x000F0000, 0x000F0000, 0x000F0000, 0x00000000,
    // Tile 68: '8'
    0x00000000, 0x00FFFF00, 0x00F00F00, 0x00FFFF00,
    0x00F00F00, 0x00F00F00, 0x00FFFF00, 0x00000000,
    // Tile 69: '9'
    0x00000000, 0x00FFFF00, 0x00F00F00, 0x00FFFF00,
    0x00F00000, 0x00F00000, 0x00FFFF00, 0x00000000
};

void assets_init(void) {
    load_bg_palette(s_bg_palette, 16);
    load_obj_palette(s_obj_palette, 16);

    // 64 words of BG tiles into CharBlock 0
    load_bg_tiles(s_bg_tiles, 64, 0);

    // OBJ tiles into CharBlock 4
    load_obj_tiles(s_obj_tiles, sizeof(s_obj_tiles) / sizeof(u32));
}
