#include "gba.h"
#include "graphics.h"
#include "input.h"
#include "audio.h"
#include "assets.h"
#include "game.h"

int main(void) {
    // 1. Initialize Hardware Subsystems
    graphics_init();
    assets_init();
    audio_init();
    game_init();

    // 2. Main 60 FPS Hardware VBlank Synchronized Game Loop
    while (1) {
        // Poll controller inputs
        input_poll();

        // Advance simulation physics, enemy AI, bullet hell & audio
        game_update();

        // Synchronize with GBA Hardware Vertical Blanking interval (line 160)
        graphics_wait_vblank();

        // Render OAM sprites and update dynamic background tile telemetry
        game_render();
    }

    return 0;
}
