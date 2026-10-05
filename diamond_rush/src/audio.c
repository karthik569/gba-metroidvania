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

// Title Theme (16 steps @ 8 frames per step)
static const u16 s_title_lead[16] = {
    NOTE_E5, NOTE_G5, NOTE_A5, NOTE_B5,
    NOTE_C6, NOTE_B5, NOTE_A5, NOTE_G5,
    NOTE_E5, NOTE_A5, NOTE_G5, NOTE_E5,
    NOTE_D5, NOTE_E5, REST,    REST
};

static const u16 s_title_bass[16] = {
    NOTE_A4, REST, NOTE_A4, REST,
    NOTE_F4, REST, NOTE_G4, REST,
    NOTE_A4, REST, NOTE_E4, REST,
    NOTE_F4, REST, NOTE_E4, REST
};

// Expedition Theme (32 steps @ 7 frames per step - bouncy, rhythmic, adventurous)
static const u16 s_expedition_lead[32] = {
    NOTE_A4, NOTE_C5, NOTE_E5, NOTE_A5,
    NOTE_G5, NOTE_E5, NOTE_D5, NOTE_E5,
    NOTE_F5, NOTE_E5, NOTE_D5, NOTE_C5,
    NOTE_D5, NOTE_E5, REST,    REST,
    NOTE_A4, NOTE_C5, NOTE_E5, NOTE_A5,
    NOTE_B5, NOTE_C6, NOTE_B5, NOTE_A5,
    NOTE_G5, NOTE_A5, NOTE_F5, NOTE_E5,
    NOTE_D5, NOTE_E5, NOTE_A4, REST
};

static const u16 s_expedition_bass[32] = {
    NOTE_A4, REST, NOTE_A4, NOTE_E4,
    NOTE_G4, REST, NOTE_A4, REST,
    NOTE_D4, REST, NOTE_F4, REST,
    NOTE_G4, REST, NOTE_E4, REST,
    NOTE_A4, REST, NOTE_A4, NOTE_E4,
    NOTE_F4, REST, NOTE_G4, REST,
    NOTE_C4, REST, NOTE_D4, REST,
    NOTE_E4, REST, NOTE_A4, REST
};

// Victory Fanfare (16 steps @ 6 frames)
static const u16 s_victory_lead[16] = {
    NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C6,
    REST,    NOTE_G5, NOTE_C6, REST,
    NOTE_C6, NOTE_D6, NOTE_E6, NOTE_F6,
    NOTE_E6, NOTE_D6, NOTE_C6, REST
};

// Game Over Downfall (16 steps @ 8 frames)
static const u16 s_gameover_lead[16] = {
    NOTE_E5, NOTE_D5, NOTE_C5, NOTE_B4,
    NOTE_A4, REST,    NOTE_G4, REST,
    NOTE_F4, REST,    NOTE_E4, REST,
    NOTE_D4, REST,    NOTE_C4, REST
};

void audio_init(void) {
    REG_SOUNDCNT_X = 0x0080; // Master sound enable
    REG_SOUNDCNT_L = 0xFF77; // Route all channels to Left & Right speakers
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
    REG_SOUND1CNT_X = 0;
    REG_SOUND2CNT_H = 0;
    REG_SOUND4CNT_H = 0;
}

void audio_update(void) {
    if (s_sfx_lock > 0) {
        s_sfx_lock--;
    }

    if (s_current_bgm == BGM_NONE) return;

    u8 step_len = 7;
    u8 total_steps = 32;
    const u16* lead = s_expedition_lead;
    const u16* bass = s_expedition_bass;

    if (s_current_bgm == BGM_TITLE) {
        step_len = 8;
        total_steps = 16;
        lead = s_title_lead;
        bass = s_title_bass;
    } else if (s_current_bgm == BGM_VICTORY) {
        step_len = 6;
        total_steps = 16;
        lead = s_victory_lead;
        bass = s_title_bass;
    } else if (s_current_bgm == BGM_GAME_OVER) {
        step_len = 8;
        total_steps = 16;
        lead = s_gameover_lead;
        bass = s_title_bass;
    }

    u16 step = (s_bgm_frame / step_len) % total_steps;
    u8 sub_frame = s_bgm_frame % step_len;

    if (sub_frame == 0) {
        u16 note_lead = lead[step];
        if (note_lead != REST && s_sfx_lock == 0) {
            // Pulse wave channel 1: 50% duty, decay envelope
            REG_SOUND1CNT_H = 0xB880; 
            REG_SOUND1CNT_X = 0x8000 | note_lead;
        }

        u16 note_bass = bass[step];
        if (note_bass != REST) {
            // Channel 2 bass: 25% duty, short decay
            REG_SOUND2CNT_L = 0x8840;
            REG_SOUND2CNT_H = 0x8000 | note_bass;
        }

        // Channel 4 percussion on beats 2 and 4 of expedition
        if (s_current_bgm == BGM_EXPEDITION && (step % 4 == 2)) {
            REG_SOUND4CNT_L = 0x7200; // Medium noise envelope
            REG_SOUND4CNT_H = 0x8052; // Crisp snare hit
        }
    }

    s_bgm_frame++;
    // Loop track if completed
    if (s_bgm_frame >= (u16)(total_steps * step_len)) {
        if (s_current_bgm == BGM_VICTORY || s_current_bgm == BGM_GAME_OVER) {
            // Play victory/gameover once, then stop
            s_current_bgm = BGM_NONE;
            return;
        }
        s_bgm_frame = 0;
    }
}

// SFX: Digging / Rustling foliage
void sfx_play_dig_dirt(void) {
    s_sfx_lock = 4;
    REG_SOUND4CNT_L = 0x6100; // Fast decay noise
    REG_SOUND4CNT_H = 0x8033; // Soft rustle
}

// SFX: Diamond Sparkle Chime (high dual tone)
void sfx_play_gem_pickup(void) {
    s_sfx_lock = 8;
    REG_SOUND1CNT_L = 0x0000;
    REG_SOUND1CNT_H = 0xF240; // 50% duty, quick envelope
    REG_SOUND1CNT_X = 0x8000 | 1920; // High C6 sparkle
}

// SFX: Boulder Push Grind
void sfx_play_boulder_push(void) {
    s_sfx_lock = 6;
    REG_SOUND1CNT_L = 0x0022; // Slight downward pitch sweep
    REG_SOUND1CNT_H = 0x9380;
    REG_SOUND1CNT_X = 0x8000 | 1450; // Low gritty rumble
}

// SFX: Boulder Land Thud
void sfx_play_boulder_land(void) {
    s_sfx_lock = 8;
    REG_SOUND4CNT_L = 0xB400; // Heavy impact noise
    REG_SOUND4CNT_H = 0x8070; // Low frequency thud
}

// SFX: Boulder Rolling Rumble
void sfx_play_boulder_roll(void) {
    s_sfx_lock = 5;
    REG_SOUND4CNT_L = 0x5100;
    REG_SOUND4CNT_H = 0x8062;
}

// SFX: Snake Hiss
void sfx_play_snake_hiss(void) {
    s_sfx_lock = 10;
    REG_SOUND4CNT_L = 0x8200;
    REG_SOUND4CNT_H = 0x8015; // High hissing noise
}

// SFX: Snake Squashed by Boulder
void sfx_play_snake_squash(void) {
    s_sfx_lock = 12;
    // Low crunch noise + comical pitch down
    REG_SOUND4CNT_L = 0xC300;
    REG_SOUND4CNT_H = 0x8061;
    REG_SOUND1CNT_L = 0x0044; // Pitch sweep down
    REG_SOUND1CNT_H = 0xE200;
    REG_SOUND1CNT_X = 0x8000 | 1600;
}

// SFX: Key Pickup Fanfare
void sfx_play_key_pickup(void) {
    s_sfx_lock = 10;
    REG_SOUND1CNT_L = 0x0012; // Upward pitch sweep
    REG_SOUND1CNT_H = 0xE180;
    REG_SOUND1CNT_X = 0x8000 | 1850;
}

// SFX: Locked Door Opening
void sfx_play_door_open(void) {
    s_sfx_lock = 14;
    REG_SOUND1CNT_L = 0x0000;
    REG_SOUND1CNT_H = 0xD480;
    REG_SOUND1CNT_X = 0x8000 | 1500;
    REG_SOUND4CNT_L = 0x7300;
    REG_SOUND4CNT_H = 0x8042;
}

// SFX: Treasure Chest Open
void sfx_play_chest_open(void) {
    s_sfx_lock = 12;
    REG_SOUND1CNT_L = 0x0015; // Fast upward chirp
    REG_SOUND1CNT_H = 0xF180;
    REG_SOUND1CNT_X = 0x8000 | 1750;
}

// SFX: Player Hurt Grunt
void sfx_play_player_hurt(void) {
    s_sfx_lock = 10;
    REG_SOUND1CNT_L = 0x0064; // Downward buzz
    REG_SOUND1CNT_H = 0xF440;
    REG_SOUND1CNT_X = 0x8000 | 1550;
}

// SFX: Stone Pressure Plate Click
void sfx_play_plate_click(void) {
    s_sfx_lock = 4;
    REG_SOUND1CNT_L = 0x0000;
    REG_SOUND1CNT_H = 0xA100;
    REG_SOUND1CNT_X = 0x8000 | 1900;
}

// SFX: Exit Portal Open
void sfx_play_exit_open(void) {
    s_sfx_lock = 14;
    REG_SOUND1CNT_L = 0x0011; // Shimmering rising arpeggio
    REG_SOUND1CNT_H = 0xE2C0;
    REG_SOUND1CNT_X = 0x8000 | 1800;
}
