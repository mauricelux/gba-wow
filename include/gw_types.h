#ifndef GW_TYPES_H
#define GW_TYPES_H

#include "bn_fixed_point.h"
#include "bn_random.h"

namespace gw
{

enum class facing : uint8_t
{
    DOWN,
    UP,
    LEFT,
    RIGHT
};

// The direction from one point towards another, preferring the dominant axis.
[[nodiscard]] facing facing_towards(const bn::fixed_point& from, const bn::fixed_point& to);

// Squared distance in whole pixels.
[[nodiscard]] int distance_squared(const bn::fixed_point& a, const bn::fixed_point& b);

// Distance in whole pixels (rounded down).
[[nodiscard]] int distance(const bn::fixed_point& a, const bn::fixed_point& b);

// The game's random number generator.
[[nodiscard]] bn::random& rng();

// A random integer in [min, max].
[[nodiscard]] int random_range(int min, int max);

// True with the given chance in percent.
[[nodiscard]] bool random_chance(int percent);

}

#endif
