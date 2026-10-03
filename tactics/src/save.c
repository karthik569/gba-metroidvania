#include "save.h"
#include "gba.h"

#define SRAM_MAGIC      "IRNT100"
#define SRAM_MAGIC_LEN  7

typedef struct {
    char magic[8];        // "IRNT100\0"
    u8 max_mission;       // Highest unlocked mission (1..3)
    u8 mission_ranks[4];  // Ranks for missions 1..3: 0=none, 1=C, 2=B, 3=A, 4=S
    u8 reserved[2];
    u32 career_score;     // Total tactical score
    u16 checksum;
} __attribute__((packed)) SaveState;

// Tag in ROM so emulators configure 32KB SRAM
__attribute__((used)) static const char s_sram_tag[] = "SRAM_V110";

static u16 calc_chk(const SaveState* s) {
    const u8* bytes = (const u8*)s;
    u32 sum = 0;
    for (u32 i = 0; i < sizeof(SaveState) - sizeof(u16); i++) {
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
        if (out->max_mission < 1) out->max_mission = 1;
        if (out->max_mission > 3) out->max_mission = 3;
        return true;
    }

    return false;
}

u8 save_get_max_mission(void) {
    SaveState s;
    if (read_current_save(&s)) {
        return s.max_mission;
    }
    return 1;
}

void save_set_max_mission(u8 mission) {
    SaveState s;
    if (!read_current_save(&s)) {
        s.max_mission = (mission >= 1 && mission <= 3) ? mission : 1;
        for (int i = 0; i < 4; i++) s.mission_ranks[i] = 0;
        s.career_score = 0;
    } else {
        if (mission > s.max_mission && mission <= 3) {
            s.max_mission = mission;
        }
    }

    for (int i = 0; i < SRAM_MAGIC_LEN; i++) s.magic[i] = SRAM_MAGIC[i];
    s.magic[7] = '\0';
    s.reserved[0] = 0;
    s.reserved[1] = 0;
    s.checksum = calc_chk(&s);
    write_save(&s);
}

u8 save_get_mission_rank(u8 mission) {
    if (mission > 3) return 0;
    SaveState s;
    if (read_current_save(&s)) {
        return s.mission_ranks[mission];
    }
    return 0;
}

void save_set_mission_rank(u8 mission, u8 rank) {
    if (mission > 3) return;
    SaveState s;
    if (!read_current_save(&s)) {
        s.max_mission = 1;
        for (int i = 0; i < 4; i++) s.mission_ranks[i] = 0;
        s.career_score = 0;
    }

    if (rank > s.mission_ranks[mission]) {
        s.mission_ranks[mission] = rank;
    }

    for (int i = 0; i < SRAM_MAGIC_LEN; i++) s.magic[i] = SRAM_MAGIC[i];
    s.magic[7] = '\0';
    s.reserved[0] = 0;
    s.reserved[1] = 0;
    s.checksum = calc_chk(&s);
    write_save(&s);
}

u32 save_get_career_score(void) {
    SaveState s;
    if (read_current_save(&s)) {
        return s.career_score;
    }
    return 0;
}

void save_add_career_score(u32 score) {
    SaveState s;
    if (!read_current_save(&s)) {
        s.max_mission = 1;
        for (int i = 0; i < 4; i++) s.mission_ranks[i] = 0;
        s.career_score = score;
    } else {
        s.career_score += score;
    }

    for (int i = 0; i < SRAM_MAGIC_LEN; i++) s.magic[i] = SRAM_MAGIC[i];
    s.magic[7] = '\0';
    s.reserved[0] = 0;
    s.reserved[1] = 0;
    s.checksum = calc_chk(&s);
    write_save(&s);
}
