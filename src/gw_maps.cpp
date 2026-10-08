#include "gw_maps.h"

#include "bn_regular_bg_items_map_abbey_ground.h"
#include "bn_regular_bg_items_map_abbey_overhead.h"
#include "bn_regular_bg_items_map_deadmines_ground.h"
#include "bn_regular_bg_items_map_deadmines_overhead.h"
#include "bn_regular_bg_items_map_echo_ridge_ground.h"
#include "bn_regular_bg_items_map_echo_ridge_overhead.h"
#include "bn_regular_bg_items_map_elwynn_ground.h"
#include "bn_regular_bg_items_map_elwynn_overhead.h"
#include "bn_regular_bg_items_map_fargodeep_ground.h"
#include "bn_regular_bg_items_map_fargodeep_overhead.h"
#include "bn_regular_bg_items_map_inn_ground.h"
#include "bn_regular_bg_items_map_inn_overhead.h"
#include "bn_regular_bg_items_map_stockade_ground.h"
#include "bn_regular_bg_items_map_stockade_overhead.h"
#include "bn_regular_bg_items_map_stormwind_ground.h"
#include "bn_regular_bg_items_map_stormwind_overhead.h"
#include "bn_regular_bg_items_map_westfall_ground.h"
#include "bn_regular_bg_items_map_westfall_overhead.h"

#include "gw_map_abbey.h"
#include "gw_map_deadmines.h"
#include "gw_map_echo_ridge.h"
#include "gw_map_elwynn.h"
#include "gw_map_fargodeep.h"
#include "gw_map_inn.h"
#include "gw_map_stockade.h"
#include "gw_map_stormwind.h"
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
    constexpr point_def echo_ridge_graveyards[] = { map_data::echo_ridge::respawn };
    constexpr point_def fargodeep_graveyards[] = { map_data::fargodeep::respawn };
    constexpr point_def stormwind_graveyards[] = { map_data::stormwind::respawn };
    constexpr point_def stockade_graveyards[] = { map_data::stockade::respawn };

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
        bn::span<const point_def>(NAME##_graveyards), \
        list(map_data::NAME::chests, map_data::NAME::chests[0].id == chest_def::none) \
    }

    const map_info maps[] = {
        GW_MAP_INFO(ELWYNN, elwynn, false),
        GW_MAP_INFO(ABBEY, abbey, false),
        GW_MAP_INFO(INN, inn, false),
        GW_MAP_INFO(WESTFALL, westfall, false),
        GW_MAP_INFO(DEADMINES, deadmines, true),
        GW_MAP_INFO(ECHO_RIDGE, echo_ridge, true),
        GW_MAP_INFO(FARGODEEP, fargodeep, true),
        GW_MAP_INFO(STORMWIND, stormwind, false),
        GW_MAP_INFO(STOCKADE, stockade, true),
    };

    [[nodiscard]] int count_chests()
    {
        int result = 0;

        for(const map_info& map : maps)
        {
            result += map.chests.size();
        }

        return result;
    }

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

bn::span<const map_info> all_maps()
{
    return maps;
}

int total_chests()
{
    static const int result = count_chests();
    return result;
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
