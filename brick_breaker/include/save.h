#ifndef SAVE_H
#define SAVE_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

void save_init(void);
u32 save_get_high_score(void);
void save_set_high_score(u32 score);

#ifdef __cplusplus
}
#endif

#endif // SAVE_H
