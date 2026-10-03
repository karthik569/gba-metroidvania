#include "save.h"
#include "gba.h"

#define SRAM_MAGIC      "BKBK102"
#define SRAM_LEGACY2    "BKBK101"
#define SRAM_LEGACY     "BKBK100"
#define SRAM_MAGIC_LEN  7

typedef struct {
    char magic[8];        // "BKBK102\0"
    u32 high_score;       // Highest achieved score
    u8 max_stage;         // Highest stage unlocked (1..30)
    u8 zone1_cleared;     // 1 if stage 10 boss defeated
    u8 zone2_cleared;     // 1 if stage 20 boss defeated
    u8 zone3_cleared;     // 1 if stage 30 AI Overlord Core defeated
    u16 checksum;
} __attribute__((packed)) SaveState;

typedef struct {
    char magic[8];        // "BKBK101\0"
    u32 high_score;
    u8 max_stage;         // 1..20
    u8 reserved[3];
    u16 checksum;
} __attribute__((packed)) Legacy101SaveState;

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

static u16 calc_101_chk(const Legacy101SaveState* s) {
    const u8* bytes = (const u8*)s;
    u32 sum = 0;
    for (u32 i = 0; i < sizeof(Legacy101SaveState) - sizeof(u16); i++) {
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

    // Check for BKBK102
    bool magic_match = true;
    for (int i = 0; i < SRAM_MAGIC_LEN; i++) {
        if (out->magic[i] != SRAM_MAGIC[i]) {
            magic_match = false;
            break;
        }
    }

    if (magic_match && out->checksum == calc_chk(out)) {
        if (out->max_stage < 1) out->max_stage = 1;
        if (out->max_stage > 30) out->max_stage = 30;
        return true;
    }

    // Check for legacy "BKBK101" format
    Legacy101SaveState leg101;
    u8* leg101_dst = (u8*)&leg101;
    for (u32 i = 0; i < sizeof(Legacy101SaveState); i++) {
        leg101_dst[i] = src[i];
    }

    bool leg101_match = true;
    for (int i = 0; i < SRAM_MAGIC_LEN; i++) {
        if (leg101.magic[i] != SRAM_LEGACY2[i]) {
            leg101_match = false;
            break;
        }
    }

    if (leg101_match && leg101.checksum == calc_101_chk(&leg101)) {
        out->high_score = leg101.high_score;
        out->max_stage = (leg101.max_stage >= 1 && leg101.max_stage <= 30) ? leg101.max_stage : 1;
        out->zone1_cleared = (out->max_stage > 10) ? 1 : 0;
        out->zone2_cleared = (out->max_stage > 20) ? 1 : 0;
        out->zone3_cleared = 0;
        for (int i = 0; i < SRAM_MAGIC_LEN; i++) {
            out->magic[i] = SRAM_MAGIC[i];
        }
        out->magic[7] = '\0';
        out->checksum = calc_chk(out);
        write_save(out);
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
        out->zone1_cleared = 0;
        out->zone2_cleared = 0;
        out->zone3_cleared = 0;
        for (int i = 0; i < SRAM_MAGIC_LEN; i++) {
            out->magic[i] = SRAM_MAGIC[i];
        }
        out->magic[7] = '\0';
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

bool save_is_zone_cleared(u8 zone) {
    SaveState s;
    if (read_current_save(&s)) {
        if (zone == 1) return s.zone1_cleared != 0;
        if (zone == 2) return s.zone2_cleared != 0;
        if (zone == 3) return s.zone3_cleared != 0;
    }
    return false;
}

void save_set_zone_cleared(u8 zone) {
    SaveState s;
    if (!read_current_save(&s)) {
        s.high_score = 5000;
        s.max_stage = 1;
        s.zone1_cleared = 0;
        s.zone2_cleared = 0;
        s.zone3_cleared = 0;
    }
    if (zone == 1) s.zone1_cleared = 1;
    else if (zone == 2) s.zone2_cleared = 1;
    else if (zone == 3) s.zone3_cleared = 1;

    for (int i = 0; i < SRAM_MAGIC_LEN; i++) {
        s.magic[i] = SRAM_MAGIC[i];
    }
    s.magic[7] = '\0';
    s.checksum = calc_chk(&s);
    write_save(&s);
}

void save_record_progress(u32 score, u8 stage) {
    SaveState s;
    if (!read_current_save(&s)) {
        s.high_score = (score > 5000) ? score : 5000;
        s.max_stage = (stage >= 1 && stage <= 30) ? stage : 1;
        s.zone1_cleared = (s.max_stage > 10) ? 1 : 0;
        s.zone2_cleared = (s.max_stage > 20) ? 1 : 0;
        s.zone3_cleared = 0;
    } else {
        if (score > s.high_score) s.high_score = score;
        if (stage > s.max_stage && stage <= 30) s.max_stage = stage;
        if (stage > 10) s.zone1_cleared = 1;
        if (stage > 20) s.zone2_cleared = 1;
    }

    for (int i = 0; i < SRAM_MAGIC_LEN; i++) {
        s.magic[i] = SRAM_MAGIC[i];
    }
    s.magic[7] = '\0';
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
