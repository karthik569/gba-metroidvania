#ifndef SAVE_H
#define SAVE_H

#include "types.h"
#include "entity.h"

#define SRAM_SAVE_MAGIC     "AEROSAVE"
#define SRAM_SAVE_MAGIC_LEN 8
#define SRAM_SAVE_BASE      0x0E000000

typedef struct {
    char magic[8];        // "AEROSAVE"
    u16 version;          // Save version
    s16 health;           // Player health
    s16 max_health;       // Max health
    s16 missiles;         // Missile ammo
    s16 max_missiles;     // Max missiles
    u8 has_missiles;      // Upgrade pod acquired flag
    u8 checkpoint_room;   // Checkpoint room ID (0..6)
    s16 checkpoint_x;     // Spawn X coordinate
    s16 checkpoint_y;     // Spawn Y coordinate
    u32 playtime_frames;  // Playtime frame counter
    u16 checksum;         // Validation checksum
} __attribute__((packed)) SaveData;

#ifdef __cplusplus
extern "C" {
#endif

// Check if valid persistent save data exists in SRAM
bool save_exists(void);

// Save current player progress and checkpoint coordinates to SRAM
bool save_game(const Player* player, u8 room_id, s16 px, s16 py);

// Load persistent game state from SRAM
bool load_game(Player* player, u8* room_id, s16* px, s16* py);

// Clear / invalidate save data in SRAM
void save_clear(void);

#ifdef __cplusplus
}
#endif

#endif // SAVE_H
