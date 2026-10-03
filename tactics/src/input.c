#include "input.h"

static u16 s_key_current = 0;
static u16 s_key_previous = 0;

void input_poll(void) {
    s_key_previous = s_key_current;
    // Keys are active low in GBA hardware: 0 is pressed, 1 is released
    s_key_current = (~REG_KEYINPUT) & 0x03FF;
}

bool key_is_down(u16 key) {
    return (s_key_current & key) != 0;
}

bool key_was_pressed(u16 key) {
    return ((s_key_current & key) != 0) && ((s_key_previous & key) == 0);
}

bool key_was_released(u16 key) {
    return ((s_key_current & key) == 0) && ((s_key_previous & key) != 0);
}
