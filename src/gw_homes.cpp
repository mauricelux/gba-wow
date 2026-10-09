#include "gw_homes.h"

#include "gw_map_elwynn.h"
#include "gw_map_inn.h"
#include "gw_map_redridge.h"
#include "gw_map_stormwind.h"
#include "gw_map_westfall.h"

namespace gw
{

namespace
{
    constexpr home_def homes[] = {
        { "Northshire Abbey", map_id::ELWYNN, map_data::elwynn::start },
        { "Goldshire", map_id::INN, map_data::inn::respawn },
        { "Sentinel Hill", map_id::WESTFALL, map_data::westfall::sentinel_respawn },
        { "Stormwind", map_id::STORMWIND, map_data::stormwind::inn },
        { "Lakeshire", map_id::REDRIDGE, map_data::redridge::lakeshire_respawn },
    };

    static_assert(sizeof(homes) / sizeof(homes[0]) == int(home_id::COUNT));
}

const home_def& get_home(home_id home)
{
    return homes[int(home) < int(home_id::COUNT) ? int(home) : 0];
}

home_id innkeeper_home(npc_id npc)
{
    switch(npc)
    {

    case npc_id::FARLEY:
        return home_id::GOLDSHIRE;

    case npc_id::HEATHER:
        return home_id::SENTINEL_HILL;

    case npc_id::ALLISON:
        return home_id::STORMWIND;

    case npc_id::BRIANNA:
        return home_id::LAKESHIRE;

    default:
        return home_id::COUNT;
    }
}

}
