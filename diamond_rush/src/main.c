#include "gba.h"
#include "graphics.h"
#include "input.h"
#include "audio.h"
#include "assets.h"
#include "save.h"
#include "game.h"

int main(void) {
    // 1. Initialize battery SRAM access (8 waitstates)
    save_init();

    // 2. Initialize sound subsystem (PSG Stereo)
    audio_init();

    // 3. Initialize Mode 0 graphics, palettes, tilemaps & sprites
    graphics_init();

    // 5. Initialize game state machine (Title Screen)
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
