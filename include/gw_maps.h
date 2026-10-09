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
    const uint8_t* water;           // the solid cells that are water, laid out like collision
    int collision_columns;
    music_id music;
    bool dungeon;
    bool indoors;       // no mounts
    bool night;         // dark outside a circle of light around the hero
    bn::span<const warp_def> warps;
    bn::span<const npc_def> npcs;
    bn::span<const spawn_def> spawns;
    bn::span<const area_def> areas;
    bn::span<const point_def> graveyards;   // where the player comes back to life after dying
    bn::span<const chest_def> chests;
    bn::span<const point_def> patrol;       // the road a patrolling enemy walks, or a boss's blink spots
};

// Every map, in map_id order (without NONE).
[[nodiscard]] bn::span<const map_info> all_maps();

// Treasure chests in the whole world.
[[nodiscard]] int total_chests();

[[nodiscard]] const map_info& get_map(map_id id);

// The graveyard closest to the point.
[[nodiscard]] const point_def& nearest_graveyard(const map_info& map, int x, int y);

// The smallest named area containing the point, or nullptr. Radiation pools and event areas don't count.
[[nodiscard]] const area_def* area_at(const map_info& map, int x, int y);

// GONG, CAGE or PRISON when the point is where an event starts (a gong to ring, a cage to open, a
// prison whose force field is down), else NONE.
[[nodiscard]] area_id event_area_at(const map_info& map, int x, int y);

// Whether the point is in a radiation pool (area_id::RADIATION), which hurts while the player stands in it.
[[nodiscard]] bool in_radiation(const map_info& map, int x, int y);

}

#endif
