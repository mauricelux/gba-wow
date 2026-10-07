#ifndef GW_ZONE_BANNER_H
#define GW_ZONE_BANNER_H

#include "bn_sprite_ptr.h"
#include "bn_sprite_text_generator.h"
#include "bn_string_view.h"
#include "bn_vector.h"

namespace gw
{

// Shows a zone's name at the top of the screen for a few seconds, like WoW does when you
// enter a new area.
class zone_banner
{

public:
    explicit zone_banner(bn::sprite_text_generator& text_generator);

    void show(const bn::string_view& zone_name);

    void update();

private:
    bn::sprite_text_generator& _text_generator;
    bn::vector<bn::sprite_ptr, 8> _sprites;
    int _frames_left = 0;
};

}

#endif
