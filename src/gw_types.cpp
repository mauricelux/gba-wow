#include "gw_types.h"

#include "bn_math.h"

namespace gw
{

facing facing_towards(const bn::fixed_point& from, const bn::fixed_point& to)
{
    bn::fixed dx = to.x() - from.x();
    bn::fixed dy = to.y() - from.y();

    if(bn::abs(dx) >= bn::abs(dy))
    {
        return dx < 0 ? facing::LEFT : facing::RIGHT;
    }

    return dy < 0 ? facing::UP : facing::DOWN;
}

int distance_squared(const bn::fixed_point& a, const bn::fixed_point& b)
{
    int dx = (a.x() - b.x()).integer();
    int dy = (a.y() - b.y()).integer();
    return dx * dx + dy * dy;
}

int distance(const bn::fixed_point& a, const bn::fixed_point& b)
{
    return bn::sqrt(distance_squared(a, b));
}

bn::random& rng()
{
    static bn::random random;
    return random;
}

int random_range(int min, int max)
{
    if(max <= min)
    {
        return min;
    }

    return min + rng().get_int(max - min + 1);
}

bool random_chance(int percent)
{
    return rng().get_int(100) < percent;
}

}
