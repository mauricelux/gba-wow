#ifndef GW_INPUT_H
#define GW_INPUT_H

#include "bn_keypad.h"

namespace gw::input
{

// Call once per frame before reading repeats.
void update();

// True when the key was pressed this frame, then repeatedly while it stays held (menus).
[[nodiscard]] bool repeated(bn::keypad::key_type key);

}

#endif
