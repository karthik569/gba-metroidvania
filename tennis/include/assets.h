#ifndef ASSETS_H
#define ASSETS_H

#include "gba.h"

#ifdef __cplusplus
extern "C" {
#endif

// Character and Screen Base Blocks
#define BG_CBB_TILES    0
#define BG0_SBB         26   // Scoreboard HUD & Announcements (32x32)
#define BG1_SBB         28   // Stadium Stands, Crowd & Net (32x32)
#define BG2_SBB         30   // Perspective Tennis Court (32x32)

// 8x8 Background Tile Indices in CBB 0
#define TILE_EMPTY              0
#define TILE_SOLID_BLACK        1
#define TILE_SOLID_WHITE        2

// 5x7 ASCII Font
#define TILE_DIGIT_0            3   // 3..12 ('0'..'9')
#define TILE_LETTER_A           13  // 13..38 ('A'..'Z')
#define TILE_COLON              39  // ':'
#define TILE_MINUS              40  // '-'
#define TILE_DOT                41  // '.'
#define TILE_EXCLAMATION        42  // '!'
#define TILE_SLASH              43  // '/'
#define TILE_ARROW_L            44  // '<'
#define TILE_ARROW_R            45  // '>'
#define TILE_BRACKET_L          46  // '['
#define TILE_BRACKET_R          47  // ']'

// Scoreboard Box & Icons
#define TILE_BOX_TL             48
#define TILE_BOX_TOP            49
#define TILE_BOX_TR             50
#define TILE_BOX_LEFT           51
#define TILE_BOX_RIGHT          52
#define TILE_BOX_BL             53
#define TILE_BOX_BOTTOM         54
#define TILE_BOX_BR             55
#define TILE_BOX_FILL           56
#define TILE_BALL_ICON          57
#define TILE_TROPHY_ICON        58
#define TILE_RADAR_ICON         59

// Stadium & Net 8x8 Tiles
#define TILE_CROWD_0            60
#define TILE_CROWD_1            61
#define TILE_STADIUM_WALL       62
#define TILE_SPONSOR_BANNER     63
#define TILE_UMPIRE_CHAIR_T     64
#define TILE_UMPIRE_CHAIR_B     65
#define TILE_NET_POST_L         66
#define TILE_NET_POST_R         67
#define TILE_NET_TAPE           68
#define TILE_NET_MESH           69

// Court Surface & Line Tiles
#define TILE_COURT_SURFACE      70
#define TILE_COURT_LINE_H       71
#define TILE_COURT_LINE_V       72
#define TILE_COURT_LINE_SL_L    73
#define TILE_COURT_LINE_SL_R    74
#define TILE_COURT_CORNER_TL    75
#define TILE_COURT_CORNER_TR    76
#define TILE_COURT_CORNER_BL    77
#define TILE_COURT_CORNER_BR    78
#define TILE_COURT_T_MARK       79

// Continuous Perspective Sideline Tiles (Clean anti-aliased transitions)
#define TILE_COURT_SL_L_TOP     80
#define TILE_COURT_SL_L_MID     81
#define TILE_COURT_SL_L_BOT     82
#define TILE_COURT_SL_R_TOP     83
#define TILE_COURT_SL_R_MID     84
#define TILE_COURT_SL_R_BOT     85

// Stadium Roof & Side Audience Stands
#define TILE_STADIUM_ROOF       86
#define TILE_CROWD_SIDE_L0      87
#define TILE_CROWD_SIDE_L1      88
#define TILE_CROWD_SIDE_R0      89
#define TILE_CROWD_SIDE_R1      90

// 16x16 Sprite Tile Indices (1D mapping: 4 8x8 tiles per 16x16 sprite)
// Near Player
#define SPRITE_PLAYER_READY_0       0   // 0..3
#define SPRITE_PLAYER_READY_1       4   // 4..7
#define SPRITE_PLAYER_RUN_L         8   // 8..11
#define SPRITE_PLAYER_RUN_R         12  // 12..15
#define SPRITE_PLAYER_SWING_FH_0    16  // 16..19
#define SPRITE_PLAYER_SWING_FH_1    20  // 20..23
#define SPRITE_PLAYER_SWING_BH_0    24  // 24..27
#define SPRITE_PLAYER_SWING_BH_1    28  // 28..31
#define SPRITE_PLAYER_TOSS          32  // 32..35
#define SPRITE_PLAYER_SERVE_HIT     36  // 36..39
#define SPRITE_PLAYER_SMASH         40  // 40..43

// Far Opponent Player (Scaled for perspective)
#define SPRITE_OPP_READY_0          44  // 44..47
#define SPRITE_OPP_READY_1          48  // 48..51
#define SPRITE_OPP_RUN_L            52  // 52..55
#define SPRITE_OPP_RUN_R            56  // 56..59
#define SPRITE_OPP_SWING_FH         60  // 60..63
#define SPRITE_OPP_SWING_BH         64  // 64..67
#define SPRITE_OPP_SERVE            68  // 68..71

// Tennis Ball & Drop Shadow
#define SPRITE_BALL_SMALL           72  // 72..75
#define SPRITE_BALL_MID             76  // 76..79
#define SPRITE_BALL_LARGE           80  // 80..83
#define SPRITE_BALL_SHADOW          84  // 84..87

// VFX & Mini-Game
#define SPRITE_CHALK_PUFF           88  // 88..91
#define SPRITE_SWING_TRAIL_RED      92  // 92..95
#define SPRITE_SWING_TRAIL_BLUE     96  // 96..99
#define SPRITE_SWING_TRAIL_YEL      100 // 100..103
#define SPRITE_TARGET_RING          104 // 104..107
#define SPRITE_TROPHY_GOLD          108 // 108..111

// Palettes
#define PAL_BG_SCOREBOARD   0
#define PAL_BG_STADIUM      1
#define PAL_BG_GRASS        2
#define PAL_BG_CLAY         3
#define PAL_BG_HARD         4

#define PAL_OBJ_PLAYER      0
#define PAL_OBJ_OPPONENT    1
#define PAL_OBJ_BALL        2
#define PAL_OBJ_VFX         3
#define PAL_OBJ_TROPHY      4

void assets_load_palettes(void);
void assets_load_tiles(void);

#ifdef __cplusplus
}
#endif

#endif // ASSETS_H
