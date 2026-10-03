#include "audio.h"

static BgmTrack s_current_bgm = BGM_NONE;
static u16 s_bgm_frame = 0;
static u8  s_sfx_lock = 0;

#define NOTE_B3 1484
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
#define NOTE_D6 1937
#define NOTE_E6 1949
#define NOTE_F6 1955
#define NOTE_G6 1965
#define REST    0

// Centre Court Prelude (32 steps @ 8 frames per note)
static const u16 s_menu_lead[32] = {
    NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C6,
    NOTE_G5, NOTE_E5, NOTE_G5, NOTE_C5,
    NOTE_F4, NOTE_A4, NOTE_C5, NOTE_F5,
    NOTE_E5, NOTE_D5, NOTE_C5, REST,
    NOTE_E5, NOTE_G5, NOTE_A5, NOTE_C6,
    NOTE_D6, NOTE_C6, NOTE_B5, NOTE_G5,
    NOTE_A5, NOTE_F5, NOTE_G5, NOTE_E5,
    NOTE_C5, REST,    REST,    REST
};

static const u16 s_menu_bass[32] = {
    NOTE_C4, REST, NOTE_C4, REST, NOTE_G4, REST, NOTE_G4, REST,
    NOTE_F4, REST, NOTE_F4, REST, NOTE_G4, REST, NOTE_G4, REST,
    NOTE_C4, REST, NOTE_C4, REST, NOTE_F4, REST, NOTE_F4, REST,
    NOTE_G4, REST, NOTE_G4, REST, NOTE_C4, REST, REST,    REST
};

// Championship Rally Beat (32 steps @ 6 frames per note - driving, rhythmic)
static const u16 s_match_lead[32] = {
    NOTE_G4, NOTE_C5, NOTE_E5, NOTE_G5,
    REST,    NOTE_E5, NOTE_C5, REST,
    NOTE_A4, NOTE_D5, NOTE_F5, NOTE_A5,
    REST,    NOTE_F5, NOTE_D5, REST,
    NOTE_G4, NOTE_B4, NOTE_D5, NOTE_G5,
    REST,    NOTE_D5, NOTE_B4, REST,
    NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C6,
    REST,    REST,    REST,    REST
};

static const u16 s_match_bass[32] = {
    NOTE_C4, NOTE_C4, REST, NOTE_C4, NOTE_F4, NOTE_F4, REST, NOTE_F4,
    NOTE_G4, NOTE_G4, REST, NOTE_G4, NOTE_C4, NOTE_C4, REST, NOTE_C4,
    NOTE_C4, NOTE_C4, REST, NOTE_C4, NOTE_F4, NOTE_F4, REST, NOTE_F4,
    NOTE_G4, NOTE_G4, REST, NOTE_G4, NOTE_C4, REST,    REST, REST
};

// Trophy Fanfare (16 steps)
static const u16 s_victory_lead[16] = {
    NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C6,
    REST,    NOTE_G5, NOTE_C6, REST,
    NOTE_D6, NOTE_E6, NOTE_F6, NOTE_G6,
    NOTE_E6, NOTE_D6, NOTE_C6, REST
};

void audio_init(void) {
    REG_SOUNDCNT_X = 0x0080; // Master sound enable
    REG_SOUNDCNT_L = 0xFF77; // Route all 4 channels to Left & Right speakers
    REG_SOUNDCNT_H = 0x0002; // Master volume 100%

    REG_SOUND1CNT_L = 0;
    REG_SOUND1CNT_H = 0;
    REG_SOUND1CNT_X = 0;
    REG_SOUND2CNT_L = 0;
    REG_SOUND2CNT_H = 0;
    REG_SOUND4CNT_L = 0;
    REG_SOUND4CNT_H = 0;

    s_current_bgm = BGM_NONE;
    s_bgm_frame = 0;
    s_sfx_lock = 0;
}

void audio_play_bgm(BgmTrack track) {
    if (s_current_bgm == track) return;
    s_current_bgm = track;
    s_bgm_frame = 0;
}

void audio_stop_bgm(void) {
    s_current_bgm = BGM_NONE;
    s_bgm_frame = 0;
    REG_SOUND1CNT_H = 0;
    REG_SOUND2CNT_H = 0;
}

void audio_update(void) {
    if (s_sfx_lock > 0) s_sfx_lock--;

    if (s_current_bgm == BGM_NONE) return;

    s_bgm_frame++;

    if (s_current_bgm == BGM_MENU) {
        u8 speed = 8;
        u8 step = (s_bgm_frame / speed) % 32;
        u8 sub = s_bgm_frame % speed;

        if (sub == 0) {
            u16 lead = s_menu_lead[step];
            u16 bass = s_menu_bass[step];

            if (lead != REST && s_sfx_lock == 0) {
                REG_SOUND1CNT_L = 0;
                REG_SOUND1CNT_H = 0x8840 | (7 << 12);
                REG_SOUND1CNT_X = 0x8000 | lead;
            }
            if (bass != REST) {
                REG_SOUND2CNT_L = 0x8840 | (5 << 12);
                REG_SOUND2CNT_H = 0x8000 | bass;
            }
        }
    } else if (s_current_bgm == BGM_MATCH) {
        u8 speed = 6;
        u8 step = (s_bgm_frame / speed) % 32;
        u8 sub = s_bgm_frame % speed;

        if (sub == 0) {
            u16 lead = s_match_lead[step];
            u16 bass = s_match_bass[step];

            if (lead != REST && s_sfx_lock == 0) {
                REG_SOUND1CNT_L = 0;
                REG_SOUND1CNT_H = 0x8640 | (6 << 12);
                REG_SOUND1CNT_X = 0x8000 | lead;
            }
            if (bass != REST) {
                REG_SOUND2CNT_L = 0x8640 | (4 << 12);
                REG_SOUND2CNT_H = 0x8000 | bass;
            }
        }
    } else if (s_current_bgm == BGM_VICTORY) {
        u8 speed = 8;
        u8 step = s_bgm_frame / speed;
        if (step < 16) {
            u8 sub = s_bgm_frame % speed;
            if (sub == 0) {
                u16 lead = s_victory_lead[step];
                if (lead != REST) {
                    REG_SOUND1CNT_L = 0;
                    REG_SOUND1CNT_H = 0x8840 | (10 << 12);
                    REG_SOUND1CNT_X = 0x8000 | lead;
                }
            }
        }
    }
}

void sfx_play_racket_topspin(void) {
    s_sfx_lock = 6;
    REG_SOUND1CNT_L = 0x0032;             // Fast downward pitch sweep
    REG_SOUND1CNT_H = 0x4240 | (11 << 12); // High volume punch
    REG_SOUND1CNT_X = 0x8000 | NOTE_A4;
    REG_SOUND4CNT_L = 0x4140 | (8 << 12);
    REG_SOUND4CNT_H = 0x8041;             // Racket string pop
}

void sfx_play_racket_slice(void) {
    s_sfx_lock = 6;
    REG_SOUND1CNT_L = 0x0012;             // Upward pitch bend
    REG_SOUND1CNT_H = 0x2240 | (8 << 12);
    REG_SOUND1CNT_X = 0x8000 | NOTE_C5;
    REG_SOUND4CNT_L = 0x6140 | (6 << 12);
    REG_SOUND4CNT_H = 0x8031;             // Slice whistling swoosh
}

void sfx_play_racket_smash(void) {
    s_sfx_lock = 16;
    REG_SOUND1CNT_L = 0x0055;             // Steep pitch drop
    REG_SOUND1CNT_H = 0x8240 | (14 << 12); // Maximum power
    REG_SOUND1CNT_X = 0x8000 | NOTE_E5;
    REG_SOUND4CNT_L = 0xF240 | (15 << 12); // Massive impact crack
    REG_SOUND4CNT_H = 0x8062;
}

void sfx_play_ball_bounce(CourtSurface surface) {
    u16 note = NOTE_G4;
    if (surface == SURFACE_CLAY) note = NOTE_E4;
    if (surface == SURFACE_HARD) note = NOTE_F4;

    REG_SOUND1CNT_L = 0x0041;
    REG_SOUND1CNT_H = 0x2140 | (8 << 12); // Hollow rubber thump
    REG_SOUND1CNT_X = 0x8000 | note;
}

void sfx_play_net_hit(void) {
    s_sfx_lock = 8;
    REG_SOUND1CNT_L = 0x0014;
    REG_SOUND1CNT_H = 0x2240 | (9 << 12); // Metallic vibration rattle
    REG_SOUND1CNT_X = 0x8000 | NOTE_B3;
}

void sfx_play_umpire_chime(void) {
    s_sfx_lock = 12;
    REG_SOUND1CNT_L = 0;
    REG_SOUND1CNT_H = 0x4240 | (10 << 12);
    REG_SOUND1CNT_X = 0x8000 | NOTE_C5;
    REG_SOUND2CNT_L = 0x4240 | (8 << 12);
    REG_SOUND2CNT_H = 0x8000 | NOTE_G5;
}

void sfx_play_crowd_cheer(void) {
    s_sfx_lock = 24;
    REG_SOUND4CNT_L = 0xE440 | (12 << 12); // Long filtered crowd applause swell
    REG_SOUND4CNT_H = 0x8053;
}
