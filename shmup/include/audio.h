#ifndef AUDIO_H
#define AUDIO_H

#include "gba.h"

typedef enum {
    BGM_NONE = 0,
    BGM_TITLE,
    BGM_STAGE,
    BGM_BOSS,
    BGM_VICTORY
} BgmTrack;

#ifdef __cplusplus
extern "C" {
#endif

void audio_init(void);
void audio_update(void);
void audio_play_bgm(BgmTrack track);
void audio_stop_bgm(void);

// Sound effects
void sfx_laser_fire(void);
void sfx_plasma_wave(void);
void sfx_nova_bomb(void);
void sfx_enemy_explode(void);
void sfx_boss_explode(void);
void sfx_shield_hit(void);
void sfx_shield_break(void);
void sfx_powerup_pickup(void);
void sfx_player_death(void);
void sfx_extra_life(void);

#ifdef __cplusplus
}
#endif

#endif // AUDIO_H
