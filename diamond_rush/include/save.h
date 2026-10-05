#ifndef SAVE_H
#define SAVE_H

#include "types.h"
#include "gba.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SAVE_MAGIC "DMRUSH1"
#define MAX_LEVELS 4

typedef struct {
    char magic[8];         // "DMRUSH1\0"
    u8   max_level_unlocked; // 1..MAX_LEVELS
    u8   level_stars[MAX_LEVELS]; // 0..3 stars per level
    u8   reserved[3];
    u32  high_score;       // Career best score
    u32  total_diamonds;   // Lifetime diamonds collected
    u16  checksum;         // 16-bit integrity checksum
} __attribute__((packed, aligned(4))) SaveSlot;

void save_init(void);
bool save_is_slot_valid(u8 slot_idx);
bool save_read_slot(u8 slot_idx, SaveSlot* slot);
bool save_write_slot(u8 slot_idx, const SaveSlot* slot);
void save_erase_slot(u8 slot_idx);

#ifdef __cplusplus
}
#endif

#endif // SAVE_H
