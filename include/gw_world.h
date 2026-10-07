#ifndef GW_WORLD_H
#define GW_WORLD_H

#include "bn_fixed_point.h"

namespace gw
{

// The playable map. Coordinates are world pixels with (0, 0) at the map's top-left corner.
// For milestone 1 this is the single Northshire test map; milestone 2 replaces it with a
// streamed open world.
class world
{

public:
    static constexpr int width = 1024;
    static constexpr int height = 1024;

    // True if the pixel is blocked (walls, trunks, water) or outside the map.
    [[nodiscard]] static bool solid_at(int x, int y);

    // True if no pixel of the inclusive rectangle [left, right] x [top, bottom] is solid.
    [[nodiscard]] static bool area_free(int left, int top, int right, int bottom);

    // Converts world pixels to Butano's coordinates, where (0, 0) is the map's center.
    [[nodiscard]] static constexpr bn::fixed_point to_screen_space(const bn::fixed_point& world_position)
    {
        return world_position - bn::fixed_point(width / 2, height / 2);
    }
};

}

#endif
