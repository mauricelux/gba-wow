#ifndef GW_LOOKS_H
#define GW_LOOKS_H

#include "bn_sprite_item.h"
#include "bn_sprite_palette_item.h"

#include "gw_look_ids.h"

namespace gw
{

// What a character looks like: one of the generated sheets (tools/gen_characters.py) drawn with
// one of the generated palettes.
struct look_def
{
    const bn::sprite_item& sheet;
    const bn::sprite_palette_item& palette;
    bool creature;   // side-view creature sheet (6 frames) instead of a humanoid sheet (17 frames)
};

[[nodiscard]] const look_def& get_look(look_id look);

}

#endif
