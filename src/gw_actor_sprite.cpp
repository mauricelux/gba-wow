#include "gw_actor_sprite.h"

#include "bn_sprite_double_size_mode.h"
#include "bn_sprite_palette_ptr.h"
#include "bn_sprite_palettes.h"
#include "bn_sprite_tiles_ptr.h"

#include "gw_night.h"
#include "gw_world.h"

namespace gw
{

namespace
{
    // The 32x32 frames have the feet on row 29, so the sprite center sits 13 pixels above them.
    constexpr int feet_to_center = 13;

    constexpr int attack_frames = 10;
    constexpr int walk_frame_ticks = 8;
    constexpr int character_bg_priority = 2;   // between the ground (3) and the overhead layer (1)

    namespace humanoid
    {
        constexpr int down = 0;
        constexpr int up = 4;
        constexpr int side = 8;
        constexpr int attack_down = 12;
        constexpr int attack_up = 13;
        constexpr int attack_side = 14;
        constexpr int cast = 15;
        constexpr int dead = 16;
    }

    namespace creature
    {
        constexpr int attack = 4;
        constexpr int dead = 5;
    }
}

bool actor_sprite::can_create(look_id look)
{
    // Keep two palettes free for effects, icons and floating text.
    int needed = get_look(look).palette.find_palette() ? 32 : 48;
    return bn::sprite_palettes::available_colors_count() >= needed;
}

actor_sprite::actor_sprite(look_id look, const bn::camera_ptr& camera, bn::fixed scale) :
    _sprite(bn::sprite_ptr::create(0, 0, get_look(look).sheet.shape_size(),
                                   get_look(look).sheet.tiles_item().create_tiles(0),
                                   get_look(look).palette.create_palette())),
    _look(&get_look(look)),
    _scale(scale)
{
    _sprite.set_camera(camera);
    _sprite.set_bg_priority(character_bg_priority);

    if(scale != 1)
    {
        _sprite.set_double_size_mode(bn::sprite_double_size_mode::ENABLED);
        _sprite.set_scale(scale);
    }
}

void actor_sprite::set_look(look_id look)
{
    _look = &get_look(look);
    _sprite.set_palette(_look->palette);
    _frame = -1;
}

void actor_sprite::update(const bn::fixed_point& feet, facing direction, bool moving, int walk_counter)
{
    int step = moving ? ((walk_counter / walk_frame_ticks) + 1) % 4 : 0;
    bool attacking = _attack_frames > 0;
    int frame;

    if(attacking)
    {
        --_attack_frames;
    }

    if(_look->creature)
    {
        if(_dead)
        {
            frame = creature::dead;
        }
        else if(attacking)
        {
            frame = creature::attack;
        }
        else
        {
            frame = step;
        }

        // Creatures only have a side view: keep the last horizontal direction when moving vertically.
        if(direction == facing::LEFT)
        {
            _flipped = false;
        }
        else if(direction == facing::RIGHT)
        {
            _flipped = true;
        }
    }
    else
    {
        if(_dead)
        {
            frame = humanoid::dead;
            _flipped = false;
        }
        else if(_casting)
        {
            frame = humanoid::cast;
            _flipped = false;
        }
        else
        {
            switch(direction)
            {

            case facing::UP:
                frame = attacking ? humanoid::attack_up : humanoid::up + step;
                break;

            case facing::LEFT:
            case facing::RIGHT:
                frame = attacking ? humanoid::attack_side : humanoid::side + step;
                break;

            default:
                frame = attacking ? humanoid::attack_down : humanoid::down + step;
                break;
            }

            _flipped = direction == facing::RIGHT;
        }
    }

    if(frame != _frame)
    {
        _sprite.set_tiles(_look->sheet.tiles_item(), frame);
        _frame = frame;
    }

    _sprite.set_horizontal_flip(_flipped);

    if(_flash_frames > 0)
    {
        --_flash_frames;
        _sprite.set_visible((_flash_frames & 2) == 0);
    }

    // Whole pixels only, so characters never shimmer against the background.
    bn::fixed_point screen = world::to_screen_space(feet);
    int center_y = (screen.y() - feet_to_center * _scale).floor_integer() + _offset_y;
    _sprite.set_position(screen.x().floor_integer() + _offset_x, center_y);

    // Characters lower on the map are drawn in front.
    _sprite.set_z_order(-feet.y().floor_integer());

    // At night characters dim with the ground outside the light.
    _sprite.set_blending_enabled(night::enabled());
}

void actor_sprite::play_attack()
{
    _attack_frames = attack_frames;
}

void actor_sprite::set_visible(bool visible)
{
    _sprite.set_visible(visible);
}

void actor_sprite::flash()
{
    _flash_frames = 8;
}

int actor_sprite::height() const
{
    int base = _look->creature ? 20 : 26;
    return (base * _scale).floor_integer() - _offset_y;
}

}
