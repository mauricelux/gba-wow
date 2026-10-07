#include "gw_world.h"

#include "gw_northshire_collision.h"

namespace gw
{

namespace
{
    constexpr int cell_size = northshire::collision_cell_size;
    constexpr int bytes_per_row = northshire::collision_columns / 8;

    static_assert(northshire::collision_columns * cell_size == world::width);
    static_assert(northshire::collision_rows * cell_size == world::height);

    [[nodiscard]] bool cell_solid(int column, int row)
    {
        if(column < 0 || row < 0 || column >= northshire::collision_columns || row >= northshire::collision_rows)
        {
            return true;
        }

        int byte = northshire::collision[row * bytes_per_row + (column >> 3)];
        return (byte >> (column & 7)) & 1;
    }
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
    if(left < 0 || top < 0 || right >= width || bottom >= height)
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

}
