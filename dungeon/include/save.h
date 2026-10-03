#ifndef SAVE_H
#define SAVE_H

#include "gba.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SAVE_MAGIC "AETH100"

typedef struct {
    char magic[8];            // "AETH100\0"
    u8 max_hearts;            // 6..16 (half-hearts: 6 = 3 full hearts)
    u8 current_health;        // Current HP (half-hearts)
    u8 unlocked_tools;        // Bitmask of SubWeaponType
    u8 active_tool;           // Currently selected tool
    u16 gold_coins;           // Player gold currency
    u8 small_keys;            // Current dungeon small keys
    bool has_boss_key;        // Big Boss Key acquired
    bool boss_defeated;       // Golem slain
    u8 current_room;          // Room ID
    u32 room_flags[8];        // 256 bitflags for chests/doors/switches
    u16 checksum;             // Additive 16-bit word verification
} __attribute__((packed)) SaveSlot;

void save_init(void);
bool save_is_slot_valid(u8 slot_idx);
bool save_write_slot(u8 slot_idx, const SaveSlot* slot);
bool save_read_slot(u8 slot_idx, SaveSlot* slot);
void save_erase_slot(u8 slot_idx);

#ifdef __cplusplus
}
#endif

#endif // SAVE_H
