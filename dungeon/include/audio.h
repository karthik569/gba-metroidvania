#ifndef AUDIO_H
#define AUDIO_H

#include "gba.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BGM_NONE,
    BGM_CAMP,
    BGM_CRYPT,
    BGM_BOSS,
    BGM_VICTORY
} BgmTrack;

void audio_init(void);
void audio_play_bgm(BgmTrack track);
void audio_stop_bgm(void);
void audio_update(void);

// Sound Effects
void sfx_play_sword_swing(void);
void sfx_play_enemy_hit(void);
void sfx_play_player_hurt(void);
void sfx_play_dodge_roll(void);
void sfx_play_boomerang(void);
void sfx_play_bomb_drop(void);
void sfx_play_bomb_explode(void);
void sfx_play_fire_wand(void);
void sfx_play_door_unlock(void);
void sfx_play_chest_open(void);
void sfx_play_heart_pickup(void);
void sfx_play_puzzle_solve(void);

#ifdef __cplusplus
}
#endif

#endif // AUDIO_H
