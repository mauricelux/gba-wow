#include "gw_maps.h"

#include "bn_regular_bg_items_map_abbey_ground.h"
#include "bn_regular_bg_items_map_abbey_overhead.h"
#include "bn_regular_bg_items_map_blackfathom_deeps_ground.h"
#include "bn_regular_bg_items_map_blackfathom_deeps_overhead.h"
#include "bn_regular_bg_items_map_darkshore_ground.h"
#include "bn_regular_bg_items_map_darkshore_overhead.h"
#include "bn_regular_bg_items_map_deadmines_ground.h"
#include "bn_regular_bg_items_map_deadmines_overhead.h"
#include "bn_regular_bg_items_map_deeprun_tram_ground.h"
#include "bn_regular_bg_items_map_deeprun_tram_overhead.h"
#include "bn_regular_bg_items_map_dun_morogh_ground.h"
#include "bn_regular_bg_items_map_dun_morogh_overhead.h"
#include "bn_regular_bg_items_map_duskwood_ground.h"
#include "bn_regular_bg_items_map_duskwood_overhead.h"
#include "bn_regular_bg_items_map_echo_ridge_ground.h"
#include "bn_regular_bg_items_map_echo_ridge_overhead.h"
#include "bn_regular_bg_items_map_elwynn_ground.h"
#include "bn_regular_bg_items_map_elwynn_overhead.h"
#include "bn_regular_bg_items_map_fargodeep_ground.h"
#include "bn_regular_bg_items_map_fargodeep_overhead.h"
#include "bn_regular_bg_items_map_gnomeregan_ground.h"
#include "bn_regular_bg_items_map_gnomeregan_overhead.h"
#include "bn_regular_bg_items_map_inn_ground.h"
#include "bn_regular_bg_items_map_inn_overhead.h"
#include "bn_regular_bg_items_map_ironforge_ground.h"
#include "bn_regular_bg_items_map_ironforge_overhead.h"
#include "bn_regular_bg_items_map_redridge_ground.h"
#include "bn_regular_bg_items_map_redridge_overhead.h"
#include "bn_regular_bg_items_map_shadowfang_ground.h"
#include "bn_regular_bg_items_map_shadowfang_overhead.h"
#include "bn_regular_bg_items_map_silverpine_ground.h"
#include "bn_regular_bg_items_map_silverpine_overhead.h"
#include "bn_regular_bg_items_map_stockade_ground.h"
#include "bn_regular_bg_items_map_stockade_overhead.h"
#include "bn_regular_bg_items_map_stormwind_ground.h"
#include "bn_regular_bg_items_map_stormwind_overhead.h"
#include "bn_regular_bg_items_map_westfall_ground.h"
#include "bn_regular_bg_items_map_westfall_overhead.h"
#include "bn_regular_bg_items_map_wetlands_ground.h"
#include "bn_regular_bg_items_map_wetlands_overhead.h"

#include "gw_map_abbey.h"
#include "gw_map_blackfathom_deeps.h"
#include "gw_map_darkshore.h"
#include "gw_map_deadmines.h"
#include "gw_map_deeprun_tram.h"
#include "gw_map_dun_morogh.h"
#include "gw_map_duskwood.h"
#include "gw_map_echo_ridge.h"
#include "gw_map_elwynn.h"
#include "gw_map_fargodeep.h"
#include "gw_map_gnomeregan.h"
#include "gw_map_inn.h"
#include "gw_map_ironforge.h"
#include "gw_map_redridge.h"
#include "gw_map_shadowfang.h"
#include "gw_map_silverpine.h"
#include "gw_map_stockade.h"
#include "gw_map_stormwind.h"
#include "gw_map_westfall.h"
#include "gw_map_wetlands.h"

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
    constexpr point_def deeprun_tram_graveyards[] = { map_data::deeprun_tram::respawn };
    constexpr point_def redridge_graveyards[] = { map_data::redridge::lakeshire_respawn };
    constexpr point_def duskwood_graveyards[] = { map_data::duskwood::darkshire_respawn,
                                                  map_data::duskwood::raven_hill_respawn };
    constexpr point_def silverpine_graveyards[] = { map_data::silverpine::silverpine_respawn };
    constexpr point_def shadowfang_graveyards[] = { map_data::shadowfang::respawn };
    constexpr point_def ironforge_graveyards[] = { map_data::ironforge::ironforge_respawn };
    constexpr point_def dun_morogh_graveyards[] = { map_data::dun_morogh::dun_morogh_respawn };
    constexpr point_def wetlands_graveyards[] = { map_data::wetlands::menethil_respawn };
    constexpr point_def darkshore_graveyards[] = { map_data::darkshore::auberdine_respawn };
    constexpr point_def blackfathom_deeps_graveyards[] = { map_data::blackfathom_deeps::respawn };
    constexpr point_def gnomeregan_graveyards[] = { map_data::gnomeregan::respawn };

#define GW_MAP_INFO(ID, NAME, DUNGEON, INDOORS) \
    map_info{ \
        map_id::ID, \
        bn::regular_bg_items::map_##NAME##_ground, \
        bn::regular_bg_items::map_##NAME##_overhead, \
        map_data::NAME::width, \
        map_data::NAME::height, \
        map_data::NAME::collision, \
        map_data::NAME::water, \
        map_data::NAME::collision_columns, \
        map_data::NAME::music, \
        DUNGEON, \
        INDOORS, \
        map_data::NAME::night, \
        list(map_data::NAME::warps, map_data::NAME::warps[0].target == map_id::NONE), \
        list(map_data::NAME::npcs, map_data::NAME::npcs[0].npc == npc_id::NONE), \
        list(map_data::NAME::spawns, map_data::NAME::spawns[0].enemy == enemy_id::NONE), \
        list(map_data::NAME::areas, map_data::NAME::areas[0].width == 0), \
        bn::span<const point_def>(NAME##_graveyards), \
        list(map_data::NAME::chests, map_data::NAME::chests[0].id == chest_def::none), \
        list(map_data::NAME::patrol, map_data::NAME::patrol[0].x < 0) \
    }

    const map_info maps[] = {
        GW_MAP_INFO(ELWYNN, elwynn, false, false),
        GW_MAP_INFO(ABBEY, abbey, false, true),
        GW_MAP_INFO(INN, inn, false, true),
        GW_MAP_INFO(WESTFALL, westfall, false, false),
        GW_MAP_INFO(DEADMINES, deadmines, true, true),
        GW_MAP_INFO(ECHO_RIDGE, echo_ridge, true, true),
        GW_MAP_INFO(FARGODEEP, fargodeep, true, true),
        GW_MAP_INFO(STORMWIND, stormwind, false, false),
        GW_MAP_INFO(STOCKADE, stockade, true, true),
        GW_MAP_INFO(DEEPRUN_TRAM, deeprun_tram, false, true),
        GW_MAP_INFO(REDRIDGE, redridge, false, false),
        GW_MAP_INFO(DUSKWOOD, duskwood, false, false),
        GW_MAP_INFO(SILVERPINE, silverpine, false, false),
        GW_MAP_INFO(SHADOWFANG, shadowfang, true, true),
        GW_MAP_INFO(IRONFORGE, ironforge, false, false),
        GW_MAP_INFO(DUN_MOROGH, dun_morogh, false, false),
        GW_MAP_INFO(WETLANDS, wetlands, false, false),
        GW_MAP_INFO(DARKSHORE, darkshore, false, false),
        GW_MAP_INFO(BLACKFATHOM_DEEPS, blackfathom_deeps, true, true),
        GW_MAP_INFO(GNOMEREGAN, gnomeregan, true, true),
    };

    [[nodiscard]] int count_chests()
    {
        int result = 0;

        for(const map_info& map : maps)
        {
            for(const chest_def& chest : map.chests)
            {
                // Braziers aren't treasure.
                result += chest.level != 0;
            }
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
        if(area.id == area_id::RADIATION)
        {
            continue;
        }

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

bool in_radiation(const map_info& map, int x, int y)
{
    for(const area_def& area : map.areas)
    {
        if(area.id == area_id::RADIATION && x >= area.x && y >= area.y && x < area.x + area.width &&
           y < area.y + area.height)
        {
            return true;
        }
    }

    return false;
}

}
