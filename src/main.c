#include "gba.h"
#include "types.h"
#include "graphics.h"
#include "input.h"
#include "audio.h"
#include "assets.h"
#include "map.h"
#include "entity.h"
#include "logger.h"
#include "save.h"

int main(void) {
    // 1. Initialize Logging System (mGBA port handshake + SRAM ring buffer)
    log_init();

    // 2. Hardware and Engine Subsystems Initialization
    gfx_init();
    sound_init();
    assets_init();
    map_init();

    LOG_INFO("BOOT", "=== AERO-VOID: Outpost Zero ===");
    LOG_INFO("BOOT", "GBA Architecture: ARM7TDMI @ 16.78MHz (Mode 0, 1D OBJ mapping, PSG Sound)");
    LOG_INFO("BOOT", "Debug Host Interface: %s", log_is_mgba_active() ? "mGBA Live Port Active" : "Cartridge SRAM Active");

    // 3. Check for existing game save in SRAM slot 0
    Player saved_player;
    u8 saved_room = 0;
    s16 saved_x = 32, saved_y = 112;
    if (load_game(&saved_player, &saved_room, &saved_x, &saved_y)) {
        LOG_INFO("BOOT", "Restoring mission from SRAM save: Room %d (%s) @ (%d, %d)",
                 saved_room, map_get_room_name(saved_room), saved_x, saved_y);
        entity_restore_save(&saved_player, saved_room, saved_x, saved_y);
    } else {
        LOG_INFO("BOOT", "No SRAM save found. Deploying to Room 0 (Landing Dock)");
        entities_init();
    }

    // 4. Main 60 FPS Game Loop
    while (1) {
        // Synchronize with VBlank (Frame pacing @ 60 FPS)
        vsync();

        // Advance global engine frame clock for telemetry
        log_tick();

        // Poll Hardware Keypad Input
        input_poll();

        // Check Player Defeat / Respawn Condition
        const Player* player = entity_get_player();
        if (player->health <= 0) {
            LOG_WARN("GAME", "Mission Critical: Player health depleted! Reloading from checkpoint");
            entity_respawn();
        }

        // Update Physics, Collisions, Enemies, Weapons & Boss
        entities_update();

        // Render Sprites & HUD to Shadow OAM and commit via DMA3
        entities_render();
    }

    return 0;
}
