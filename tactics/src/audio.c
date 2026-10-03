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

// Player Phase March (32 steps @ 8 frames per 16th note)
static const u16 s_player_lead[32] = {
    NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C6,
    NOTE_G5, NOTE_E5, NOTE_G5, NOTE_C5,
    NOTE_F4, NOTE_A4, NOTE_C5, NOTE_F5,
    NOTE_E5, NOTE_D5, NOTE_C5, REST,
    NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C6,
    NOTE_B5, NOTE_G5, NOTE_A5, NOTE_F5,
    NOTE_G5, NOTE_E5, NOTE_F5, NOTE_D5,
    NOTE_C5, REST,    REST,    REST
};

static const u16 s_player_bass[32] = {
    NOTE_C4, REST, NOTE_C4, REST, NOTE_G4, REST, NOTE_G4, REST,
    NOTE_F4, REST, NOTE_F4, REST, NOTE_G4, REST, NOTE_G4, REST,
    NOTE_C4, REST, NOTE_C4, REST, NOTE_E4, REST, NOTE_F4, REST,
    NOTE_G4, REST, NOTE_G4, REST, NOTE_C4, REST, REST,    REST
};

// Enemy Phase Menacing March (16 steps)
static const u16 s_enemy_lead[16] = {
    NOTE_A4, NOTE_C5, NOTE_D5, NOTE_D4,
    NOTE_A4, NOTE_C5, NOTE_D5, REST,
    NOTE_F4, NOTE_A4, NOTE_C5, NOTE_C4,
    NOTE_E4, NOTE_G4, NOTE_A4, REST
};

// CO Power Supercharge (16 steps)
static const u16 s_co_lead[16] = {
    NOTE_E5, NOTE_G5, NOTE_B5, NOTE_E5,
    NOTE_F5, NOTE_A5, NOTE_C6, NOTE_F5,
    NOTE_G5, NOTE_B5, NOTE_D5, NOTE_G5,
    NOTE_A5, NOTE_C6, NOTE_E5, NOTE_A5
};

void audio_init(void) {
    REG_SOUNDCNT_X = 0x0080;
    REG_SOUNDCNT_L = 0xFF77;
    REG_SOUNDCNT_H = 0x0002;
    REG_SOUNDBIAS   = 0x0200;

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

    // 8 frames per step (~112 BPM)
    if (s_bgm_frame % 8 == 0) {
        u16 step = (s_bgm_frame / 8);

        if (s_current_bgm == BGM_PLAYER_PHASE) {
            u8 idx = step % 32;

            // Ch2 Bassline
            u16 bnote = s_player_bass[idx];
            if (bnote != REST) {
                REG_SOUND2CNT_L = 0x8830;
                REG_SOUND2CNT_H = 0x8000 | (bnote & 0x07FF);
            }

            // Ch1 Melody (if no SFX playing)
            if (s_sfx_lock == 0) {
                u16 lnote = s_player_lead[idx];
                if (lnote != REST) {
                    REG_SOUND1CNT_L = 0;
                    REG_SOUND1CNT_H = 0x9930;
                    REG_SOUND1CNT_X = 0x8000 | (lnote & 0x07FF);
                }
            }

            // Ch4 March Snare
            if ((idx & 1) == 0) {
                REG_SOUND4CNT_L = 0x5120;
                REG_SOUND4CNT_H = 0x8012;
            }
        } else if (s_current_bgm == BGM_ENEMY_PHASE) {
            u8 idx = step % 16;
            if (s_sfx_lock == 0) {
                u16 lnote = s_enemy_lead[idx];
                if (lnote != REST) {
                    REG_SOUND1CNT_L = 0;
                    REG_SOUND1CNT_H = 0x8840;
                    REG_SOUND1CNT_X = 0x8000 | (lnote & 0x07FF);
                }
            }
            REG_SOUND4CNT_L = 0x6130;
            REG_SOUND4CNT_H = 0x8023;
        } else if (s_current_bgm == BGM_CO_POWER) {
            u8 idx = step % 16;
            if (s_sfx_lock == 0) {
                u16 note = s_co_lead[idx];
                REG_SOUND1CNT_L = 0;
                REG_SOUND1CNT_H = 0xB940;
                REG_SOUND1CNT_X = 0x8000 | (note & 0x07FF);
            }
            REG_SOUND4CNT_L = 0x7120;
            REG_SOUND4CNT_H = 0x8011;
        } else if (s_current_bgm == BGM_TITLE) {
            u8 idx = step % 16;
            if (s_sfx_lock == 0 && (idx & 1) == 0) {
                u16 note = s_player_lead[idx];
                if (note != REST) {
                    REG_SOUND1CNT_L = 0;
                    REG_SOUND1CNT_H = 0x8830;
                    REG_SOUND1CNT_X = 0x8000 | (note & 0x07FF);
                }
            }
        }
    }
}

void sfx_cursor_move(void) {
    // Light selection blip
    REG_SOUND2CNT_L = 0x5110;
    REG_SOUND2CNT_H = 0x8000 | NOTE_A5;
}

void sfx_select(void) {
    s_sfx_lock = 4;
    // Confirm chirp
    REG_SOUND1CNT_L = 0x0011;
    REG_SOUND1CNT_H = 0x9220;
    REG_SOUND1CNT_X = 0x8000 | NOTE_C6;
}

void sfx_cancel(void) {
    s_sfx_lock = 4;
    // Cancel buzz
    REG_SOUND1CNT_L = 0x001F;
    REG_SOUND1CNT_H = 0x8220;
    REG_SOUND1CNT_X = 0x8000 | NOTE_D4;
}

void sfx_unit_move(void) {
    // Mech servo hum
    REG_SOUND2CNT_L = 0x6220;
    REG_SOUND2CNT_H = 0x8000 | NOTE_E4;
}

void sfx_cannon_fire(void) {
    s_sfx_lock = 8;
    // Heavy cannon blast: low pitch plunge + noise crunch
    REG_SOUND1CNT_L = 0x003F;
    REG_SOUND1CNT_H = 0xF440;
    REG_SOUND1CNT_X = 0x8000 | NOTE_A4;

    REG_SOUND4CNT_L = 0xE340;
    REG_SOUND4CNT_H = 0x8063;
}

void sfx_autocannon_burst(void) {
    s_sfx_lock = 6;
    // Rapid triple autocannon burst
    REG_SOUND1CNT_L = 0x0012;
    REG_SOUND1CNT_H = 0xB220;
    REG_SOUND1CNT_X = 0x8000 | NOTE_F5;
}

void sfx_rocket_launch(void) {
    s_sfx_lock = 8;
    // Sizzling rocket whoosh
    REG_SOUND4CNT_L = 0xD430;
    REG_SOUND4CNT_H = 0x8045;
}

void sfx_explosion(void) {
    // Low-frequency crunch explosion
    REG_SOUND4CNT_L = 0xF540;
    REG_SOUND4CNT_H = 0x8074;
}

void sfx_capture(void) {
    s_sfx_lock = 10;
    // Terminal capture sequence
    REG_SOUND1CNT_L = 0x0012;
    REG_SOUND1CNT_H = 0xB330;
    REG_SOUND1CNT_X = 0x8000 | NOTE_D5;
}

void sfx_co_power(void) {
    s_sfx_lock = 24;
    // Dramatic supercharge roar
    REG_SOUND1CNT_L = 0x0041; // Rising pitch sweep
    REG_SOUND1CNT_H = 0xF540;
    REG_SOUND1CNT_X = 0x8000 | NOTE_G4;

    REG_SOUND4CNT_L = 0xF540;
    REG_SOUND4CNT_H = 0x8062;
}
