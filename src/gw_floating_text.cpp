#include "gw_floating_text.h"

#include "bn_math.h"
#include "bn_sprite_text_generator.h"
#include "bn_string.h"

#include "bn_sprite_items_fx_text_blue.h"
#include "bn_sprite_items_fx_text_green.h"
#include "bn_sprite_items_fx_text_purple.h"
#include "bn_sprite_items_fx_text_red.h"
#include "bn_sprite_items_fx_text_white.h"
#include "bn_sprite_items_fx_text_yellow.h"
#include "common_fixed_8x8_sprite_font.h"

#include "gw_sprite_palettes.h"
#include "gw_world.h"

namespace gw
{

namespace
{
    constexpr int lifetime = 48;

    // Butano's fixed 8x8 font drawn in each style's colors (tools/gen_effects.py). The sheets share one
    // palette, so every style on screen together takes a single sprite palette.
    constexpr bn::utf8_characters_map_ref characters = common::fixed_8x8_sprite_font_utf8_characters_map.reference();

    constexpr bn::sprite_font fonts[] = {
        bn::sprite_font(bn::sprite_items::fx_text_white, characters),
        bn::sprite_font(bn::sprite_items::fx_text_yellow, characters),
        bn::sprite_font(bn::sprite_items::fx_text_red, characters),
        bn::sprite_font(bn::sprite_items::fx_text_green, characters),
        bn::sprite_font(bn::sprite_items::fx_text_blue, characters),
        bn::sprite_font(bn::sprite_items::fx_text_purple, characters),
    };
}

floating_texts::floating_texts(const bn::camera_ptr& camera) :
    _camera(camera)
{
}

void floating_texts::show(const bn::fixed_point& world_position, const bn::string_view& text, style text_style)
{
    if(! sprite_palettes::fits(bn::sprite_items::fx_text_white.palette_item()))
    {
        return;
    }

    if(_entries.full())
    {
        _entries.erase(_entries.begin());
    }

    // Texts started at the same spot stack upwards instead of overlapping.
    bn::fixed_point position = world_position;

    for(const entry& other : _entries)
    {
        if(other.frames < 12 && bn::abs(other.position.x() - position.x()) < 16 &&
           bn::abs(other.position.y() - position.y()) < 8)
        {
            position.set_y(position.y() - 8);
        }
    }

    entry& new_entry = _entries.emplace_back();
    new_entry.position = position;
    bn::sprite_text_generator generator(fonts[int(text_style)]);
    generator.set_center_alignment();
    generator.set_bg_priority(1);   // over tree tops, under the ui layer (menus, dialog)

    bn::fixed_point screen = world::to_screen_space(position);
    generator.generate(screen, text.substr(0, bn::min(text.size(), max_length)), new_entry.sprites);

    for(bn::sprite_ptr& sprite : new_entry.sprites)
    {
        sprite.set_camera(_camera);
    }
}

void floating_texts::show_number(const bn::fixed_point& world_position, int value, style text_style)
{
    show(world_position, bn::to_string<8>(value), text_style);
}

void floating_texts::update()
{
    for(auto it = _entries.begin(); it != _entries.end();)
    {
        entry& current = *it;
        ++current.frames;

        if(current.frames >= lifetime)
        {
            it = _entries.erase(it);
            continue;
        }

        // Rise quickly at first, then slow down.
        if((current.frames < 16 && (current.frames & 1)) || (current.frames & 3) == 0)
        {
            for(bn::sprite_ptr& sprite : current.sprites)
            {
                sprite.set_y(sprite.y() - 1);
            }
        }

        ++it;
    }
}

void floating_texts::clear()
{
    _entries.clear();
}

}
