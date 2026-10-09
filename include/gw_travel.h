#ifndef GW_TRAVEL_H
#define GW_TRAVEL_H

#include "gw_ids.h"
#include "gw_map_types.h"

namespace gw
{

// Flight paths. Saves keep a bit per path: only append.
enum class flight_id : uint8_t
{
    STORMWIND,
    SENTINEL_HILL,
    LAKESHIRE,
    DARKSHIRE,
    SCOUTS_CAMP,
    IRONFORGE,
    MENETHIL,
    AUBERDINE,
    SOUTHSHORE,
    ARGENT_WATCH,
    BOOTY_BAY,
    GADGETZAN,
    FEATHERMOON,
    NIJELS_POINT,
    MORGANS_VIGIL,
    SWAMP_OF_SORROWS,
    COUNT
};

struct flight_def
{
    const char* name;
    npc_id master;          // the gryphon master who looks after the path
    map_id map;
    point_def landing;      // in front of the gryphon master
    uint8_t continent;      // index in gw::continents
    uint8_t x;              // its spot on the continent picture, beside its zone's
    uint8_t y;
};

[[nodiscard]] const flight_def& get_flight(flight_id flight);

// The path a gryphon master looks after; COUNT for other npcs.
[[nodiscard]] flight_id master_flight(npc_id npc);

[[nodiscard]] bool flight_known(flight_id flight);

// Talking to its gryphon master. Returns true when the path was new.
bool discover_flight(flight_id flight);

// In copper: more for longer flights.
[[nodiscard]] int flight_cost(flight_id from, flight_id to);

// Frames in the air.
[[nodiscard]] int flight_frames(flight_id from, flight_id to);

}

#endif
