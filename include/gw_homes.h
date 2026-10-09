#ifndef GW_HOMES_H
#define GW_HOMES_H

#include "gw_ids.h"
#include "gw_map_types.h"

namespace gw
{

// Inns the hearthstone can take the player back to. Saves store the index: append only.
enum class home_id : uint8_t
{
    NORTHSHIRE,
    GOLDSHIRE,
    SENTINEL_HILL,
    STORMWIND,
    LAKESHIRE,
    DARKSHIRE,
    IRONFORGE,
    MENETHIL,
    AUBERDINE,
    SOUTHSHORE,
    COUNT
};

struct home_def
{
    const char* name;
    map_id map;
    point_def point;
};

[[nodiscard]] const home_def& get_home(home_id home);

// The inn an innkeeper looks after, or COUNT for npcs who aren't innkeepers.
[[nodiscard]] home_id innkeeper_home(npc_id npc);

// Frames between two uses of the hearthstone.
constexpr int hearthstone_cooldown = 10 * 60 * 60;

}

#endif
