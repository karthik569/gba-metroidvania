#ifndef MAP_H
#define MAP_H

#include "types.h"

#define ROOM_WIDTH_TILES   30
#define ROOM_HEIGHT_TILES  20
#define NUM_ROOMS          7

typedef struct {
    u8 tiles[ROOM_HEIGHT_TILES][ROOM_WIDTH_TILES];
    s8 exit_left;
    s8 exit_right;
    s8 exit_up;
    s8 exit_down;
    s8 enemy_type; // 0=none, 1=crawler, 2=boss
    s16 enemy_x;
    s16 enemy_y;
    s8 item_type;  // 0=none, 1=missiles
    s16 item_x;
    s16 item_y;
} Room;

#ifdef __cplusplus
extern "C" {
#endif

void map_init(void);
void map_load_room(u8 room_id);
u8 map_get_tile(s16 pixel_x, s16 pixel_y);
void map_set_tile(u8 tile_x, u8 tile_y, u8 tile_id);
void map_clear_room_item(u8 room_id);
u8 map_current_room_id(void);
const Room* map_get_current_room(void);

#ifdef __cplusplus
}
#endif

#endif // MAP_H
