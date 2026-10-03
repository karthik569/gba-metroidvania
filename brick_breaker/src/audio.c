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
