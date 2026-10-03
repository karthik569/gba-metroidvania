#ifndef AUDIO_H
#define AUDIO_H

#include "gba.h"

typedef enum {
    BGM_NONE = 0,
    BGM_TITLE,
    BGM_PLAYER_PHASE,
    BGM_ENEMY_PHASE,
    BGM_CO_POWER,
    BGM_VICTORY
} BgmTrack;

#ifdef __cplusplus
extern "C" {
#endif

void audio_init(void);
void audio_update(void);
void audio_play_bgm(BgmTrack track);
void audio_stop_bgm(void);

// Tactical Sound Effects
void sfx_cursor_move(void);
void sfx_select(void);
void sfx_cancel(void);
void sfx_unit_move(void);
void sfx_cannon_fire(void);
void sfx_autocannon_burst(void);
void sfx_rocket_launch(void);
void sfx_explosion(void);
void sfx_capture(void);
void sfx_co_power(void);

#ifdef __cplusplus
}
#endif

#endif // AUDIO_H
