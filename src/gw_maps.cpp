#include "gw_maps.h"

#include "bn_regular_bg_items_map_abbey_ground.h"
#include "bn_regular_bg_items_map_abbey_overhead.h"
#include "bn_regular_bg_items_map_deadmines_ground.h"
#include "bn_regular_bg_items_map_deadmines_overhead.h"
#include "bn_regular_bg_items_map_elwynn_ground.h"
#include "bn_regular_bg_items_map_elwynn_overhead.h"
#include "bn_regular_bg_items_map_inn_ground.h"
#include "bn_regular_bg_items_map_inn_overhead.h"
#include "bn_regular_bg_items_map_westfall_ground.h"
#include "bn_regular_bg_items_map_westfall_overhead.h"

#include "gw_map_abbey.h"
#include "gw_map_deadmines.h"
#include "gw_map_elwynn.h"
#include "gw_map_inn.h"
#include "gw_map_westfall.h"

namespace gw
{

namespace
{
    // The generator writes one placeholder element when a list is empty; drop it here.
    template<typename Type, int Size>
    constexpr bn::span<const Type> list(const Type (&items)[Size], bool placeholder)
    {
        return placeholder ? bn::span<const Type>() : bn::span<const Type>(items, Size);
    }

    constexpr point_def elwynn_graveyards[] = { map_data::elwynn::northshire_respawn,
                                                map_data::elwynn::goldshire_respawn };
    constexpr point_def abbey_graveyards[] = { map_data::abbey::respawn };
    constexpr point_def inn_graveyards[] = { map_data::inn::respawn };
    constexpr point_def westfall_graveyards[] = { map_data::westfall::sentinel_respawn };
    constexpr point_def deadmines_graveyards[] = { map_data::deadmines::respawn };

#define GW_MAP_INFO(ID, NAME, DUNGEON) \
    map_info{ \
        map_id::ID, \
        bn::regular_bg_items::map_##NAME##_ground, \
        bn::regular_bg_items::map_##NAME##_overhead, \
        map_data::NAME::width, \
        map_data::NAME::height, \
        map_data::NAME::collision, \
        map_data::NAME::collision_columns, \
        map_data::NAME::music, \
        DUNGEON, \
        list(map_data::NAME::warps, map_data::NAME::warps[0].target == map_id::NONE), \
        list(map_data::NAME::npcs, map_data::NAME::npcs[0].npc == npc_id::NONE), \
        list(map_data::NAME::spawns, map_data::NAME::spawns[0].enemy == enemy_id::NONE), \
        list(map_data::NAME::areas, map_data::NAME::areas[0].width == 0), \
        bn::span<const point_def>(NAME##_graveyards) \
    }

    const map_info maps[] = {
        GW_MAP_INFO(ELWYNN, elwynn, false),
        GW_MAP_INFO(ABBEY, abbey, false),
        GW_MAP_INFO(INN, inn, false),
        GW_MAP_INFO(WESTFALL, westfall, false),
        GW_MAP_INFO(DEADMINES, deadmines, true),
    };

#undef GW_MAP_INFO
}

const map_info& get_map(map_id id)
{
    for(const map_info& map : maps)
    {
        if(map.id == id)
        {
            return map;
        }
    }

    return maps[0];
}

const point_def& nearest_graveyard(const map_info& map, int x, int y)
{
    const point_def* best = &map.graveyards[0];
    int best_distance = -1;

    for(const point_def& graveyard : map.graveyards)
    {
        int dx = graveyard.x - x;
        int dy = graveyard.y - y;
        int d = dx * dx + dy * dy;

        if(best_distance < 0 || d < best_distance)
        {
            best = &graveyard;
            best_distance = d;
        }
    }

    return *best;
}

const area_def* area_at(const map_info& map, int x, int y)
{
    const area_def* result = nullptr;
    int result_size = 0;

    for(const area_def& area : map.areas)
    {
        if(x >= area.x && y >= area.y && x < area.x + area.width && y < area.y + area.height)
        {
            int size = area.width * area.height;

            if(! result || size < result_size)
            {
                result = &area;
                result_size = size;
            }
        }
    }

    return result;
}

}
