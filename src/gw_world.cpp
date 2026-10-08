#include "gw_world.h"

#include "bn_math.h"

namespace gw
{

namespace
{
    const map_info* current_map = nullptr;
    int bytes_per_row = 0;
    int rows = 0;

    [[nodiscard]] bool cell_solid(int column, int row)
    {
        if(column < 0 || row < 0 || column >= current_map->collision_columns || row >= rows)
        {
            return true;
        }

        int byte = current_map->collision[row * bytes_per_row + (column >> 3)];
        return (byte >> (column & 7)) & 1;
    }
}

void world::set_map(const map_info& map)
{
    current_map = &map;
    bytes_per_row = map.collision_columns / 8;
    rows = map.height / cell_size;
}

const map_info& world::map()
{
    return *current_map;
}

int world::width()
{
    return current_map->width;
}

int world::height()
{
    return current_map->height;
}

bool world::solid_at(int x, int y)
{
    if(x < 0 || y < 0)
    {
        return true;
    }

    return cell_solid(x / cell_size, y / cell_size);
}

bool world::area_free(int left, int top, int right, int bottom)
{
    if(left < 0 || top < 0 || right >= current_map->width || bottom >= current_map->height)
    {
        return false;
    }

    for(int row = top / cell_size, last_row = bottom / cell_size; row <= last_row; ++row)
    {
        for(int column = left / cell_size, last_column = right / cell_size; column <= last_column; ++column)
        {
            if(cell_solid(column, row))
            {
                return false;
            }
        }
    }

    return true;
}

bool world::line_clear(int x0, int y0, int x1, int y1)
{
    // Sample every half cell along the line; walls are at least one cell thick.
    int dx = x1 - x0;
    int dy = y1 - y0;
    int steps = bn::max(bn::abs(dx), bn::abs(dy)) / (cell_size / 2);

    for(int step = 1; step < steps; ++step)
    {
        if(solid_at(x0 + dx * step / steps, y0 + dy * step / steps))
        {
            return false;
        }
    }

    return true;
}

bn::fixed_point world::to_screen_space(const bn::fixed_point& world_position)
{
    if(! current_map)
    {
        return world_position;
    }

    return world_position - bn::fixed_point(current_map->width / 2, current_map->height / 2);
}

}
