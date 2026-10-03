#include "audio.h"

void sound_init(void) {
    // Master sound enable
    REG_SOUNDCNT_X = 0x0080;

    // Set master volume for left and right outputs to maximum (7)
    REG_SOUNDCNT_L = 0x0077;

    // Enable Channel 1, 2, and 4 in DMG mix
    REG_SOUNDCNT_H = 0x000F;
}

void sfx_bounce(void) {
    // Channel 1: Short pleasant upward square ping
    REG_SOUND1CNT_L = 0x0014;       // Sweep time 1, increase, shift 4
    REG_SOUND1CNT_H = 0xF140;       // Duty 50%, envelope decay 1, initial vol 15
    REG_SOUND1CNT_X = 0x8680;       // Initial frequency ~1600Hz, trigger
}

void sfx_brick_hit(void) {
    // Channel 2: Crisp punchy brick pop
    REG_SOUND2CNT_L = 0xE180;       // Duty 50%, envelope decay 1, vol 14
    REG_SOUND2CNT_H = 0x8540;       // Frequency ~1300Hz, trigger
}

void sfx_brick_hard(void) {
    // Channel 1: High metallic clink
    REG_SOUND1CNT_L = 0x0000;       // No sweep
    REG_SOUND1CNT_H = 0xF180;       // Duty 75%, fast decay, max vol
    REG_SOUND1CNT_X = 0x87C0;       // Very high pitch ~1950Hz, trigger
}

void sfx_laser(void) {
    // Channel 1: Arcade laser zap (fast downward sweep)
    REG_SOUND1CNT_L = 0x001C;       // Sweep decrease, shift 4
    REG_SOUND1CNT_H = 0xF240;       // Initial vol 15, decay 2
    REG_SOUND1CNT_X = 0x8780;       // Start high, trigger
}

void sfx_powerup_drop(void) {
    // Channel 2: Gentle chime
    REG_SOUND2CNT_L = 0x8240;       // Medium vol, decay 2
    REG_SOUND2CNT_H = 0x8620;       // Trigger
}

void sfx_powerup_get(void) {
    // Channel 1: Rising fanfare chirp
    REG_SOUND1CNT_L = 0x0022;       // Sweep time 2, upward, shift 2
    REG_SOUND1CNT_H = 0xF280;       // Duty 50%, vol 15
    REG_SOUND1CNT_X = 0x8500;       // Trigger
}

void sfx_life_lost(void) {
    // Channel 4: Low white noise crunch
    REG_SOUND4CNT_L = 0xF400;       // Vol 15, decay 4
    REG_SOUND4CNT_H = 0x8073;       // Low rumble frequency, 7-stage, trigger
}

void sfx_stage_clear(void) {
    // Channel 1: Victory chime
    REG_SOUND1CNT_L = 0x0013;       // Fast upward sweep
    REG_SOUND1CNT_H = 0xF380;       // High duty, sustained
    REG_SOUND1CNT_X = 0x86C0;       // Trigger
}
