#ifndef GW_FADE_H
#define GW_FADE_H

#include "bn_fixed.h"

namespace gw
{

// Fades every background and sprite towards black. 0 is fully visible, 1 is black.
void set_fade(bn::fixed intensity);

}

#endif
