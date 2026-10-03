#include "gba.h"
#include "graphics.h"
#include "input.h"
#include "audio.h"
#include "assets.h"
#include "game.h"

int main(void) {
    // 1. Initialize GBA hardware subsystems
    gfx_init();
    sound_init();
    assets_init();
    game_init();

    // 2. Hardware-locked 60 FPS Game Loop
    while (1) {
        // Synchronize with VBlank (Vertical blank interrupt line @ 60 FPS)
        vsync();

        // Poll physical gamepad keys
        input_poll();

        // Update physics, ball collisions, power-ups, and game state
        game_update();

        // Render playfield, paddle, balls, capsules, and sidebar HUD
        game_render();
    }

    return 0;
}
