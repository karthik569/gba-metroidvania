#ifndef AUDIO_H
#define AUDIO_H

#include "gba.h"

#ifdef __cplusplus
extern "C" {
#endif

void sound_init(void);
void sfx_laser(void);
void sfx_missile(void);
void sfx_jump(void);
void sfx_hit(void);
void sfx_pickup(void);
void sfx_boss_roar(void);

#ifdef __cplusplus
}
#endif

#endif // AUDIO_H
