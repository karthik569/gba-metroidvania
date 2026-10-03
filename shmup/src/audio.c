#include "audio.h"

static BgmTrack s_current_bgm = BGM_NONE;
static u16 s_bgm_frame = 0;
static u8  s_sfx_lock = 0;

#define NOTE_C4 1546
#define NOTE_D4 1602
#define NOTE_E4 1650
#define NOTE_F4 1673
#define NOTE_G4 1714
#define NOTE_A4 1750
#define NOTE_B4 1783
#define NOTE_C5 1798
#define NOTE_D5 1825
#define NOTE_E5 1849
#define NOTE_F5 1861
#define NOTE_G5 1881
#define NOTE_A5 1899
#define NOTE_B5 1915
#define NOTE_C6 1923
#define REST    0

// Stage 1-3 Driving Synth Pulse (32 steps)
static const u16 s_stage_lead[32] = {
    NOTE_A4, NOTE_C5, NOTE_E5, NOTE_A5,
    NOTE_G5, NOTE_E5, NOTE_D5, NOTE_E5,
    NOTE_F5, NOTE_E5, NOTE_D5, NOTE_C5,
    NOTE_D5, NOTE_E5, NOTE_D5, REST,
    NOTE_A4, NOTE_C5, NOTE_E5, NOTE_A5,
    NOTE_B5, NOTE_C6, NOTE_B5, NOTE_G5,
    NOTE_E5, NOTE_G5, NOTE_A5, NOTE_C6,
    NOTE_B5, NOTE_G5, NOTE_A5, REST
};

static const u16 s_stage_bass[32] = {
    NOTE_A4, REST, NOTE_A4, REST, NOTE_F4, REST, NOTE_F4, REST,
    NOTE_D4, REST, NOTE_D4, REST, NOTE_E4, REST, NOTE_E4, REST,
    NOTE_A4, REST, NOTE_A4, REST, NOTE_G4, REST, NOTE_G4, REST,
    NOTE_F4, REST, NOTE_F4, REST, NOTE_E4, REST, NOTE_G4, REST
};

// Boss Battle Urgent Arpeggio
static const u16 s_boss_lead[16] = {
    NOTE_D5, NOTE_F5, NOTE_A5, NOTE_D5,
    NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C5,
    NOTE_B4, NOTE_D5, NOTE_F5, NOTE_B4,
    NOTE_A4, NOTE_C5, NOTE_E5, NOTE_A5
};

// Title Screen Heroic March
static const u16 s_title_lead[16] = {
    NOTE_A4, REST, NOTE_A4, NOTE_C5,
    NOTE_E5, REST, NOTE_D5, REST,
    NOTE_G4, REST, NOTE_B4, NOTE_D5,
    NOTE_A4, REST, REST,    REST
};

void audio_init(void) {
    // 1. Enable master sound hardware
    REG_SOUNDCNT_X = 0x0080;

    // 2. Route all 4 channels to Left and Right speakers at max volume (0x77)
    REG_SOUNDCNT_L = 0xFF77;

    // 3. Set PSG volume ratio to 100% (bits 0-1 = 2)
    REG_SOUNDCNT_H = 0x0002;

    // 4. Bias to center
    REG_SOUNDBIAS = 0x0200;

    s_current_bgm = BGM_NONE;
    s_bgm_frame = 0;
    s_sfx_lock = 0;
}

void audio_play_bgm(BgmTrack track) {
    s_current_bgm = track;
    s_bgm_frame = 0;
}

void audio_stop_bgm(void) {
    s_current_bgm = BGM_NONE;
    REG_SOUND1CNT_H = 0;
    REG_SOUND2CNT_H = 0;
    REG_SOUND4CNT_H = 0;
}

void audio_update(void) {
    if (s_sfx_lock > 0) s_sfx_lock--;

    if (s_current_bgm == BGM_NONE) return;

    s_bgm_frame++;

    // 7 frames per 16th note (~128 BPM)
    if (s_bgm_frame % 7 == 0) {
        u16 step = (s_bgm_frame / 7);

        if (s_current_bgm == BGM_STAGE) {
            u8 idx = step % 32;

            // Ch2 Bassline
            u16 bnote = s_stage_bass[idx];
            if (bnote != REST) {
                REG_SOUND2CNT_L = 0x8840; // 50% duty, decay envelope
                REG_SOUND2CNT_H = 0x8000 | (bnote & 0x07FF);
            }

            // Ch1 Melody (if no SFX currently active)
            if (s_sfx_lock == 0) {
                u16 lnote = s_stage_lead[idx];
                if (lnote != REST) {
                    REG_SOUND1CNT_L = 0;      // No sweep
                    REG_SOUND1CNT_H = 0x9940; // Volume 9, decay
                    REG_SOUND1CNT_X = 0x8000 | (lnote & 0x07FF);
                }
            }

            // Ch4 Percussion (Hi-hat / Snare rhythm)
            if ((idx & 3) == 0) {
                // Downbeat Kick/Snare
                REG_SOUND4CNT_L = 0x7120; // Fast decay
                REG_SOUND4CNT_H = 0x8013; // Short noise burst
            } else if ((idx & 1) == 0) {
                // Upbeat Hat
                REG_SOUND4CNT_L = 0x3110;
                REG_SOUND4CNT_H = 0x8001;
            }
        } else if (s_current_bgm == BGM_BOSS) {
            u8 idx = step % 16;
            if (s_sfx_lock == 0) {
                u16 note = s_boss_lead[idx];
                REG_SOUND1CNT_L = 0;
                REG_SOUND1CNT_H = 0xA940; // Volume 10
                REG_SOUND1CNT_X = 0x8000 | (note & 0x07FF);
            }
            // Driving bass kick on Channel 4
            REG_SOUND4CNT_L = 0x6120;
            REG_SOUND4CNT_H = 0x8012;
        } else if (s_current_bgm == BGM_TITLE) {
            u8 idx = step % 16;
            if (s_sfx_lock == 0) {
                u16 note = s_title_lead[idx];
                if (note != REST) {
                    REG_SOUND1CNT_L = 0;
                    REG_SOUND1CNT_H = 0x8840;
                    REG_SOUND1CNT_X = 0x8000 | (note & 0x07FF);
                }
            }
        }
    }
}

void sfx_laser_fire(void) {
    s_sfx_lock = 4;
    // Crisp laser shot: Ch1 pitch sweep down
    REG_SOUND1CNT_L = 0x001B; // Sweep time 1, downward sweep step 3
    REG_SOUND1CNT_H = 0xA220; // 50% duty, vol 10, fast decay
    REG_SOUND1CNT_X = 0x8000 | NOTE_C6;
}

void sfx_plasma_wave(void) {
    s_sfx_lock = 6;
    // Rich resonant plasma discharge
    REG_SOUND1CNT_L = 0x002C; // Downward sweep
    REG_SOUND1CNT_H = 0xC320; // Vol 12
    REG_SOUND1CNT_X = 0x8000 | NOTE_E5;
}

void sfx_nova_bomb(void) {
    s_sfx_lock = 16;
    // Multi-channel massive detonation
    REG_SOUND1CNT_L = 0x004F; // Heavy pitch plunge
    REG_SOUND1CNT_H = 0xF540; // Max volume 15
    REG_SOUND1CNT_X = 0x8000 | NOTE_A4;

    REG_SOUND4CNT_L = 0xF540; // Max noise volume, long decay
    REG_SOUND4CNT_H = 0x8075; // Low-frequency rumble
}

void sfx_enemy_explode(void) {
    // Noise channel explosion burst
    REG_SOUND4CNT_L = 0xA230; // Vol 10, decay
    REG_SOUND4CNT_H = 0x8043; // Mid-frequency noise
}

void sfx_boss_explode(void) {
    // Deep heavy crunch
    REG_SOUND4CNT_L = 0xF440;
    REG_SOUND4CNT_H = 0x8074;
}

void sfx_shield_hit(void) {
    // High metallic ping
    REG_SOUND2CNT_L = 0x8110; // Vol 8, short decay
    REG_SOUND2CNT_H = 0x8000 | NOTE_C6;
}

void sfx_shield_break(void) {
    s_sfx_lock = 8;
    // Electric shatter sweep
    REG_SOUND1CNT_L = 0x0017;
    REG_SOUND1CNT_H = 0xB330;
    REG_SOUND1CNT_X = 0x8000 | NOTE_D5;
}

void sfx_powerup_pickup(void) {
    s_sfx_lock = 8;
    // Rising two-tone chime
    REG_SOUND1CNT_L = 0x0012; // Upward sweep
    REG_SOUND1CNT_H = 0xB320;
    REG_SOUND1CNT_X = 0x8000 | NOTE_C5;
}

void sfx_player_death(void) {
    s_sfx_lock = 24;
    REG_SOUND1CNT_L = 0x005F; // Dramatic long downward plunge
    REG_SOUND1CNT_H = 0xF540;
    REG_SOUND1CNT_X = 0x8000 | NOTE_G4;

    REG_SOUND4CNT_L = 0xF440;
    REG_SOUND4CNT_H = 0x8064;
}

void sfx_extra_life(void) {
    s_sfx_lock = 12;
    // Ascending arpeggio
    REG_SOUND1CNT_L = 0x0011;
    REG_SOUND1CNT_H = 0xD420;
    REG_SOUND1CNT_X = 0x8000 | NOTE_E5;
}
