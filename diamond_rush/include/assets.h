#ifndef ASSETS_H
#define ASSETS_H

#include "gba.h"
#include "asset_data.h"

#ifdef __cplusplus
extern "C" {
#endif

// Level Theme Selection
typedef enum {
    THEME_ANGKOR  = 0,
    THEME_BAVARIA = 1
} LevelTheme;

// Base Block settings
#define BG_CBB_TILES    0
#define BG0_SBB         26   // UI / HUD / Menus (32x32)
#define BG1_SBB         28   // Interactive layer (64x32 -> uses 28 & 29)
#define BG2_SBB         30   // Temple bedrock / floor (64x32 -> uses 30 & 31)

// Background & Object Palettes
#define PAL_BG_UI               0
#define PAL_BG_STONE            1
#define PAL_BG_DIRT             2
#define PAL_BG_GEMS             3
#define PAL_BG_OBJECTS          4

#define PAL_OBJ_EXPLORER        0
#define PAL_OBJ_ENEMIES         1
#define PAL_OBJ_BOULDER         2
#define PAL_OBJ_EFFECTS         2

// Background 8x8 Tile Indices in CBB 0
#define TILE_EMPTY              0
#define TILE_SOLID_BLACK        1
#define TILE_SOLID_WHITE        2

// Font tiles (ASCII mapped)
#define TILE_DIGIT_0            3   // 3..12 ('0'..'9')
#define TILE_LETTER_A           13  // 13..38 ('A'..'Z')
#define TILE_COLON              39  // ':'
#define TILE_MINUS              40  // '-'
#define TILE_DOT                41  // '.'
#define TILE_EXCLAMATION        42  // '!'
#define TILE_QUESTION           43  // '?'
#define TILE_SLASH              44  // '/'
#define TILE_ARROW_L            45  // '<'
#define TILE_ARROW_R            46  // '>'
#define TILE_BRACKET_L          47  // '['
#define TILE_BRACKET_R          48  // ']'

// UI / HUD 8x8 Tiles
#define TILE_DIAMOND_ICON       49
#define TILE_HEART_FULL         50
#define TILE_HEART_EMPTY        51
#define TILE_KEY_ICON           52
#define TILE_STAR_ICON          53
#define TILE_BOX_TL             54
#define TILE_BOX_TOP            55
#define TILE_BOX_TR             56
#define TILE_BOX_LEFT           57
#define TILE_BOX_RIGHT          58
#define TILE_BOX_BL             59
#define TILE_BOX_BOTTOM         60
#define TILE_BOX_BR             61
#define TILE_BOX_FILL           62

// Metatile 8x8 Base Tile Offsets in CBB 0 (4 8x8 tiles per 16x16 metatile: TL, TR, BL, BR)
#define TILE_FLOOR_TL           64   // 64..67: Weathered flagstones
#define TILE_WALL_TL            68   // 68..71: Carved ancient stone wall
#define TILE_DIRT_TL            72   // 72..75: Soft foliage / rich earth
#define TILE_BOULDER_TL         76   // 76..79: Chiseled granite boulder
#define TILE_DIAMOND_RED_TL     80   // 80..83: Sparkling Red Ruby
#define TILE_DIAMOND_PURPLE_TL  84   // 84..87: Faceted Amethyst
#define TILE_DIAMOND_GREEN_TL   88   // 88..91: Faceted Emerald
#define TILE_KEY_TL             92   // 92..95: Ancient Golden Key
#define TILE_DOOR_LOCKED_TL     96   // 96..99: Heavy Portcullis Locked
#define TILE_DOOR_OPEN_TL       100  // 100..103: Raised Portcullis Open
#define TILE_CHEST_CLOSED_TL    104  // 104..107: Closed Treasure Chest
#define TILE_CHEST_OPEN_TL      108  // 108..111: Opened Overflowing Chest
#define TILE_SPIKES_TL          112  // 112..115: Ground Steel Spikes
#define TILE_PRESSURE_PLATE_TL  116  // 116..119: Carved Sun Pressure Plate
#define TILE_BARRIER_TL         120  // 120..123: Retractable Column Barrier
#define TILE_EXIT_GATE_TL       124  // 124..127: Ancient Exit Portal

// BG2 Subterranean Parallax Bedrock Tiles (in CBB 0)
#define TILE_PILLAR_L_TL        128  // 128..131: Ancient Sandstone Pillar Left
#define TILE_PILLAR_R_TL        132  // 132..135: Ancient Sandstone Pillar Right
#define TILE_FACE_TL            136  // 136..139: Carved Buddha Face Relief
#define TILE_BEDROCK_TL         140  // 140..143: Ancient Foundation Masonry Block

// Title Screen 3D Embossed Banner Tiles (80 tiles: 20x4 tiles = 160x32px)
#define TILE_TITLE_LOGO_BASE    256  // 256..335

// Sprite Tile Indices in OBJ VRAM (1D mapped, 4 8x8 tiles per 16x16 sprite)
// Explorer (0..63)
#define SPRITE_EXPLORER_D0      0    // Down Idle
#define SPRITE_EXPLORER_D1      4    // Down Walk Step 0
#define SPRITE_EXPLORER_D2      8    // Down Walk Step 1 (Passing)
#define SPRITE_EXPLORER_D3      12   // Down Walk Step 2
#define SPRITE_EXPLORER_U0      16   // Up Idle
#define SPRITE_EXPLORER_U1      20   // Up Walk Step 0
#define SPRITE_EXPLORER_U2      24   // Up Walk Step 1
#define SPRITE_EXPLORER_U3      28   // Up Walk Step 2
#define SPRITE_EXPLORER_S0      32   // Side Idle
#define SPRITE_EXPLORER_S1      36   // Side Walk Step 0
#define SPRITE_EXPLORER_S2      40   // Side Walk Step 1
#define SPRITE_EXPLORER_S3      44   // Side Walk Step 2
#define SPRITE_EXPLORER_PUSH0   48   // Pushing Stance
#define SPRITE_EXPLORER_PUSH1   52   // Pushing Strain
#define SPRITE_EXPLORER_SQUASH  56   // Squashed Pancake / Hurt
#define SPRITE_EXPLORER_VICTORY 60   // Cheering Victory Pose

// Enemies (64..95)
#define SPRITE_SNAKE_0          64   // Cobra Slither 0
#define SPRITE_SNAKE_1          68   // Cobra Slither 1 (Flicking Tongue)
#define SPRITE_SNAKE_2          72   // Cobra Slither 2
#define SPRITE_SNAKE_3          76   // Cobra Strike Hood Flare
#define SPRITE_SPIDER_0         80   // Spider Crawl 0
#define SPRITE_SPIDER_1         84   // Spider Crawl 1
#define SPRITE_SPIDER_2         88   // Spider Crawl 2
#define SPRITE_SPIDER_3         92   // Spider Crawl 3

// Objects & FX (96..159)
#define SPRITE_BOULDER_0        96   // Rolling Boulder 0 deg
#define SPRITE_BOULDER_1        100  // Rolling Boulder 90 deg
#define SPRITE_BOULDER_2        104  // Rolling Boulder 180 deg
#define SPRITE_BOULDER_3        108  // Rolling Boulder 270 deg
#define SPRITE_DIAMOND_RED      112  // Falling Red Ruby
#define SPRITE_DIAMOND_PURPLE   116  // Falling Amethyst
#define SPRITE_DIAMOND_GREEN    120  // Falling Emerald
#define SPRITE_SPARKLE_0        124  // Sparkle Small
#define SPRITE_SPARKLE_1        128  // Sparkle Cross
#define SPRITE_SPARKLE_2        132  // Sparkle Large Flare
#define SPRITE_DUST_0           136  // Excavation Debris Small
#define SPRITE_DUST_1           140  // Leaf & Dirt Burst
#define SPRITE_DUST_2           144  // Debris Dissipating
#define SPRITE_GOO_SPLAT        148  // Crushed Enemy Goo Splatter Decal
#define SPRITE_SHADOW           152  // Ground drop-shadow oval
#define SPRITE_IMPACT_DUST      156  // Boulder landing impact dust shockwave

// Compatibility Aliases for game.c
#define SPRITE_BOULDER          SPRITE_BOULDER_0
#define SPRITE_DIAMOND          SPRITE_DIAMOND_RED
#define SPRITE_EXPLORER_PUSH    SPRITE_EXPLORER_PUSH0

// Function prototypes
void assets_init(void);
void assets_load_theme(LevelTheme theme);
void assets_update_vblank_effects(u32 frame_count);

#ifdef __cplusplus
}
#endif

#endif // ASSETS_H
