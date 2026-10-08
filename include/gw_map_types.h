#ifndef GW_MAP_TYPES_H
#define GW_MAP_TYPES_H

#include "gw_ids.h"

namespace gw
{

struct point_def
{
    int16_t x;
    int16_t y;
};

// Touching the rectangle moves the player to (target_x, target_y) on the target map.
struct warp_def
{
    int16_t x;
    int16_t y;
    int16_t width;
    int16_t height;
    map_id target;
    int16_t target_x;
    int16_t target_y;
};

struct npc_def
{
    npc_id npc;
    int16_t x;
    int16_t y;
};

struct spawn_def
{
    enemy_id enemy;
    int16_t x;
    int16_t y;
};

// A named part of a map. The smallest area containing the player names where they are.
struct area_def
{
    int16_t x;
    int16_t y;
    int16_t width;
    int16_t height;
    area_id id;
    const char* name;
};

}

#endif
