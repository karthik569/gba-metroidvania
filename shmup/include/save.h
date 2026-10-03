#ifndef SAVE_H
#define SAVE_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

void save_init(void);
u32  save_get_high_score(void);
void save_set_high_score(u32 score);
u8   save_get_max_stage(void);
void save_set_max_stage(u8 stage);
void save_record_progress(u32 score, u8 stage);

#ifdef __cplusplus
}
#endif

#endif // SAVE_H
