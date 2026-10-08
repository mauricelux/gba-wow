#include "gw_floating_text.h"

#include "bn_color.h"
#include "bn_math.h"
#include "bn_sprite_palette_item.h"
#include "bn_string.h"

#include "common_fixed_8x8_sprite_font.h"

#include "gw_world.h"

namespace gw
{

namespace
{
    constexpr int lifetime = 48;

    constexpr bn::color colors[][16] = {
        { bn::color(31, 0, 31), bn::color(31, 31, 31), bn::color(2, 2, 3) },
        { bn::color(31, 0, 31), bn::color(31, 27, 8), bn::color(6, 3, 0) },
        { bn::color(31, 0, 31), bn::color(31, 9, 7), bn::color(6, 0, 0) },
        { bn::color(31, 0, 31), bn::color(11, 30, 9), bn::color(0, 5, 0) },
        { bn::color(31, 0, 31), bn::color(15, 25, 31), bn::color(1, 3, 8) },
        { bn::color(31, 0, 31), bn::color(25, 16, 31), bn::color(5, 1, 8) },
    };

    constexpr bn::sprite_palette_item palettes[] = {
        bn::sprite_palette_item(colors[0], bn::bpp_mode::BPP_4),
        bn::sprite_palette_item(colors[1], bn::bpp_mode::BPP_4),
        bn::sprite_palette_item(colors[2], bn::bpp_mode::BPP_4),
        bn::sprite_palette_item(colors[3], bn::bpp_mode::BPP_4),
        bn::sprite_palette_item(colors[4], bn::bpp_mode::BPP_4),
        bn::sprite_palette_item(colors[5], bn::bpp_mode::BPP_4),
    };
}

floating_texts::floating_texts(const bn::camera_ptr& camera) :
    _camera(camera),
    _generator(common::fixed_8x8_sprite_font)
{
    _generator.set_center_alignment();
    _generator.set_bg_priority(0);
}

void floating_texts::show(const bn::fixed_point& world_position, const bn::string_view& text, style text_style)
{
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
    _generator.set_palette_item(palettes[int(text_style)]);

    bn::fixed_point screen = world::to_screen_space(position);
    _generator.generate(screen, text, new_entry.sprites);

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
