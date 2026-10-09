#ifndef GW_NIGHT_H
#define GW_NIGHT_H

#include "bn_fixed_point.h"

namespace gw::night
{

// Night maps (Duskwood) darken the world outside a circle of light around the hero. The ground,
// the overhead layer and the characters dim; the HUD, text and spell effects stay bright.

// Turns the darkness on or off, when a map loads.
void set_enabled(bool enabled);

[[nodiscard]] bool enabled();

// Moves the circle; center is in screen space (0, 0 is the middle of the screen).
void update(const bn::fixed_point& center);

}

#endif
