#ifndef ASSETS_H
#define ASSETS_H

#include "gba.h"

// BG Tile indices
#define TILE_EMPTY          0
#define TILE_BORDER_VERT    1
#define TILE_BORDER_HORIZ   2
#define TILE_BORDER_CORNER  3
#define TILE_BEZEL_PANEL    4
#define TILE_GIRDER_1       5
#define TILE_GIRDER_2       6
#define TILE_DEBRIS_1       10
#define TILE_DEBRIS_2       11
#define TILE_DEBRIS_3       12
#define TILE_STAR_DIM       16
#define TILE_STAR_MED       17
#define TILE_STAR_BRIGHT    18
#define TILE_NEBULA_1       19
#define TILE_NEBULA_2       20
#define TILE_COLON          21
#define TILE_MINUS          22
#define TILE_DOT            23
#define TILE_EXCLAMATION    24
#define TILE_ARROW_LEFT     25
#define TILE_ARROW_RIGHT    26
#define TILE_BRACKET_L      27
#define TILE_BRACKET_R      28
#define TILE_DIGIT_0        30
#define TILE_LETTER_A       40

// Sprite Tile indices in OBJ VRAM (1D mapped, each 8x8 is 1 unit, 16x16 is 4 units, 32x32 is 16 units)
#define SPRITE_TILE_PLAYER_NEUTRAL  0   // 16x16 (tiles 0..3)
#define SPRITE_TILE_PLAYER_LEFT     4   // 16x16 (tiles 4..7)
#define SPRITE_TILE_PLAYER_RIGHT    8   // 16x16 (tiles 8..11)
#define SPRITE_TILE_THRUSTER        12  // 8x8
#define SPRITE_TILE_WEAPON_VULCAN   13  // 8x8
#define SPRITE_TILE_WEAPON_LASER    14  // 8x8
#define SPRITE_TILE_WEAPON_PLASMA   15  // 8x8
#define SPRITE_TILE_SHIELD_AURA     16  // 16x16 (tiles 16..19)
#define SPRITE_TILE_BOMB_RING       20  // 32x32 (tiles 20..35)
#define SPRITE_TILE_ENEMY_SCOUT     36  // 16x16 (tiles 36..39)
#define SPRITE_TILE_ENEMY_INTER     40  // 16x16 (tiles 40..43)
#define SPRITE_TILE_ENEMY_GUNSHIP   44  // 32x16 (tiles 44..51)
#define SPRITE_TILE_ENEMY_BULLET    52  // 8x8
#define SPRITE_TILE_ENEMY_BOLT      53  // 8x8
#define SPRITE_TILE_PWR_WEAPON      54  // 16x8 (tiles 54..55) [P]
#define SPRITE_TILE_PWR_BOMB        56  // 16x8 (tiles 56..57) [B]
#define SPRITE_TILE_PWR_SHIELD      58  // 16x8 (tiles 58..59) [S]
#define SPRITE_TILE_PWR_1UP         60  // 16x8 (tiles 60..61) [1UP]
#define SPRITE_TILE_PARTICLE        62  // 8x8
#define SPRITE_TILE_EXPLOSION_1     64  // 16x16 (tiles 64..67)
#define SPRITE_TILE_EXPLOSION_2     68  // 16x16 (tiles 68..71)
#define SPRITE_TILE_BOSS_POD        72  // 16x16 (tiles 72..75)
#define SPRITE_TILE_BOSS_CORE       80  // 32x32 (tiles 80..95)

#ifdef __cplusplus
extern "C" {
#endif

void assets_init(void);
void assets_load_stage_theme(u8 stage);

#ifdef __cplusplus
}
#endif

#endif // ASSETS_H
