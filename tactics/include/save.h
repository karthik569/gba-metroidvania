#ifndef SAVE_H
#define SAVE_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

void save_init(void);
u8   save_get_max_mission(void);
void save_set_max_mission(u8 mission);
u8   save_get_mission_rank(u8 mission);
void save_set_mission_rank(u8 mission, u8 rank);
u32  save_get_career_score(void);
void save_add_career_score(u32 score);

#ifdef __cplusplus
}
#endif

#endif // SAVE_H
