#ifndef ASSETS_H
#define ASSETS_H

#include "gba.h"

// BG Tile indices (8x8 tiles in CBB 0)
#define TILE_EMPTY              0
#define TILE_BOX_TL             1
#define TILE_BOX_TR             2
#define TILE_BOX_BL             3
#define TILE_BOX_BR             4
#define TILE_BOX_H              5
#define TILE_BOX_V              6
#define TILE_BOX_FILL           7

// Reach Grid Highlight Tiles (BG1)
#define TILE_MOVE_GRID          8
#define TILE_ATTACK_GRID        9
#define TILE_EMP_GRID           10

// Terrain 16x16 Macro Tile components (4 tiles per macro block: TL, TR, BL, BR)
#define TILE_PLAINS_TL          12
#define TILE_PLAINS_TR          13
#define TILE_PLAINS_BL          14
#define TILE_PLAINS_BR          15

#define TILE_FOREST_TL          16
#define TILE_FOREST_TR          17
#define TILE_FOREST_BL          18
#define TILE_FOREST_BR          19

#define TILE_MOUNTAIN_TL        20
#define TILE_MOUNTAIN_TR        21
#define TILE_MOUNTAIN_BL        22
#define TILE_MOUNTAIN_BR        23

#define TILE_RIVER_TL           24
#define TILE_RIVER_TR           25
#define TILE_RIVER_BL           26
#define TILE_RIVER_BR           27

#define TILE_ROAD_TL            28
#define TILE_ROAD_TR            29
#define TILE_ROAD_BL            30
#define TILE_ROAD_BR            31

#define TILE_CITY_TL            32
#define TILE_CITY_TR            33
#define TILE_CITY_BL            34
#define TILE_CITY_BR            35

#define TILE_HQ_P_TL            36
#define TILE_HQ_P_TR            37
#define TILE_HQ_P_BL            38
#define TILE_HQ_P_BR            39

#define TILE_HQ_E_TL            40
#define TILE_HQ_E_TR            41
#define TILE_HQ_E_BL            42
#define TILE_HQ_E_BR            43

// Punctuation & UI Glyphs
#define TILE_COLON              44
#define TILE_MINUS              45
#define TILE_DOT                46
#define TILE_EXCLAMATION        47
#define TILE_ARROW_LEFT         48
#define TILE_ARROW_RIGHT        49
#define TILE_BRACKET_L          50
#define TILE_BRACKET_R          51
#define TILE_SLASH              52
#define TILE_PERCENT            53
#define TILE_DIGIT_0            60
#define TILE_LETTER_A           70

// Sprite Tile indices in OBJ VRAM (1D mapping)
#define SPRITE_TILE_WALKER      0   // 16x16 (tiles 0..3)
#define SPRITE_TILE_RECON       4   // 16x16 (tiles 4..7)
#define SPRITE_TILE_TANK        8   // 16x16 (tiles 8..11)
#define SPRITE_TILE_ARTILLERY   12  // 16x16 (tiles 12..15)
#define SPRITE_TILE_VTOL        16  // 16x16 (tiles 16..19)
#define SPRITE_TILE_CURSOR      20  // 16x16 (tiles 20..23)
#define SPRITE_TILE_EXPLOSION   24  // 16x16 (tiles 24..27)
#define SPRITE_TILE_TRACER      28  // 8x8
#define SPRITE_TILE_DAMAGE_NUM  29  // 8x8
#define SPRITE_TILE_HP_BADGE    30  // 8x8 (tiles 30..39 for '0'..'9')

#ifdef __cplusplus
extern "C" {
#endif

void assets_init(void);

#ifdef __cplusplus
}
#endif

#endif // ASSETS_H
