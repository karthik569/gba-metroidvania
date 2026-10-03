#ifndef AUDIO_H
#define AUDIO_H

#include "gba.h"

#ifdef __cplusplus
extern "C" {
#endif

void sound_init(void);
void sfx_bounce(void);
void sfx_brick_hit(void);
void sfx_brick_hard(void);
void sfx_laser(void);
void sfx_powerup_drop(void);
void sfx_powerup_get(void);
void sfx_life_lost(void);
void sfx_stage_clear(void);

#ifdef __cplusplus
}
#endif

#endif // AUDIO_H
