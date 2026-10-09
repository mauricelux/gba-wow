#include "gw_travel.h"

#include "bn_math.h"

#include "gw_character.h"
#include "gw_map_burning_steppes.h"
#include "gw_map_darkshore.h"
#include "gw_map_desolace.h"
#include "gw_map_duskwood.h"
#include "gw_map_feralas.h"
#include "gw_map_hillsbrad.h"
#include "gw_map_ironforge.h"
#include "gw_map_plaguelands.h"
#include "gw_map_redridge.h"
#include "gw_map_silverpine.h"
#include "gw_map_stormwind.h"
#include "gw_map_stranglethorn.h"
#include "gw_map_swamp_of_sorrows.h"
#include "gw_map_tanaris.h"
#include "gw_map_tirisfal.h"
#include "gw_map_westfall.h"
#include "gw_map_wetlands.h"

namespace gw
{

namespace
{
    constexpr flight_def flights[] = {
        { "Stormwind", npc_id::DUNGAR, map_id::STORMWIND, map_data::stormwind::flight, 0, 41, 87 },
        { "Sentinel Hill", npc_id::THOR, map_id::WESTFALL, map_data::westfall::flight, 0, 37, 100 },
        { "Lakeshire", npc_id::ARIENA, map_id::REDRIDGE, map_data::redridge::flight, 0, 67, 92 },
        { "Darkshire", npc_id::FELICIA, map_id::DUSKWOOD, map_data::duskwood::flight, 0, 55, 102 },
        { "Scouts' Camp", npc_id::GRYPHON_SILVERPINE, map_id::SILVERPINE, map_data::silverpine::flight, 0, 34, 43 },
        { "Ironforge", npc_id::GRYTH, map_id::IRONFORGE, map_data::ironforge::flight, 0, 41, 69 },
        { "Menethil Harbor", npc_id::SHELLEI, map_id::WETLANDS, map_data::wetlands::flight, 0, 50, 53 },
        { "Auberdine", npc_id::CAYLAIS, map_id::DARKSHORE, map_data::darkshore::flight, 1, 40, 30 },
        { "Southshore", npc_id::DARLA, map_id::HILLSBRAD, map_data::hillsbrad::flight, 0, 56, 47 },
        { "Argent Watch", npc_id::GRYPHON_TIRISFAL, map_id::TIRISFAL, map_data::tirisfal::flight, 0, 44, 20 },
        { "Booty Bay", npc_id::GYLL, map_id::STRANGLETHORN, map_data::stranglethorn::flight, 0, 29, 108 },
        { "Gadgetzan", npc_id::BERA, map_id::TANARIS, map_data::tanaris::flight, 1, 62, 92 },
        { "Feathermoon", npc_id::FYLDREN, map_id::FERALAS, map_data::feralas::flight, 1, 22, 68 },
        { "Nijel's Point", npc_id::BARITANAS, map_id::DESOLACE, map_data::desolace::flight, 1, 38, 44 },
        { "Morgan's Vigil", npc_id::BORGUS, map_id::BURNING_STEPPES, map_data::burning_steppes::flight, 0, 56, 80 },
        { "Swamp of Sorrows", npc_id::SWAMP_GRYPHON, map_id::SWAMP_OF_SORROWS, map_data::swamp_of_sorrows::flight,
          0, 62, 99 },
        { "Chillwind Camp", npc_id::BIBILFAZ, map_id::PLAGUELANDS, map_data::plaguelands::chillwind_flight, 0, 54, 32 },
        { "Light's Hope Chapel", npc_id::KHAELYN, map_id::PLAGUELANDS, map_data::plaguelands::lights_hope_flight,
          0, 90, 26 },
    };

    static_assert(sizeof(flights) / sizeof(flights[0]) == int(flight_id::COUNT));
    static_assert(int(flight_id::COUNT) <= 32, "character_data::flights has a bit per path");

    constexpr int min_cost = 50;
    constexpr int cost_per_pixel = 5;       // per pixel of the continent picture
    constexpr int min_frames = 240;
    constexpr int max_frames = 600;
    constexpr int frames_per_pixel = 8;

    [[nodiscard]] int distance(flight_id from, flight_id to)
    {
        const flight_def& a = get_flight(from);
        const flight_def& b = get_flight(to);
        int dx = a.x - b.x;
        int dy = a.y - b.y;
        return bn::sqrt(dx * dx + dy * dy);
    }
}

const flight_def& get_flight(flight_id flight)
{
    int index = int(flight);
    return flights[index < int(flight_id::COUNT) ? index : 0];
}

flight_id master_flight(npc_id npc)
{
    for(int index = 0; index < int(flight_id::COUNT); ++index)
    {
        if(flights[index].master == npc)
        {
            return flight_id(index);
        }
    }

    return flight_id::COUNT;
}

bool flight_known(flight_id flight)
{
    return character().flights & (1u << int(flight));
}

bool discover_flight(flight_id flight)
{
    if(flight == flight_id::COUNT || flight_known(flight))
    {
        return false;
    }

    character().flights |= 1u << int(flight);
    return true;
}

int flight_cost(flight_id from, flight_id to)
{
    return min_cost + distance(from, to) * cost_per_pixel;
}

int flight_frames(flight_id from, flight_id to)
{
    return bn::clamp(distance(from, to) * frames_per_pixel, min_frames, max_frames);
}

}
