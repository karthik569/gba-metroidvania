#include "save.h"
#include "gba.h"

#define SRAM_MAGIC      "BKBK101"
#define SRAM_LEGACY     "BKBK100"
#define SRAM_MAGIC_LEN  7

typedef struct {
    char magic[8];        // "BKBK101\0"
    u32 high_score;       // Highest achieved score
    u8 max_stage;         // Highest stage unlocked (1..20)
    u8 reserved[3];       // Word alignment padding
    u16 checksum;
} __attribute__((packed)) SaveState;

typedef struct {
    char magic[8];        // "BKBK100\0"
    u32 high_score;
    u16 checksum;
} __attribute__((packed)) LegacySaveState;

// Static string in ROM so emulators detect 32KB SRAM
__attribute__((used)) static const char s_sram_tag[] = "SRAM_V110";

static u16 calc_chk(const SaveState* s) {
    const u8* bytes = (const u8*)s;
    u32 sum = 0;
    for (u32 i = 0; i < sizeof(SaveState) - sizeof(u16); i++) {
        sum = (sum + bytes[i]) & 0xFFFF;
    }
    return (u16)sum;
}

static u16 calc_legacy_chk(const LegacySaveState* s) {
    const u8* bytes = (const u8*)s;
    u32 sum = 0;
    for (u32 i = 0; i < sizeof(LegacySaveState) - sizeof(u16); i++) {
        sum = (sum + bytes[i]) & 0xFFFF;
    }
    return (u16)sum;
}

void save_init(void) {
    REG_WAITCNT |= 0x0003; // Safe 8-cycle SRAM waitstates
}

static void write_save(const SaveState* s) {
    const u8* src = (const u8*)s;
    vu8* dst = (vu8*)SRAM_BASE;

    for (u32 i = 0; i < sizeof(SaveState); i++) {
        dst[i] = src[i];
    }
}

static bool read_current_save(SaveState* out) {
    vu8* src = (vu8*)SRAM_BASE;
    u8* dst = (u8*)out;

    for (u32 i = 0; i < sizeof(SaveState); i++) {
        dst[i] = src[i];
    }

    bool magic_match = true;
    for (int i = 0; i < SRAM_MAGIC_LEN; i++) {
        if (out->magic[i] != SRAM_MAGIC[i]) {
            magic_match = false;
            break;
        }
    }

    if (magic_match && out->checksum == calc_chk(out)) {
        if (out->max_stage < 1) out->max_stage = 1;
        if (out->max_stage > 20) out->max_stage = 20;
        return true;
    }

    // Check for legacy "BKBK100" format
    LegacySaveState leg;
    u8* leg_dst = (u8*)&leg;
    for (u32 i = 0; i < sizeof(LegacySaveState); i++) {
        leg_dst[i] = src[i];
    }

    bool leg_match = true;
    for (int i = 0; i < SRAM_MAGIC_LEN; i++) {
        if (leg.magic[i] != SRAM_LEGACY[i]) {
            leg_match = false;
            break;
        }
    }

    if (leg_match && leg.checksum == calc_legacy_chk(&leg)) {
        out->high_score = leg.high_score;
        out->max_stage = 1;
        for (int i = 0; i < SRAM_MAGIC_LEN; i++) {
            out->magic[i] = SRAM_MAGIC[i];
        }
        out->magic[7] = '\0';
        out->reserved[0] = 0;
        out->reserved[1] = 0;
        out->reserved[2] = 0;
        out->checksum = calc_chk(out);
        write_save(out);
        return true;
    }

    return false;
}

u32 save_get_high_score(void) {
    SaveState s;
    if (read_current_save(&s)) {
        return s.high_score;
    }
    return 5000;
}

u8 save_get_max_stage(void) {
    SaveState s;
    if (read_current_save(&s)) {
        return s.max_stage;
    }
    return 1;
}

void save_record_progress(u32 score, u8 stage) {
    SaveState s;
    if (!read_current_save(&s)) {
        s.high_score = (score > 5000) ? score : 5000;
        s.max_stage = (stage >= 1 && stage <= 20) ? stage : 1;
    } else {
        if (score > s.high_score) s.high_score = score;
        if (stage > s.max_stage && stage <= 20) s.max_stage = stage;
    }

    for (int i = 0; i < SRAM_MAGIC_LEN; i++) {
        s.magic[i] = SRAM_MAGIC[i];
    }
    s.magic[7] = '\0';
    s.reserved[0] = 0;
    s.reserved[1] = 0;
    s.reserved[2] = 0;
    s.checksum = calc_chk(&s);

    write_save(&s);
}

void save_set_high_score(u32 score) {
    u8 stage = save_get_max_stage();
    save_record_progress(score, stage);
}

void save_set_max_stage(u8 stage) {
    u32 score = save_get_high_score();
    save_record_progress(score, stage);
}
