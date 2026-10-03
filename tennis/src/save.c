#include "save.h"

#define SRAM_PTR ((vu8*)SRAM_BASE)

static u16 compute_checksum(const TennisSaveData* data) {
    const u8* bytes = (const u8*)data;
    u32 sum = 0;
    u32 len = sizeof(TennisSaveData) - sizeof(u16);
    for (u32 i = 0; i < len; i++) sum += bytes[i];
    return (u16)(sum & 0xFFFF);
}

void save_init(void) {
    // Enable 8-cycle SRAM waitstates for reliable access
    REG_WAITCNT = (REG_WAITCNT & ~0x0003) | 0x0003;
}

bool save_is_valid(void) {
    TennisSaveData data;
    u8* dst = (u8*)&data;

    for (u32 i = 0; i < sizeof(TennisSaveData); i++) {
        dst[i] = SRAM_PTR[i];
    }

    for (int i = 0; i < 7; i++) {
        if (data.magic[i] != SAVE_MAGIC[i]) return false;
    }

    u16 expected = compute_checksum(&data);
    return (data.checksum == expected);
}

bool save_write(const TennisSaveData* data) {
    TennisSaveData copy = *data;
    for (int i = 0; i < 8; i++) copy.magic[i] = SAVE_MAGIC[i];
    copy.checksum = compute_checksum(&copy);

    const u8* src = (const u8*)&copy;
    for (u32 i = 0; i < sizeof(TennisSaveData); i++) {
        SRAM_PTR[i] = src[i];
    }
    return true;
}

bool save_read(TennisSaveData* data) {
    if (!save_is_valid()) return false;

    u8* dst = (u8*)data;
    for (u32 i = 0; i < sizeof(TennisSaveData); i++) {
        dst[i] = SRAM_PTR[i];
    }
    return true;
}

void save_erase(void) {
    for (u32 i = 0; i < sizeof(TennisSaveData); i++) {
        SRAM_PTR[i] = 0;
    }
}
