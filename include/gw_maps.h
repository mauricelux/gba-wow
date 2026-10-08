#ifndef GW_MAPS_H
#define GW_MAPS_H

#include "bn_regular_bg_item.h"
#include "bn_span.h"

#include "gw_map_types.h"

namespace gw
{

struct map_info
{
    map_id id;
    const bn::regular_bg_item& ground;
    const bn::regular_bg_item& overhead;
    int width;
    int height;
    const uint8_t* collision;
    int collision_columns;
    music_id music;
    bool dungeon;
    bn::span<const warp_def> warps;
    bn::span<const npc_def> npcs;
    bn::span<const spawn_def> spawns;
    bn::span<const area_def> areas;
    point_def respawn;
};

[[nodiscard]] const map_info& get_map(map_id id);

// The smallest named area containing the point, or nullptr.
[[nodiscard]] const area_def* area_at(const map_info& map, int x, int y);

}

#endif
