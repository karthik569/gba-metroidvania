#ifndef AUDIO_H
#define AUDIO_H

#include "gba.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BGM_NONE,
    BGM_TITLE,
    BGM_EXPEDITION,
    BGM_VICTORY,
    BGM_GAME_OVER
} BgmTrack;

void audio_init(void);
void audio_play_bgm(BgmTrack track);
void audio_stop_bgm(void);
void audio_update(void);

// Sound Effects
void sfx_play_dig_dirt(void);
void sfx_play_gem_pickup(void);
void sfx_play_boulder_push(void);
void sfx_play_boulder_land(void);
void sfx_play_boulder_roll(void);
void sfx_play_snake_hiss(void);
void sfx_play_snake_squash(void);
void sfx_play_key_pickup(void);
void sfx_play_door_open(void);
void sfx_play_chest_open(void);
void sfx_play_player_hurt(void);
void sfx_play_plate_click(void);
void sfx_play_exit_open(void);

#ifdef __cplusplus
}
#endif

#endif // AUDIO_H
