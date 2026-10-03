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
#define NOTE_D6 1937
#define NOTE_E6 1949
#define NOTE_F6 1955
#define NOTE_G6 1965
#define REST    0

// Camp of the Undaunted (32 steps @ 8 frames per note)
static const u16 s_camp_lead[32] = {
    NOTE_E5, NOTE_G5, NOTE_A5, NOTE_C6,
    NOTE_B5, NOTE_G5, NOTE_E5, NOTE_G5,
    NOTE_A5, NOTE_C6, NOTE_D6, NOTE_E6,
    NOTE_D6, NOTE_B5, NOTE_G5, REST,
    NOTE_E5, NOTE_G5, NOTE_A5, NOTE_C6,
    NOTE_D6, NOTE_C6, NOTE_B5, NOTE_A5,
    NOTE_G5, NOTE_E5, NOTE_D5, NOTE_E5,
    NOTE_A4, REST,    REST,    REST
};

static const u16 s_camp_bass[32] = {
    NOTE_A4, REST, NOTE_A4, REST, NOTE_E4, REST, NOTE_E4, REST,
    NOTE_F4, REST, NOTE_F4, REST, NOTE_G4, REST, NOTE_G4, REST,
    NOTE_A4, REST, NOTE_A4, REST, NOTE_D4, REST, NOTE_D4, REST,
    NOTE_E4, REST, NOTE_E4, REST, NOTE_A4, REST, REST,    REST
};

// The Sunken Crypt (32 steps @ 10 frames per note - slow, dark, tense)
static const u16 s_crypt_lead[32] = {
    NOTE_D4, REST, NOTE_F4, REST, NOTE_A4, REST, NOTE_D5, REST,
    NOTE_C5, REST, NOTE_A4, REST, NOTE_F4, REST, NOTE_E4, REST,
    NOTE_D4, REST, NOTE_F4, REST, NOTE_A4, REST, NOTE_C5, REST,
    NOTE_D5, NOTE_C5, NOTE_B4, NOTE_A4, NOTE_G4, NOTE_F4, NOTE_E4, REST
};

static const u16 s_crypt_bass[32] = {
    NOTE_D4, NOTE_D4, REST, REST, NOTE_D4, NOTE_D4, REST, REST,
    NOTE_A4, NOTE_A4, REST, REST, NOTE_A4, NOTE_A4, REST, REST,
    NOTE_D4, NOTE_D4, REST, REST, NOTE_C4, NOTE_C4, REST, REST,
    NOTE_G4, NOTE_G4, REST, REST, NOTE_A4, NOTE_A4, REST, REST
};

// Wrath of the Golem (16 steps @ 6 frames per note - fast, driving combat)
static const u16 s_boss_lead[16] = {
    NOTE_E5, NOTE_E5, NOTE_G5, NOTE_A5,
    NOTE_B5, NOTE_A5, NOTE_G5, NOTE_E5,
    NOTE_F5, NOTE_F5, NOTE_A5, NOTE_C6,
    NOTE_B5, NOTE_G5, NOTE_E5, NOTE_D5
};

static const u16 s_boss_bass[16] = {
    NOTE_E4, REST, NOTE_E4, NOTE_E4,
    NOTE_G4, REST, NOTE_A4, REST,
    NOTE_F4, REST, NOTE_F4, NOTE_F4,
    NOTE_B4, REST, NOTE_E4, REST
};

// Victory Fanfare (16 steps)
static const u16 s_victory_lead[16] = {
    NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C6,
    REST,    NOTE_G5, NOTE_C6, REST,
    NOTE_C6, NOTE_D6, NOTE_E6, NOTE_F6,
    NOTE_E6, NOTE_D6, NOTE_C6, REST
};

void audio_init(void) {
    REG_SOUNDCNT_X = 0x0080; // Master sound enable
    REG_SOUNDCNT_L = 0xFF77; // Route all 4 channels to Left & Right speakers
    REG_SOUNDCNT_H = 0x0002; // Master volume 100%

    // Reset sound channels
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
    if (s_sfx_lock > 0) {
        s_sfx_lock--;
    }

    if (s_current_bgm == BGM_NONE) return;

    s_bgm_frame++;

    if (s_current_bgm == BGM_CAMP) {
        u8 speed = 8;
        u8 step = (s_bgm_frame / speed) % 32;
        u8 sub = s_bgm_frame % speed;

        if (sub == 0) {
            u16 lead = s_camp_lead[step];
            u16 bass = s_camp_bass[step];

            if (lead != REST && s_sfx_lock == 0) {
                REG_SOUND1CNT_L = 0;
                REG_SOUND1CNT_H = 0x8840 | (5 << 12); // Duty 50%, Vol 8, decay
                REG_SOUND1CNT_X = 0x8000 | lead;
            }
            if (bass != REST) {
                REG_SOUND2CNT_L = 0x8840 | (4 << 12); // Duty 50%, Vol 6
                REG_SOUND2CNT_H = 0x8000 | bass;
            }
        }
    } else if (s_current_bgm == BGM_CRYPT) {
        u8 speed = 10;
        u8 step = (s_bgm_frame / speed) % 32;
        u8 sub = s_bgm_frame % speed;

        if (sub == 0) {
            u16 lead = s_crypt_lead[step];
            u16 bass = s_crypt_bass[step];

            if (lead != REST && s_sfx_lock == 0) {
                REG_SOUND1CNT_L = 0;
                REG_SOUND1CNT_H = 0x8A40 | (7 << 12); // Duty 50%, Vol 7
                REG_SOUND1CNT_X = 0x8000 | lead;
            }
            if (bass != REST) {
                REG_SOUND2CNT_L = 0x8840 | (5 << 12); // Duty 50%, Vol 5
                REG_SOUND2CNT_H = 0x8000 | bass;
            }
        }
    } else if (s_current_bgm == BGM_BOSS) {
        u8 speed = 6;
        u8 step = (s_bgm_frame / speed) % 16;
        u8 sub = s_bgm_frame % speed;

        if (sub == 0) {
            u16 lead = s_boss_lead[step];
            u16 bass = s_boss_bass[step];

            if (lead != REST && s_sfx_lock == 0) {
                REG_SOUND1CNT_L = 0;
                REG_SOUND1CNT_H = 0x8640 | (9 << 12); // Duty 50%, Vol 9
                REG_SOUND1CNT_X = 0x8000 | lead;
            }
            if (bass != REST) {
                REG_SOUND2CNT_L = 0x8640 | (7 << 12);
                REG_SOUND2CNT_H = 0x8000 | bass;
            }
            // Driving percussion noise on beats
            if ((step % 2 == 0) && s_sfx_lock == 0) {
                REG_SOUND4CNT_L = 0x6140 | (6 << 12);
                REG_SOUND4CNT_H = 0x8052;
            }
        }
    } else if (s_current_bgm == BGM_VICTORY) {
        u8 speed = 8;
        u8 step = (s_bgm_frame / speed);
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

void sfx_play_sword_swing(void) {
    s_sfx_lock = 6;
    REG_SOUND4CNT_L = 0x4140 | (8 << 12); // Noise envelope decay
    REG_SOUND4CNT_H = 0x8031;             // High-frequency noise whoosh
}

void sfx_play_enemy_hit(void) {
    s_sfx_lock = 8;
    REG_SOUND1CNT_L = 0x0032;             // Fast sweep down
    REG_SOUND1CNT_H = 0x4240 | (11 << 12); // Punchy volume
    REG_SOUND1CNT_X = 0x8000 | NOTE_G4;
    REG_SOUND4CNT_L = 0x4140 | (9 << 12);
    REG_SOUND4CNT_H = 0x8062;             // Impact crack
}

void sfx_play_player_hurt(void) {
    s_sfx_lock = 12;
    REG_SOUND1CNT_L = 0x0055;             // Steep downward pitch sweep
    REG_SOUND1CNT_H = 0x8240 | (12 << 12);
    REG_SOUND1CNT_X = 0x8000 | NOTE_C4;
}

void sfx_play_dodge_roll(void) {
    s_sfx_lock = 6;
    REG_SOUND4CNT_L = 0x6140 | (5 << 12);
    REG_SOUND4CNT_H = 0x8043;             // Soft wind whoosh
}

void sfx_play_boomerang(void) {
    s_sfx_lock = 4;
    REG_SOUND1CNT_L = 0x0012;             // Pitch oscillates
    REG_SOUND1CNT_H = 0x2240 | (6 << 12);
    REG_SOUND1CNT_X = 0x8000 | NOTE_A5;
}

void sfx_play_bomb_drop(void) {
    REG_SOUND1CNT_L = 0;
    REG_SOUND1CNT_H = 0x2140 | (8 << 12);
    REG_SOUND1CNT_X = 0x8000 | NOTE_D4;
}

void sfx_play_bomb_explode(void) {
    s_sfx_lock = 24;
    REG_SOUND4CNT_L = 0xF240 | (15 << 12); // Maximum volume heavy explosion
    REG_SOUND4CNT_H = 0x8073;             // Deep low rumble
}

void sfx_play_fire_wand(void) {
    s_sfx_lock = 8;
    REG_SOUND1CNT_L = 0x0021;             // Fast sweep up
    REG_SOUND1CNT_H = 0x4240 | (9 << 12);
    REG_SOUND1CNT_X = 0x8000 | NOTE_E4;
    REG_SOUND4CNT_L = 0x4140 | (7 << 12);
    REG_SOUND4CNT_H = 0x8052;
}

void sfx_play_door_unlock(void) {
    s_sfx_lock = 16;
    REG_SOUND1CNT_L = 0;
    REG_SOUND1CNT_H = 0x8440 | (10 << 12);
    REG_SOUND1CNT_X = 0x8000 | NOTE_C5;
    REG_SOUND2CNT_L = 0x8440 | (8 << 12);
    REG_SOUND2CNT_H = 0x8000 | NOTE_G5;
}

void sfx_play_chest_open(void) {
    s_sfx_lock = 16;
    REG_SOUND1CNT_L = 0;
    REG_SOUND1CNT_H = 0x8240 | (10 << 12);
    REG_SOUND1CNT_X = 0x8000 | NOTE_E5;
    REG_SOUND2CNT_L = 0x8240 | (8 << 12);
    REG_SOUND2CNT_H = 0x8000 | NOTE_B5;
}

void sfx_play_heart_pickup(void) {
    REG_SOUND1CNT_L = 0;
    REG_SOUND1CNT_H = 0x4140 | (9 << 12);
    REG_SOUND1CNT_X = 0x8000 | NOTE_G5;
}

void sfx_play_puzzle_solve(void) {
    s_sfx_lock = 20;
    // Ascending arpeggio
    REG_SOUND1CNT_L = 0;
    REG_SOUND1CNT_H = 0x6240 | (11 << 12);
    REG_SOUND1CNT_X = 0x8000 | NOTE_C6;
}
