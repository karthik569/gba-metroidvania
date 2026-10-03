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
// Each word contains 8 4-bit nibbles (representing 8 horizontal pixels)
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

// Sprite Tiles: Player, Enemies, Projectiles
// Standard 4bpp tiles
static const u32 s_obj_tiles[] = {
    // Tile 0..7: Player Upper & Lower Body (Idle frame)
    // 0: Head/Visor
    0x00011000, 0x00144100, 0x01456410, 0x01466410,
    0x00133100, 0x00233200, 0x02433420, 0x02433420,
    // 1: Torso/Cannon
    0x02333320, 0x02433720, 0x00233200, 0x00122100,
    0x00211200, 0x00200200, 0x00300300, 0x00100100,
    // 2: Legs Idle
    0x00200200, 0x00200200, 0x00300300, 0x00300300,
    0x00400400, 0x00400400, 0x01100110, 0x01100110,
    // 3: Empty filler
    0, 0, 0, 0, 0, 0, 0, 0,
    // 4..7: Filler
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,

    // Tile 8..15: Player Run frames
    0x00011000, 0x00144100, 0x01456410, 0x01466410,
    0x00133100, 0x02433420, 0x02333720, 0x00233200,
    0x00200200, 0x02000020, 0x03000030, 0x04000040,
    0x11000011, 0x00000000, 0x00000000, 0x00000000,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,

    // Tile 16..23: Player Jump Frame
    0x00011000, 0x00144100, 0x01456410, 0x01466410,
    0x00233200, 0x02433720, 0x02333200, 0x00211200,
    0x02200220, 0x03300330, 0x01100110, 0x00000000,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,

    // Tile 24..27: Player Nano-Drone / Morph Form (16x16 = 4 tiles)
    0x00011000, 0x00144100, 0x01377310, 0x017FF710,
    0x017FF710, 0x01377310, 0x00144100, 0x00011000,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,

    // Tile 28: Cannon Blaster Energy Beam (8x8)
    0x00000000, 0x00777700, 0x07FFFF70, 0x07FFFF70,
    0x07FFFF70, 0x00777700, 0x00000000, 0x00000000,

    // Tile 29: Concussion Missile (8x8)
    0x00088000, 0x00888800, 0x088FF880, 0x88FFFF88,
    0x088FF880, 0x00999900, 0x00099000, 0x00000000,

    // Tile 30..31: Empty
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,

    // Tile 32..35: Alien Crawler Enemy (16x16 = 4 tiles)
    0x00011000, 0x001AA100, 0x01ABBA10, 0x1ABBBA10,
    0x1ABBBBA1, 0x01AAAA10, 0x01000010, 0x10000001,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,

    // Tile 40..55: Boss Sentinel (32x32 = 16 tiles)
    0x00011000, 0x001CC100, 0x01CDDC10, 0x1CDDDDC1,
    0x1CDDDC10, 0x01CCCC10, 0x00111000, 0x00000000,
    0x00111100, 0x01CCCC10, 0x1CDDDC10, 0x1CDDDDC1,
    0x01CDDC10, 0x001CC100, 0x00011000, 0x00000000,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,

    // Tile 56..59: Missile Upgrade Item Pickup (16x16 = 4 tiles)
    0x00088000, 0x008FF800, 0x08FFFF80, 0x8FFFFFF8,
    0x8FFFFFF8, 0x08FFFF80, 0x008FF800, 0x00088000,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

void assets_init(void) {
    load_bg_palette(s_bg_palette, 16);
    load_obj_palette(s_obj_palette, 16);

    // 64 words of BG tiles into CharBlock 0
    load_bg_tiles(s_bg_tiles, 64, 0);

    // OBJ tiles into CharBlock 4
    load_obj_tiles(s_obj_tiles, sizeof(s_obj_tiles) / sizeof(u32));
}
