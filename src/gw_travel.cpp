#include "gw_travel.h"

#include "bn_math.h"

#include "gw_character.h"
#include "gw_map_redridge.h"
#include "gw_map_stormwind.h"
#include "gw_map_westfall.h"

namespace gw
{

namespace
{
    constexpr flight_def flights[] = {
        { "Stormwind", npc_id::DUNGAR, map_id::STORMWIND, map_data::stormwind::flight, 0, 41, 87 },
        { "Sentinel Hill", npc_id::THOR, map_id::WESTFALL, map_data::westfall::flight, 0, 37, 100 },
        { "Lakeshire", npc_id::ARIENA, map_id::REDRIDGE, map_data::redridge::flight, 0, 67, 92 },
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
