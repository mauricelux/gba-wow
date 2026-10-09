#ifndef GW_MAP_MARKS_H
#define GW_MAP_MARKS_H

namespace gw
{

// The frames of fx_map_marks (tools/gen_effects.py): the world map's and the flight's markers.
enum map_mark
{
    MARK_PLAYER,
    MARK_PLAYER_BLINK,
    MARK_QUEST,
    MARK_TURN_IN,
    MARK_CHEST,
    MARK_FLIGHT,
    MARK_ZONE,
    MARK_ZONE_LATER,
    MARK_CURSOR,
    MARK_CURSOR_BLINK,
    MARK_ROUTE
};

}

#endif
