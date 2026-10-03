#ifndef SAVE_H
#define SAVE_H

#include "gba.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SAVE_MAGIC "TENN100"

typedef struct {
    char magic[8];            // "TENN100\0"
    u8 trophies[3];           // Cups won: Grass (0), Clay (1), Hard Court (2)
    u16 matches_played;       // Total career matches
    u16 matches_won;          // Total career victories
    u16 total_aces;           // Career aces served
    u16 max_serve_speed;      // Fastest serve in MPH (e.g. 132)
    u16 longest_rally;        // Longest shot rally record
    u32 target_high_score;    // Target Practice high score
    u16 checksum;             // Additive 16-bit word verification
} __attribute__((packed)) TennisSaveData;

void save_init(void);
bool save_is_valid(void);
bool save_write(const TennisSaveData* data);
bool save_read(TennisSaveData* data);
void save_erase(void);

#ifdef __cplusplus
}
#endif

#endif // SAVE_H
