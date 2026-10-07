#include "gw_zone_banner.h"

namespace gw
{

namespace
{
    constexpr int visible_frames = 180;
    constexpr int banner_y = -60;
}

zone_banner::zone_banner(bn::sprite_text_generator& text_generator) :
    _text_generator(text_generator)
{
}

void zone_banner::show(const bn::string_view& zone_name)
{
    _sprites.clear();
    _text_generator.set_center_alignment();
    _text_generator.generate(0, banner_y, zone_name, _sprites);
    _frames_left = visible_frames;
}

void zone_banner::update()
{
    if(_frames_left > 0 && --_frames_left == 0)
    {
        _sprites.clear();
    }
}

}
