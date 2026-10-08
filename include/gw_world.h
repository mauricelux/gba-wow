#ifndef GW_WORLD_H
#define GW_WORLD_H

#include "bn_fixed_point.h"

#include "gw_maps.h"

namespace gw
{

// The map the player is on. Coordinates are world pixels with (0, 0) at the map's top-left corner.
class world
{

public:
    static constexpr int cell_size = 8;

    static void set_map(const map_info& map);

    [[nodiscard]] static const map_info& map();

    [[nodiscard]] static int width();

    [[nodiscard]] static int height();

    // True if the pixel is blocked (walls, trunks, water) or outside the map.
    [[nodiscard]] static bool solid_at(int x, int y);

    // True if no pixel of the inclusive rectangle [left, right] x [top, bottom] is solid.
    [[nodiscard]] static bool area_free(int left, int top, int right, int bottom);

    // True if nothing solid lies on the straight line between the two points.
    [[nodiscard]] static bool line_clear(int x0, int y0, int x1, int y1);

    // Converts world pixels to Butano's coordinates, where (0, 0) is the map's center.
    [[nodiscard]] static bn::fixed_point to_screen_space(const bn::fixed_point& world_position);
};

}

#endif
