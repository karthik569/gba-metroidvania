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
void sfx_hazard_spawn(void);
void sfx_hazard_hit(void);
void sfx_tnt_explode(void);
void sfx_regen_pulse(void);
void sfx_kinetic_hit(void);
void sfx_menu_tick(void);
void jingle_title(void);
void jingle_victory(void);

// Power-Up SFX Suite
void sfx_barrier_hit(void);
void sfx_barrier_shatter(void);
void sfx_mega_smash(void);

// Boss Audio Suite (Stage 10 & Stage 20)
void sfx_boss_fire(void);
void sfx_paddle_stun(void);
void sfx_boss_pod_destroyed(void);
void sfx_boss_hurt(void);
void sfx_boss_defeat(void);
void bgm_boss_start(void);
void bgm_boss_stop(void);
void bgm_boss_tick(bool enraged);

#ifdef __cplusplus
}
#endif

#endif // AUDIO_H
