#include "save.h"
#include <stddef.h>

#define SRAM_PTR ((vu8*)SRAM_BASE)
#define SLOT_OFFSET(idx) ((idx) * sizeof(SaveSlot))

static u16 compute_checksum(const SaveSlot* slot) {
    const u8* data = (const u8*)slot;
    u32 sum = 0;
    u32 len = offsetof(SaveSlot, checksum);  // Exclude checksum field and any trailing padding
    for (u32 i = 0; i < len; i++) {
        sum += data[i];
    }
    return (u16)(sum & 0xFFFF);
}

void save_init(void) {
    // Enable 8-cycle SRAM waitstates for reliable read/write on hardware and emulators
    REG_WAITCNT = (REG_WAITCNT & ~0x0003) | 0x0003;
}

bool save_is_slot_valid(u8 slot_idx) {
    if (slot_idx >= 3) return false;

    SaveSlot slot;
    vu8* src = SRAM_PTR + SLOT_OFFSET(slot_idx);
    u8* dst = (u8*)&slot;

    for (u32 i = 0; i < sizeof(SaveSlot); i++) {
        dst[i] = src[i];
    }

    for (int i = 0; i < 7; i++) {
        if (slot.magic[i] != SAVE_MAGIC[i]) return false;
    }

    u16 expected = compute_checksum(&slot);
    return (slot.checksum == expected);
}

bool save_read_slot(u8 slot_idx, SaveSlot* slot) {
    if (!save_is_slot_valid(slot_idx)) return false;

    vu8* src = SRAM_PTR + SLOT_OFFSET(slot_idx);
    u8* dst = (u8*)slot;

    for (u32 i = 0; i < sizeof(SaveSlot); i++) {
        dst[i] = src[i];
    }

    return true;
}

bool save_write_slot(u8 slot_idx, const SaveSlot* slot) {
    if (slot_idx >= 3) return false;

    SaveSlot copy = *slot;
    for (int i = 0; i < 8; i++) {
        copy.magic[i] = SAVE_MAGIC[i];
    }
    copy.checksum = compute_checksum(&copy);

    vu8* dst = SRAM_PTR + SLOT_OFFSET(slot_idx);
    const u8* src = (const u8*)&copy;

    for (u32 i = 0; i < sizeof(SaveSlot); i++) {
        dst[i] = src[i];
    }

    return true;
}

void save_erase_slot(u8 slot_idx) {
    if (slot_idx >= 3) return;

    vu8* dst = SRAM_PTR + SLOT_OFFSET(slot_idx);
    for (u32 i = 0; i < sizeof(SaveSlot); i++) {
        dst[i] = 0xFF;
    }
}
