#include "gw_homes.h"

#include "gw_map_burning_steppes.h"
#include "gw_map_darkshore.h"
#include "gw_map_desolace.h"
#include "gw_map_duskwood.h"
#include "gw_map_elwynn.h"
#include "gw_map_feralas.h"
#include "gw_map_hillsbrad.h"
#include "gw_map_inn.h"
#include "gw_map_ironforge.h"
#include "gw_map_redridge.h"
#include "gw_map_stormwind.h"
#include "gw_map_stranglethorn.h"
#include "gw_map_tanaris.h"
#include "gw_map_westfall.h"
#include "gw_map_wetlands.h"

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
        { "Darkshire", map_id::DUSKWOOD, map_data::duskwood::darkshire_respawn },
        { "Ironforge", map_id::IRONFORGE, map_data::ironforge::ironforge_respawn },
        { "Menethil Harbor", map_id::WETLANDS, map_data::wetlands::menethil_respawn },
        { "Auberdine", map_id::DARKSHORE, map_data::darkshore::auberdine_respawn },
        { "Southshore", map_id::HILLSBRAD, map_data::hillsbrad::southshore_respawn },
        { "Booty Bay", map_id::STRANGLETHORN, map_data::stranglethorn::booty_bay_respawn },
        { "Gadgetzan", map_id::TANARIS, map_data::tanaris::gadgetzan_respawn },
        { "Feathermoon", map_id::FERALAS, map_data::feralas::feathermoon_respawn },
        { "Nijel's Point", map_id::DESOLACE, map_data::desolace::nijels_respawn },
        { "Morgan's Vigil", map_id::BURNING_STEPPES, map_data::burning_steppes::vigil_respawn },
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

    case npc_id::TRELAYNE:
        return home_id::DARKSHIRE;

    case npc_id::FIREBREW:
        return home_id::IRONFORGE;

    case npc_id::HELBREK:
        return home_id::MENETHIL;

    case npc_id::SHAUSSIY:
        return home_id::AUBERDINE;

    case npc_id::ANDERSON:
        return home_id::SOUTHSHORE;

    case npc_id::SKINDLE:
        return home_id::BOOTY_BAY;

    case npc_id::FIZZGRIMBLE:
        return home_id::GADGETZAN;

    case npc_id::SHYRIA:
        return home_id::FEATHERMOON;

    case npc_id::LYSHAERYA:
        return home_id::NIJELS_POINT;

    case npc_id::VIGIL_INNKEEPER:
        return home_id::MORGANS_VIGIL;

    default:
        return home_id::COUNT;
    }
}

}
