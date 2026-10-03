#include "gba.h"
#include "types.h"
#include "graphics.h"
#include "input.h"
#include "audio.h"
#include "assets.h"
#include "map.h"
#include "entity.h"

int main(void) {
    // Hardware and Engine Subsystems Initialization
    gfx_init();
    sound_init();
    assets_init();
    map_init();
    entities_init();

    // Main 60 FPS Game Loop
    while (1) {
        // Synchronize with VBlank (Frame pacing @ 60 FPS)
        vsync();

        // 1. Poll Hardware Keypad Input
        input_poll();

        // 2. Check Game Over / Respawn condition
        const Player* player = entity_get_player();
        if (player->health <= 0) {
            // Respawn at landing dock
            map_load_room(0);
            entities_init();
        }

        // 3. Update Physics, Collisions, Enemies & Projectiles
        entities_update();

        // 4. Render Sprites to Shadow OAM and commit via DMA3
        entities_render();
    }

    return 0;
}
