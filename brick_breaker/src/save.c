#include "save.h"
#include "gba.h"

#define SRAM_MAGIC      "BKBK100"
#define SRAM_MAGIC_LEN  7

typedef struct {
    char magic[8];        // "BKBK100\0"
    u32 high_score;       // Highest achieved score
    u16 checksum;
} __attribute__((packed)) SaveState;

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

void save_init(void) {
    REG_WAITCNT |= 0x0003; // Safe 8-cycle SRAM waitstates
}

u32 save_get_high_score(void) {
    SaveState s;
    vu8* src = (vu8*)SRAM_BASE;
    u8* dst = (u8*)&s;

    for (u32 i = 0; i < sizeof(SaveState); i++) {
        dst[i] = src[i];
    }

    for (int i = 0; i < SRAM_MAGIC_LEN; i++) {
        if (s.magic[i] != SRAM_MAGIC[i]) {
            return 5000; // Default arcade factory high score
        }
    }

    if (s.checksum != calc_chk(&s)) {
        return 5000;
    }

    return s.high_score;
}

void save_set_high_score(u32 score) {
    SaveState s;
    for (int i = 0; i < SRAM_MAGIC_LEN; i++) {
        s.magic[i] = SRAM_MAGIC[i];
    }
    s.magic[7] = '\0';
    s.high_score = score;
    s.checksum = calc_chk(&s);

    const u8* src = (const u8*)&s;
    vu8* dst = (vu8*)SRAM_BASE;

    for (u32 i = 0; i < sizeof(SaveState); i++) {
        dst[i] = src[i];
    }
}
