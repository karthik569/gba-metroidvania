#ifndef ASSETS_H
#define ASSETS_H

#include "gba.h"

#ifdef __cplusplus
extern "C" {
#endif

// Base Block settings
#define BG_CBB_TILES    0
#define BG0_SBB         26   // UI / Text (32x32)
#define BG1_SBB         28   // Foreground arches / lighting (64x32 -> uses 28 & 29)
#define BG2_SBB         30   // Terrain map (64x32 -> uses 30 & 31)

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
#define TILE_HEART_FULL         49
#define TILE_HEART_HALF         50
#define TILE_HEART_EMPTY        51
#define TILE_MP_ICON            52
#define TILE_KEY_ICON           53
#define TILE_COIN_ICON          54
#define TILE_BOX_TL             55
#define TILE_BOX_TOP            56
#define TILE_BOX_TR             57
#define TILE_BOX_LEFT           58
#define TILE_BOX_RIGHT          59
#define TILE_BOX_BL             60
#define TILE_BOX_BOTTOM         61
#define TILE_BOX_BR             62
#define TILE_BOX_FILL           63

// Terrain 8x8 Base Tiles (Metatiles are 2x2 of these)
#define TILE_FLOOR_STONE_TL     64  // 64..67: Stone floor metatile
#define TILE_FLOOR_DIRT_TL      68  // 68..71: Dirt/grass metatile
#define TILE_WALL_TOP_TL        72  // 72..75: Wall top crenelation
#define TILE_WALL_FACE_TL       76  // 76..79: Wall face brick
#define TILE_WATER_TL           80  // 80..83: Subterranean water
#define TILE_BRIDGE_TL          84  // 84..87: Wood plank bridge
#define TILE_DOOR_OPEN_TL       88  // 88..91: Open passage
#define TILE_DOOR_LOCKED_TL     92  // 92..95: Iron portcullis gate
#define TILE_DOOR_BOSS_TL       96  // 96..99: Boss skull door
#define TILE_WALL_CRACKED_TL    100 // 100..103: Cracked wall (bombable)
#define TILE_SWITCH_OFF_TL      104 // 104..107: Crystal switch (blue/down)
#define TILE_SWITCH_ON_TL       108 // 108..111: Crystal switch (red/up)
#define TILE_PLATE_UP_TL        112 // 112..115: Pressure plate up
#define TILE_PLATE_DOWN_TL      116 // 116..119: Pressure plate down
#define TILE_TORCH_UNLIT_TL     120 // 120..123: Wall torch unlit
#define TILE_TORCH_LIT_TL       124 // 124..127: Wall torch lit
#define TILE_CHEST_CLOSED_TL    128 // 128..131: Treasure chest closed
#define TILE_CHEST_OPEN_TL      132 // 132..135: Treasure chest open
#define TILE_POT_TL             136 // 136..139: Breakable clay pot
#define TILE_BLOCK_TL           140 // 140..143: Pushable granite block
#define TILE_CAMPFIRE_TL        144 // 144..147: Crackling campfire
#define TILE_TREE_TL            148 // 148..151: Pine tree
#define TILE_ARCH_TL            152 // 152..155: Foreground arch / pillar (BG1)
#define TILE_VIGNETTE_TL        156 // 156..159: Darkness mask vignette (BG1)

// Metatile IDs (16x16 macro tiles)
typedef enum {
    META_FLOOR_STONE    = 0,
    META_FLOOR_DIRT     = 1,
    META_WALL_TOP       = 2,
    META_WALL_FACE      = 3,
    META_WATER          = 4,
    META_BRIDGE         = 5,
    META_DOOR_OPEN      = 6,
    META_DOOR_LOCKED    = 7,
    META_DOOR_BOSS      = 8,
    META_WALL_CRACKED   = 9,
    META_SWITCH_OFF     = 10,
    META_SWITCH_ON      = 11,
    META_PLATE_UP       = 12,
    META_PLATE_DOWN     = 13,
    META_TORCH_UNLIT    = 14,
    META_TORCH_LIT      = 15,
    META_CHEST_CLOSED   = 16,
    META_CHEST_OPEN     = 17,
    META_POT            = 18,
    META_BLOCK          = 19,
    META_CAMPFIRE       = 20,
    META_TREE           = 21,
    META_COUNT          = 22
} MetatileId;

// Sprite Tile Indices in OBJ VRAM (1D mapped, each 16x16 sprite uses 4 8x8 tiles)
#define SPRITE_KNIGHT_DOWN_0    0   // 0..3
#define SPRITE_KNIGHT_DOWN_1    4   // 4..7
#define SPRITE_KNIGHT_UP_0      8   // 8..11
#define SPRITE_KNIGHT_UP_1      12  // 12..15
#define SPRITE_KNIGHT_SIDE_0    16  // 16..19 (hflip for other side)
#define SPRITE_KNIGHT_SIDE_1    20  // 20..23
#define SPRITE_KNIGHT_SLASH_D   24  // 24..27
#define SPRITE_KNIGHT_SLASH_U   28  // 28..31
#define SPRITE_KNIGHT_SLASH_S   32  // 32..35
#define SPRITE_KNIGHT_ROLL_0    36  // 36..39
#define SPRITE_KNIGHT_ROLL_1    40  // 40..43
#define SPRITE_SWORD_ARC_D      44  // 44..47
#define SPRITE_SWORD_ARC_U      48  // 48..51
#define SPRITE_SWORD_ARC_S      52  // 52..55

// Weapons / Projectiles / VFX
#define SPRITE_BOOMERANG_0      56  // 56..59
#define SPRITE_BOOMERANG_1      60  // 60..63
#define SPRITE_BOMB             64  // 64..67
#define SPRITE_EXPLOSION_0      68  // 68..71
#define SPRITE_EXPLOSION_1      72  // 72..75
#define SPRITE_EXPLOSION_2      76  // 76..79
#define SPRITE_FIREBALL_0       80  // 80..83
#define SPRITE_FIREBALL_1       84  // 84..87

// Monsters
#define SPRITE_SLIME_0          88  // 88..91
#define SPRITE_SLIME_1          92  // 92..95
#define SPRITE_SKELETON_D_0     96  // 96..99
#define SPRITE_SKELETON_D_1     100 // 100..103
#define SPRITE_SKELETON_S_0     104 // 104..107
#define SPRITE_SKELETON_S_1     108 // 108..111
#define SPRITE_BAT_0            112 // 112..115
#define SPRITE_BAT_1            116 // 116..119
#define SPRITE_BLADE_TRAP       120 // 120..123

// Boss: Ancient Crypt Golem (32x32 multi-part: 4 quadrants of 16x16)
#define SPRITE_GOLEM_TL         124 // 124..127
#define SPRITE_GOLEM_TR         128 // 128..131
#define SPRITE_GOLEM_BL         132 // 132..135
#define SPRITE_GOLEM_BR         136 // 136..139
#define SPRITE_GOLEM_FIST       140 // 140..143
#define SPRITE_GOLEM_CORE_EYE   144 // 144..147
#define SPRITE_STALACTITE       148 // 148..151

// Pickups & Items
#define SPRITE_HEART_PICKUP     152 // 152..155
#define SPRITE_MP_PICKUP        156 // 156..159
#define SPRITE_COIN_PICKUP      160 // 160..163
#define SPRITE_KEY_PICKUP       164 // 164..167
#define SPRITE_BOSS_KEY_PICKUP  168 // 168..171
#define SPRITE_HEART_CONTAINER  172 // 172..175
#define SPRITE_NPC_LOREKEEPER   176 // 176..179
#define SPRITE_SMOKE_PUFF       180 // 180..183

// Palette Indices
#define PAL_BG_STONE        0
#define PAL_BG_NATURE       1
#define PAL_BG_WATER        2
#define PAL_BG_FIRE         3
#define PAL_BG_UI           4
#define PAL_BG_SHADOW       5

#define PAL_OBJ_KNIGHT      0
#define PAL_OBJ_MONSTER     1
#define PAL_OBJ_FIRE_BOMB   2
#define PAL_OBJ_GOLEM       3
#define PAL_OBJ_ITEMS       4
#define PAL_OBJ_NPC         5

// Asset upload functions
void assets_load_palettes(void);
void assets_load_tiles(void);

#ifdef __cplusplus
}
#endif

#endif // ASSETS_H
