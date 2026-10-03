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

// 16-color Object / Sprite Palette
static const u16 s_obj_palette[16] = {
    0,                  // 0: Transparent
    RGB15(3, 4, 6),     // 1: Armor Black Shadow
    RGB15(5, 10, 22),   // 2: Cyber Blue Suit
    RGB15(8, 20, 28),   // 3: Suit Highlight Cyan
    RGB15(22, 30, 31),  // 4: White Armor Plating
    RGB15(31, 14, 2),   // 5: Visor Amber
    RGB15(31, 28, 4),   // 6: Visor Bright Flare
    RGB15(8, 26, 31),   // 7: Blaster Energy Cyan
    RGB15(28, 4, 4),    // 8: Missile Crimson
    RGB15(31, 16, 2),   // 9: Missile Exhaust Orange
    RGB15(16, 4, 20),   // 10: Alien Violet Carapace
    RGB15(4, 28, 24),   // 11: Alien Bioluminescent Eye
    RGB15(22, 14, 4),   // 12: Sentinel Boss Bronze
    RGB15(31, 2, 2),    // 13: Sentinel Laser Red
    RGB15(31, 31, 12),  // 14: Energy Spark Yellow
    RGB15(31, 31, 31)   // 15: Pure White Sparkle
};

// Background Tiles (8 tiles * 8 words each = 64 words)
static const u32 s_bg_tiles[64] = {
    // Tile 0: Empty / Void space (Color 0)
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,

    // Tile 1: Solid Metal Block (Riveted Plate with bevel)
    0x33333333, 0x32222221, 0x32722271, 0x32222221,
    0x32222221, 0x32722271, 0x32222221, 0x11111111,

    // Tile 2: Platform Grate
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

// Sprite Tiles: Each 8x8 tile = exactly 8 words!
// In 1D mapping mode, 16x16 uses [T, T+1, T+2, T+3]
static const u32 s_obj_tiles[] = {
    // -------------------------------------------------------------
    // Tile 0..3: Player Idle (16x16 = 4 tiles)
    // -------------------------------------------------------------
    // Tile 0: Top-Left (Head/Visor left)
    0x00000111, 0x00014444, 0x00145666, 0x00146666,
    0x00013333, 0x00233333, 0x02433333, 0x02433333,
    // Tile 1: Top-Right (Head/Cannon right)
    0x11000000, 0x44100000, 0x66410000, 0x66410000,
    0x33100000, 0x33200770, 0x33420770, 0x33420770,
    // Tile 2: Bottom-Left (Legs left)
    0x00233333, 0x00222222, 0x00200000, 0x00200000,
    0x00300000, 0x00400000, 0x00400000, 0x01100000,
    // Tile 3: Bottom-Right (Legs right)
    0x33200000, 0x22100000, 0x00200000, 0x00200000,
    0x00300000, 0x00400000, 0x00400000, 0x01100000,

    // -------------------------------------------------------------
    // Tile 4..7: Player Run (16x16 = 4 tiles)
    // -------------------------------------------------------------
    // Tile 4: Top-Left
    0x00000111, 0x00014444, 0x00145666, 0x00146666,
    0x00013333, 0x02433333, 0x02333333, 0x00233333,
    // Tile 5: Top-Right
    0x11000000, 0x44100000, 0x66410000, 0x66410000,
    0x33100000, 0x33420000, 0x33720000, 0x33200000,
    // Tile 6: Bottom-Left (Running legs)
    0x00200000, 0x02000000, 0x03000000, 0x04000000,
    0x11000000, 0x00000000, 0x00000000, 0x00000000,
    // Tile 7: Bottom-Right (Trailing leg)
    0x00000020, 0x00000020, 0x00000030, 0x00000040,
    0x00000011, 0x00000000, 0x00000000, 0x00000000,

    // -------------------------------------------------------------
    // Tile 8..11: Player Jump (16x16 = 4 tiles)
    // -------------------------------------------------------------
    // Tile 8: Top-Left
    0x00000111, 0x00014444, 0x00145666, 0x00146666,
    0x00233333, 0x02433333, 0x02333333, 0x00211111,
    // Tile 9: Top-Right
    0x11000000, 0x44100000, 0x66410000, 0x66410000,
    0x33200000, 0x33720000, 0x33200000, 0x11200000,
    // Tile 10: Bottom-Left (Tucked legs)
    0x02220000, 0x03330000, 0x01110000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    // Tile 11: Bottom-Right
    0x00002220, 0x00003330, 0x00001110, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,

    // -------------------------------------------------------------
    // Tile 12..15: Player Nano-Drone / Morph (16x16 = 4 tiles)
    // -------------------------------------------------------------
    // Tile 12: Top-Left sphere
    0x00000001, 0x00001441, 0x00013773, 0x0017FFFF,
    0x0017FFFF, 0x00013773, 0x00001441, 0x00000001,
    // Tile 13: Top-Right sphere
    0x10000000, 0x14410000, 0x37731000, 0xFFFF7100,
    0xFFFF7100, 0x37731000, 0x14410000, 0x10000000,
    // Tile 14: Bottom-Left sphere
    0x00000001, 0x00001441, 0x00013773, 0x0017FFFF,
    0x0017FFFF, 0x00013773, 0x00001441, 0x00000001,
    // Tile 15: Bottom-Right sphere
    0x10000000, 0x14410000, 0x37731000, 0xFFFF7100,
    0xFFFF7100, 0x37731000, 0x14410000, 0x10000000,

    // -------------------------------------------------------------
    // Tile 16: Blaster Beam (8x8 = 1 tile)
    // -------------------------------------------------------------
    0x00000000, 0x00777700, 0x07FFFF70, 0x07FFFF70,
    0x07FFFF70, 0x00777700, 0x00000000, 0x00000000,

    // -------------------------------------------------------------
    // Tile 17: Concussion Missile (8x8 = 1 tile)
    // -------------------------------------------------------------
    0x00088000, 0x00888800, 0x088FF880, 0x88FFFF88,
    0x088FF880, 0x00999900, 0x00099000, 0x00000000,

    // Tile 18..19: Empty padding (2 tiles)
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,

    // -------------------------------------------------------------
    // Tile 20..23: Alien Crawler (16x16 = 4 tiles)
    // -------------------------------------------------------------
    // Tile 20: Top-Left
    0x00000001, 0x00001AA1, 0x0001ABBA, 0x001ABBBA,
    0x01ABBBBA, 0x0001AAAA, 0x00000100, 0x00001000,
    // Tile 21: Top-Right
    0x10000000, 0x1AA10000, 0xABBA1000, 0xABBBA100,
    0xABBBBA10, 0xAAAA1000, 0x00100000, 0x00010000,
    // Tile 22: Bottom-Left (Legs)
    0x01000000, 0x10000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    // Tile 23: Bottom-Right (Legs)
    0x00000010, 0x00000001, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,

    // -------------------------------------------------------------
    // Tile 24..27: Missile Item Pickup (16x16 = 4 tiles)
    // -------------------------------------------------------------
    // Tile 24: Top-Left
    0x00000008, 0x000008FF, 0x00008FFF, 0x0008FFFF,
    0x0008FFFF, 0x00008FFF, 0x000008FF, 0x00000008,
    // Tile 25: Top-Right
    0x80000000, 0xFF800000, 0xFFF80000, 0xFFFF8000,
    0xFFFF8000, 0xFFF80000, 0xFF800000, 0x80000000,
    // Tile 26: Bottom-Left
    0x00000008, 0x000008FF, 0x00008FFF, 0x0008FFFF,
    0x0008FFFF, 0x00008FFF, 0x000008FF, 0x00000008,
    // Tile 27: Bottom-Right
    0x80000000, 0xFF800000, 0xFFF80000, 0xFFFF8000,
    0xFFFF8000, 0xFFF80000, 0xFF800000, 0x80000000,

    // Tile 28..31: Empty padding (4 tiles)
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,

    // -------------------------------------------------------------
    // Tile 32..47: Boss Sentinel (32x32 = 16 tiles)
    // -------------------------------------------------------------
    // 16 tiles of 8 words each = 128 words
    0x00011000, 0x001CC100, 0x01CDDC10, 0x1CDDDDC1, 0x1CDDDC10, 0x01CCCC10, 0x00111000, 0x00000000,
    0x00011000, 0x001CC100, 0x01CDDC10, 0x1CDDDDC1, 0x1CDDDC10, 0x01CCCC10, 0x00111000, 0x00000000,
    0x00011000, 0x001CC100, 0x01CDDC10, 0x1CDDDDC1, 0x1CDDDC10, 0x01CCCC10, 0x00111000, 0x00000000,
    0x00011000, 0x001CC100, 0x01CDDC10, 0x1CDDDDC1, 0x1CDDDC10, 0x01CCCC10, 0x00111000, 0x00000000,
    0x00011000, 0x001CC100, 0x01CDDC10, 0x1CDDDDC1, 0x1CDDDC10, 0x01CCCC10, 0x00111000, 0x00000000,
    0x00011000, 0x001CC100, 0x01CDDC10, 0x1CDDDDC1, 0x1CDDDC10, 0x01CCCC10, 0x00111000, 0x00000000,
    0x00011000, 0x001CC100, 0x01CDDC10, 0x1CDDDDC1, 0x1CDDDC10, 0x01CCCC10, 0x00111000, 0x00000000,
    0x00011000, 0x001CC100, 0x01CDDC10, 0x1CDDDDC1, 0x1CDDDC10, 0x01CCCC10, 0x00111000, 0x00000000,
    0x00011000, 0x001CC100, 0x01CDDC10, 0x1CDDDDC1, 0x1CDDDC10, 0x01CCCC10, 0x00111000, 0x00000000,
    0x00011000, 0x001CC100, 0x01CDDC10, 0x1CDDDDC1, 0x1CDDDC10, 0x01CCCC10, 0x00111000, 0x00000000,
    0x00011000, 0x001CC100, 0x01CDDC10, 0x1CDDDDC1, 0x1CDDDC10, 0x01CCCC10, 0x00111000, 0x00000000,
    0x00011000, 0x001CC100, 0x01CDDC10, 0x1CDDDDC1, 0x1CDDDC10, 0x01CCCC10, 0x00111000, 0x00000000,
    0x00011000, 0x001CC100, 0x01CDDC10, 0x1CDDDDC1, 0x1CDDDC10, 0x01CCCC10, 0x00111000, 0x00000000,
    0x00011000, 0x001CC100, 0x01CDDC10, 0x1CDDDDC1, 0x1CDDDC10, 0x01CCCC10, 0x00111000, 0x00000000,
    0x00011000, 0x001CC100, 0x01CDDC10, 0x1CDDDDC1, 0x1CDDDC10, 0x01CCCC10, 0x00111000, 0x00000000,
    0x00011000, 0x001CC100, 0x01CDDC10, 0x1CDDDDC1, 0x1CDDDC10, 0x01CCCC10, 0x00111000, 0x00000000
};

void assets_init(void) {
    load_bg_palette(s_bg_palette, 16);
    load_obj_palette(s_obj_palette, 16);

    // 64 words of BG tiles into CharBlock 0
    load_bg_tiles(s_bg_tiles, 64, 0);

    // OBJ tiles into CharBlock 4
    load_obj_tiles(s_obj_tiles, sizeof(s_obj_tiles) / sizeof(u32));
}
