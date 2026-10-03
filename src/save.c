#include "save.h"
#include "logger.h"
#include "gba.h"

static u16 compute_checksum(const SaveData* data) {
    const u8* bytes = (const u8*)data;
    u32 sum = 0;
    u32 len = sizeof(SaveData) - sizeof(u16); // All bytes except checksum itself
    for (u32 i = 0; i < len; i++) {
        sum = (sum + bytes[i]) & 0xFFFF;
    }
    return (u16)sum;
}

static void sram_read_bytes(u32 sram_addr, u8* dst, u32 len) {
    vu8* src = (vu8*)sram_addr;
    for (u32 i = 0; i < len; i++) {
        dst[i] = src[i];
    }
}

static void sram_write_bytes(u32 sram_addr, const u8* src, u32 len) {
    vu8* dst = (vu8*)sram_addr;
    for (u32 i = 0; i < len; i++) {
        dst[i] = src[i];
    }
}

bool save_exists(void) {
    SaveData data;
    sram_read_bytes(SRAM_SAVE_BASE, (u8*)&data, sizeof(SaveData));

    for (int i = 0; i < SRAM_SAVE_MAGIC_LEN; i++) {
        if (data.magic[i] != SRAM_SAVE_MAGIC[i]) {
            return false;
        }
    }

    u16 expected_chk = compute_checksum(&data);
    return (data.checksum == expected_chk);
}

bool save_game(const Player* player, u8 room_id, s16 px, s16 py) {
    if (!player) return false;

    SaveData data;
    for (int i = 0; i < SRAM_SAVE_MAGIC_LEN; i++) {
        data.magic[i] = SRAM_SAVE_MAGIC[i];
    }
    data.version          = 1;
    data.health           = player->health;
    data.max_health       = player->max_health;
    data.missiles         = player->missiles;
    data.max_missiles     = player->max_missiles;
    data.has_missiles     = player->has_missile_upgrade ? 1 : 0;
    data.checkpoint_room  = room_id;
    data.checkpoint_x     = px;
    data.checkpoint_y     = py;
    data.playtime_frames  = log_get_frame();
    data.checksum         = compute_checksum(&data);

    sram_write_bytes(SRAM_SAVE_BASE, (const u8*)&data, sizeof(SaveData));

    LOG_INFO("SAVE", "Game saved to SRAM slot 0: Room %d @ (%d, %d), HP: %d/%d, Missiles: %d/%d",
             room_id, px, py, player->health, player->max_health, player->missiles, player->max_missiles);

    return true;
}

bool load_game(Player* player, u8* room_id, s16* px, s16* py) {
    if (!player || !room_id || !px || !py) return false;
    if (!save_exists()) return false;

    SaveData data;
    sram_read_bytes(SRAM_SAVE_BASE, (u8*)&data, sizeof(SaveData));

    player->health              = data.health;
    player->max_health          = data.max_health;
    player->missiles            = data.missiles;
    player->max_missiles        = data.max_missiles;
    player->has_missile_upgrade = (data.has_missiles != 0);

    *room_id = data.checkpoint_room;
    *px      = data.checkpoint_x;
    *py      = data.checkpoint_y;

    LOG_INFO("SAVE", "Save loaded from SRAM: Room %d @ (%d, %d), HP: %d, Missiles: %d (Playtime: %lu frames)",
             *room_id, *px, *py, player->health, player->missiles, (unsigned long)data.playtime_frames);

    return true;
}

void save_clear(void) {
    vu8* dst = (vu8*)SRAM_SAVE_BASE;
    for (int i = 0; i < SRAM_SAVE_MAGIC_LEN; i++) {
        dst[i] = 0;
    }
    LOG_WARN("SAVE", "Save slot 0 cleared in SRAM");
}
