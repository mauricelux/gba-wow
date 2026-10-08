#include "gw_fade.h"

#include "bn_bg_palettes.h"
#include "bn_color.h"
#include "bn_sprite_palettes.h"

namespace gw
{

void set_fade(bn::fixed intensity)
{
    bn::bg_palettes::set_fade(bn::color(0, 0, 0), intensity);
    bn::sprite_palettes::set_fade(bn::color(0, 0, 0), intensity);
}

}
