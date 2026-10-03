#include "audio.h"

void sound_init(void) {
    // 1. Master sound circuit power-on (Bit 7 = 1)
    REG_SOUNDCNT_X = 0x0080;

    // 2. Set default PWM bias level (0x0200 = 32.768kHz output)
    REG_SOUNDBIAS = 0x0200;

    // 3. Enable PSG channels 1, 2, 3, 4 to BOTH Left and Right stereo speakers at max volume (7)
    // Bits 0-2: Right volume (7)
    // Bits 4-6: Left volume (7)
    // Bits 8-11: Enable Ch1-4 to Right (0xF)
    // Bits 12-15: Enable Ch1-4 to Left (0xF)
    REG_SOUNDCNT_L = 0xFF77;

    // 4. PSG Sound 1-4 volume ratio set to 100% (Bits 0-1 = 2)
    REG_SOUNDCNT_H = 0x0002;
}

void sfx_bounce(void) {
    // Channel 1: Clean upward square chirp ping (no overflow)
    REG_SOUND1CNT_L = 0x0017;       // Sweep time 1, increase, shift 7
    REG_SOUND1CNT_H = 0xF280;       // Duty 50%, envelope decay 2, initial vol 15
    REG_SOUND1CNT_X = 0x8680;       // Trigger restart
}

void sfx_brick_hit(void) {
    // Channel 2: Crisp punchy brick pop
    REG_SOUND2CNT_L = 0xF180;       // Duty 50%, envelope decay 1, vol 15
    REG_SOUND2CNT_H = 0x85E0;       // Freq ~1500Hz, trigger restart
    // Channel 4: Micro noise snap
    REG_SOUND4CNT_L = 0xC100;       // Vol 12, decay 1
    REG_SOUND4CNT_H = 0x8010;       // High snap, trigger restart
}

void sfx_brick_hard(void) {
    // Channel 1: High metallic clink
    REG_SOUND1CNT_L = 0x0000;       // No sweep
    REG_SOUND1CNT_H = 0xF1C0;       // Duty 75%, fast decay, max vol 15
    REG_SOUND1CNT_X = 0x87A0;       // High metallic pitch ~1950Hz, trigger
}

void sfx_laser(void) {
    // Channel 1: Arcade laser zap (downward sweep)
    REG_SOUND1CNT_L = 0x002D;       // Sweep time 2, decrease, shift 5
    REG_SOUND1CNT_H = 0xF180;       // Duty 50%, initial vol 15, envelope decay 1
    REG_SOUND1CNT_X = 0x86D0;       // Initial freq, trigger restart
}

void sfx_powerup_drop(void) {
    // Channel 2: Gentle chime
    REG_SOUND2CNT_L = 0xA280;       // Duty 50%, vol 10, decay 2
    REG_SOUND2CNT_H = 0x8680;       // Trigger restart
}

void sfx_powerup_get(void) {
    // Channel 1: Rising fanfare chime
    REG_SOUND1CNT_L = 0x0022;       // Sweep time 2, upward, shift 2
    REG_SOUND1CNT_H = 0xF280;       // Duty 50%, vol 15, decay 2
    REG_SOUND1CNT_X = 0x8540;       // Trigger restart
}

void sfx_life_lost(void) {
    // Channel 4: Deep explosion burst
    REG_SOUND4CNT_L = 0xF500;       // Vol 15, decay step 5 (sustained deep rumble)
    REG_SOUND4CNT_H = 0x8048;       // Low rumble freq, 7-stage LFSR, trigger restart
    // Channel 1: Descending death pitch
    REG_SOUND1CNT_L = 0x003E;       // Downward sweep
    REG_SOUND1CNT_H = 0xE380;       // Vol 14, decay 3
    REG_SOUND1CNT_X = 0x8400;       // Trigger restart
}

void sfx_stage_clear(void) {
    // Channel 1: Victory fanfare
    REG_SOUND1CNT_L = 0x0013;       // Fast upward sweep
    REG_SOUND1CNT_H = 0xF380;       // High duty, sustained
    REG_SOUND1CNT_X = 0x86A0;       // Trigger restart
}

void sfx_hazard_spawn(void) {
    // Channel 1: Sci-fi hazard alert warble (upward sweep)
    REG_SOUND1CNT_L = 0x0025;       // Sweep time 2, upward (+), shift 5
    REG_SOUND1CNT_H = 0xC240;       // Duty 25%, envelope decay 2, initial vol 12
    REG_SOUND1CNT_X = 0x8500;       // Trigger restart
}

void sfx_hazard_hit(void) {
    // Channel 4: White noise vaporize burst
    REG_SOUND4CNT_L = 0xE200;       // Initial vol 14, decay 2
    REG_SOUND4CNT_H = 0x8022;       // Crisp mid/high noise, 7-stage, trigger restart
    // Channel 2: Crunch body
    REG_SOUND2CNT_L = 0xD280;       // Duty 50%, vol 13, decay 2
    REG_SOUND2CNT_H = 0x8480;       // Mid freq punch, trigger restart
}

void sfx_tnt_explode(void) {
    // Channel 4: Deep, booming explosive rumble
    REG_SOUND4CNT_L = 0xF600;       // Vol 15, envelope decay step 6 (extended rumble)
    REG_SOUND4CNT_H = 0x8052;       // Low rumble frequency, 15-stage LFSR, trigger restart
    // Channel 1: Concussive downward sweep
    REG_SOUND1CNT_L = 0x003E;       // Sweep time 3, decrease, shift 6
    REG_SOUND1CNT_H = 0xF380;       // Initial vol 15, decay 3
    REG_SOUND1CNT_X = 0x8380;       // Low blast pitch ~350Hz, trigger restart
}

void sfx_regen_pulse(void) {
    // Channel 2: Futuristic tech harmonic pulse
    REG_SOUND2CNT_L = 0xC340;       // Duty 25%, vol 12, envelope decay 3
    REG_SOUND2CNT_H = 0x86C0;       // High tech harmonic ~1760Hz, trigger restart
}

void sfx_kinetic_hit(void) {
    // Channel 1: High metallic ping
    REG_SOUND1CNT_L = 0x0000;       // No sweep
    REG_SOUND1CNT_H = 0xF1C0;       // Duty 75%, fast decay 1, max vol 15
    REG_SOUND1CNT_X = 0x87C0;       // High metallic pitch ~2200Hz, trigger restart
    // Channel 4: Crisp tap snap
    REG_SOUND4CNT_L = 0xA100;       // Vol 10, fast decay 1
    REG_SOUND4CNT_H = 0x8010;       // Micro snap
}

void sfx_menu_tick(void) {
    // Channel 2: Crisp UI navigation click
    REG_SOUND2CNT_L = 0x9180;       // Duty 50%, vol 9, fast decay 1
    REG_SOUND2CNT_H = 0x86E0;       // Crisp pitch ~1975Hz, trigger restart
}

void jingle_title(void) {
    // Channel 1: Upward melodious fanfare chime
    REG_SOUND1CNT_L = 0x0013;       // Sweep time 1, upward, shift 3
    REG_SOUND1CNT_H = 0xE380;       // Vol 14, decay 3
    REG_SOUND1CNT_X = 0x8540;       // C5 (~1046Hz), trigger restart
    // Channel 2: Harmonizing fifth
    REG_SOUND2CNT_L = 0xB380;       // Vol 11, decay 3
    REG_SOUND2CNT_H = 0x8620;       // G5 (~1568Hz), trigger restart
}

void jingle_victory(void) {
    // Channel 1: Triumphant sustained fanfare chord
    REG_SOUND1CNT_L = 0x0012;       // Bright upward sweep
    REG_SOUND1CNT_H = 0xF480;       // Vol 15, sustained decay 4
    REG_SOUND1CNT_X = 0x8680;       // High celebratory pitch
    // Channel 2: Supporting brass harmony
    REG_SOUND2CNT_L = 0xE480;       // Vol 14, decay 4
    REG_SOUND2CNT_H = 0x8700;       // Octave harmonic
}

// -----------------------------------------------------------------
// Power-Up SFX Suite
// -----------------------------------------------------------------
void sfx_barrier_hit(void) {
    // Channel 1: High electric upward deflect buzz
    REG_SOUND1CNT_L = 0x0013;       // Sweep time 1, upward, shift 3
    REG_SOUND1CNT_H = 0xF240;       // Duty 25%, vol 15, decay 2
    REG_SOUND1CNT_X = 0x86E0;       // High bright frequency (~1975Hz)
    // Channel 2: Resonant harmonic ring
    REG_SOUND2CNT_L = 0xD280;       // Duty 50%, vol 13, decay 2
    REG_SOUND2CNT_H = 0x8740;       // High ring tone
}

void sfx_barrier_shatter(void) {
    // Channel 4: Electric spark sizzle noise
    REG_SOUND4CNT_L = 0xF300;       // Vol 15, decay 3
    REG_SOUND4CNT_H = 0x8025;       // Fast 7-stage LFSR electrical shatter
    // Channel 1: Downward collapse plunge
    REG_SOUND1CNT_L = 0x002D;       // Fast downward sweep
    REG_SOUND1CNT_H = 0xE280;       // Vol 14, decay 2
    REG_SOUND1CNT_X = 0x8480;       // Mid plunge frequency
}

void sfx_mega_smash(void) {
    // Channel 4: Deep, heavy concussive crunch
    REG_SOUND4CNT_L = 0xF200;       // Vol 15, fast decay 2
    REG_SOUND4CNT_H = 0x8032;       // Mid-low crunchy noise
    // Channel 1: Concussive punch
    REG_SOUND1CNT_L = 0x001F;       // Fast downward drop
    REG_SOUND1CNT_H = 0xF180;       // Max vol 15, fast punch
    REG_SOUND1CNT_X = 0x8520;       // Punchy mid frequency
}

// -----------------------------------------------------------------
// Boss Audio Suite (Stage 10 & Stage 20)
// -----------------------------------------------------------------
void sfx_boss_fire(void) {
    // Channel 1: Sci-fi laser discharge
    REG_SOUND1CNT_L = 0x001B;       // Fast downward sweep
    REG_SOUND1CNT_H = 0xD280;       // Vol 13, decay 2
    REG_SOUND1CNT_X = 0x8640;       // High laser frequency, trigger restart
}

void sfx_paddle_stun(void) {
    // Channel 4: Harsh electric fizz noise
    REG_SOUND4CNT_L = 0xE300;       // Vol 14, decay 3
    REG_SOUND4CNT_H = 0x8035;       // Fast 7-stage LFSR electrical buzz
    // Channel 2: Low-frequency stun drone
    REG_SOUND2CNT_L = 0xF140;       // Duty 25%, vol 15, fast decay 1
    REG_SOUND2CNT_H = 0x8240;       // Low buzz pitch (~130Hz)
}

void sfx_boss_pod_destroyed(void) {
    // Channel 4: Loud metallic detonation
    REG_SOUND4CNT_L = 0xF400;       // Vol 15, decay 4
    REG_SOUND4CNT_H = 0x8042;       // Heavy noise punch
    // Channel 1: Concussive drop
    REG_SOUND1CNT_L = 0x002E;       // Downward sweep
    REG_SOUND1CNT_H = 0xE280;       // Vol 14, decay 2
    REG_SOUND1CNT_X = 0x8440;       // Mid blast frequency
}

void sfx_boss_hurt(void) {
    // Channel 1: Heavy resonant armor clang
    REG_SOUND1CNT_L = 0x0000;       // No sweep
    REG_SOUND1CNT_H = 0xE1C0;       // Duty 75%, fast decay 1, vol 14
    REG_SOUND1CNT_X = 0x8720;       // High metallic clang (~1900Hz)
    // Channel 4: Micro snap
    REG_SOUND4CNT_L = 0x9100;       // Vol 9, decay 1
    REG_SOUND4CNT_H = 0x8010;       // Tap
}

void sfx_boss_defeat(void) {
    // Channel 4: Massive extended cascading explosion
    REG_SOUND4CNT_L = 0xF700;       // Vol 15, decay 7 (long sustained thunder)
    REG_SOUND4CNT_H = 0x8062;       // Ultra low rumbling noise
    // Channel 1: Core breach downward sweep
    REG_SOUND1CNT_L = 0x003F;       // Slow downward sweep
    REG_SOUND1CNT_H = 0xF480;       // Vol 15, decay 4
    REG_SOUND1CNT_X = 0x8280;       // Low sub-bass pitch
}

static u16 s_bgm_timer = 0;
static bool s_bgm_active = false;

void bgm_boss_start(void) {
    s_bgm_timer = 0;
    s_bgm_active = true;
}

void bgm_boss_stop(void) {
    s_bgm_active = false;
    s_bgm_timer = 0;
}

void bgm_boss_tick(bool enraged) {
    if (!s_bgm_active) return;
    s_bgm_timer++;

    u16 period = enraged ? 16 : 32;
    u16 step = s_bgm_timer % period;

    // Driving combat rhythm on GBA PSG hardware channels
    if (step == 0) {
        // Kick beat on Channel 4 + Bass root note on Channel 2
        REG_SOUND4CNT_L = 0x8100;       // Vol 8, decay 1
        REG_SOUND4CNT_H = 0x8060;       // Low kick thud
        REG_SOUND2CNT_L = 0xB240;       // Duty 25%, vol 11, decay 2
        REG_SOUND2CNT_H = 0x8380;       // Low battle bass note (~220Hz)
    } else if (step == (period / 4)) {
        // Offbeat bass
        REG_SOUND2CNT_L = 0x9140;       // Vol 9, decay 1
        REG_SOUND2CNT_H = 0x83C0;       // Minor third (~262Hz)
    } else if (step == (period / 2)) {
        // Snare / Hat beat on Channel 4 + Bass Fifth
        REG_SOUND4CNT_L = 0x6100;       // Vol 6, decay 1
        REG_SOUND4CNT_H = 0x8010;       // Crisp metallic hat
        REG_SOUND2CNT_L = 0xB240;       // Vol 11, decay 2
        REG_SOUND2CNT_H = 0x8400;       // Fifth (~330Hz)
    } else if (step == (period * 3 / 4)) {
        // Syncopated tension note
        REG_SOUND2CNT_L = 0xA140;       // Vol 10, decay 1
        REG_SOUND2CNT_H = enraged ? 0x8480 : 0x83E0; // Higher tension pitch if enraged
    }
}
