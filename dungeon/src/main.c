#include "gba.h"
#include "graphics.h"
#include "input.h"
#include "audio.h"
#include "assets.h"
#include "save.h"
#include "game.h"

int main(void) {
    // 1. Initialize battery SRAM access
    save_init();

    // 2. Initialize sound subsystem (PSG Stereo)
    audio_init();

    // 3. Initialize Mode 0 graphics & tile layers
    graphics_init();

    // 4. Upload palettes, font, dungeon tiles, and sprites
    assets_load_palettes();
    assets_load_tiles();

    // 5. Initialize game state machine & start in Camp
    game_init();

    // 6. 60 FPS hardware synchronized game loop
    while (1) {
        graphics_wait_vblank();
        input_poll();
        game_update();
        game_draw();
    }

    return 0;
}
