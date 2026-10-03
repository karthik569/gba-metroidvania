#include "audio.h"

void sound_init(void) {
    // Turn on sound circuit (Bit 7 = 1)
    REG_SOUNDCNT_X = 0x0080;

    // Full volume left/right, enable PSG channels 1, 2, 4 to both speakers
    REG_SOUNDCNT_L = 0x7777;

    // DirectSound mixing flags (PSG at 100% volume)
    REG_SOUNDCNT_H = 0x0002;
}

// Channel 1: Frequency sweep laser
void sfx_laser(void) {
    // Sweep: time=2, decrease, shift=5
    REG_SOUND1CNT_L = 0x002D;
    // Duty 50%, Initial vol 15, envelope decay 1
    REG_SOUND1CNT_H = 0xF180;
    // Initial freq, restart sound (bit 15)
    REG_SOUND1CNT_X = 0x86D0;
}

// Channel 4: Deep explosion burst
void sfx_missile(void) {
    // Vol 15, decay step 4
    REG_SOUND4CNT_L = 0xF400;
    // Clock divider 2, 7-stage LFSR, restart
    REG_SOUND4CNT_H = 0x8028;
}

// Channel 1: Jump chirp
void sfx_jump(void) {
    // Sweep: upward chirp
    REG_SOUND1CNT_L = 0x0017;
    // Vol 12, decay 2
    REG_SOUND1CNT_H = 0xC280;
    // High note, restart
    REG_SOUND1CNT_X = 0x85E0;
}

// Channel 4: Short crunch noise
void sfx_hit(void) {
    REG_SOUND4CNT_L = 0xB100;
    REG_SOUND4CNT_H = 0x8010;
}

// Channel 1: Powerup chime
void sfx_pickup(void) {
    REG_SOUND1CNT_L = 0x0000;
    REG_SOUND1CNT_H = 0xF780;
    REG_SOUND1CNT_X = 0x87A0;
}

// Channel 4: Low rumble
void sfx_boss_roar(void) {
    REG_SOUND4CNT_L = 0xF700;
    REG_SOUND4CNT_H = 0x8050;
}
