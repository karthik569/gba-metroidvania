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
        input_poll();
        game_update();
        graphics_wait_vblank();
        game_render();
    }

    return 0;
}
