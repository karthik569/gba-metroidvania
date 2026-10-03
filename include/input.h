#ifndef INPUT_H
#define INPUT_H

#include "gba.h"

#ifdef __cplusplus
extern "C" {
#endif

void input_poll(void);
bool key_is_down(u16 key_mask);
bool key_was_pressed(u16 key_mask);
bool key_was_released(u16 key_mask);

#ifdef __cplusplus
}
#endif

#endif // INPUT_H
