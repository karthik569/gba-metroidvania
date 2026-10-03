#include "input.h"

static u16 s_curr_keys = 0x03FF;
static u16 s_prev_keys = 0x03FF;

void input_poll(void) {
    s_prev_keys = s_curr_keys;
    // Keys are active low on GBA (0 = pressed, 1 = released), invert so 1 = pressed
    s_curr_keys = (~REG_KEYINPUT) & 0x03FF;
}

bool key_is_down(u16 key_mask) {
    return (s_curr_keys & key_mask) != 0;
}

bool key_was_pressed(u16 key_mask) {
    return ((s_curr_keys & key_mask) != 0) && ((s_prev_keys & key_mask) == 0);
}

bool key_was_released(u16 key_mask) {
    return ((s_curr_keys & key_mask) == 0) && ((s_prev_keys & key_mask) != 0);
}
