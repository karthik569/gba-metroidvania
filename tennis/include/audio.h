#ifndef AUDIO_H
#define AUDIO_H

#include "gba.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BGM_NONE,
    BGM_MENU,
    BGM_MATCH,
    BGM_VICTORY
} BgmTrack;

void audio_init(void);
void audio_play_bgm(BgmTrack track);
void audio_stop_bgm(void);
void audio_update(void);

// Tennis Sound Effects
void sfx_play_racket_topspin(void);
void sfx_play_racket_slice(void);
void sfx_play_racket_smash(void);
void sfx_play_ball_bounce(CourtSurface surface);
void sfx_play_net_hit(void);
void sfx_play_umpire_chime(void);
void sfx_play_crowd_cheer(void);

#ifdef __cplusplus
}
#endif

#endif // AUDIO_H
