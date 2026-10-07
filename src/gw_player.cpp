#include "gw_player.h"

#include "bn_keypad.h"
#include "bn_math.h"

#include "bn_sprite_items_warrior.h"

#include "gw_world.h"

namespace gw
{

namespace
{
    constexpr bn::fixed walk_speed = 1.25;
    constexpr bn::fixed run_speed = 2;
    constexpr bn::fixed diagonal_factor = 0.7071;

    // Hitbox around the feet, in pixels relative to the feet position. Small and low so the
    // character can walk close to walls and behind tree tops.
    constexpr int hitbox_left = -5;
    constexpr int hitbox_right = 4;
    constexpr int hitbox_top = -5;

    // How far (in pixels) the player is nudged around a corner they walk into.
    constexpr int corner_tolerance = 6;

    // The 32x32 frame has the feet on row 29, so the sprite center sits 13 pixels above them.
    constexpr bn::fixed sprite_offset_y = -13;

    constexpr int walk_frame_ticks = 8;
    constexpr int frames_per_direction = 4;
    constexpr int down_frames = 0;
    constexpr int up_frames = 4;
    constexpr int side_frames = 8;

    constexpr int sprite_bg_priority = 2;   // between the ground (3) and the overhead layer (1)
}

player::player(const bn::fixed_point& feet_position, const bn::camera_ptr& camera) :
    _position(feet_position),
    _sprite(bn::sprite_items::warrior.create_sprite(0, 0))
{
    _sprite.set_camera(camera);
    _sprite.set_bg_priority(sprite_bg_priority);
    _update_sprite(false);
}

void player::update()
{
    int input_x = int(bn::keypad::right_held()) - int(bn::keypad::left_held());
    int input_y = int(bn::keypad::down_held()) - int(bn::keypad::up_held());
    bool moving = input_x || input_y;

    if(moving)
    {
        bn::fixed speed = bn::keypad::b_held() ? run_speed : walk_speed;

        if(input_x && input_y)
        {
            speed *= diagonal_factor;
        }

        bn::fixed dx = speed * input_x;
        bn::fixed dy = speed * input_y;

        if(dx != 0 && ! _move_axis(dx, 0) && dy == 0)
        {
            _slide_around_corner(dx, 0, speed);
        }

        if(dy != 0 && ! _move_axis(0, dy) && dx == 0)
        {
            _slide_around_corner(0, dy, speed);
        }

        _update_facing(input_x, input_y);
        _walk_counter += bn::keypad::b_held() ? 2 : 1;
    }
    else
    {
        _walk_counter = 0;
    }

    _update_sprite(moving);
}

bool player::_fits(bn::fixed x, bn::fixed y) const
{
    int feet_x = x.floor_integer();
    int feet_y = y.floor_integer();
    return world::area_free(feet_x + hitbox_left, feet_y + hitbox_top, feet_x + hitbox_right, feet_y);
}

bool player::_move_axis(bn::fixed dx, bn::fixed dy)
{
    // Try the full step, then smaller ones so the player ends up flush against walls.
    for(int attempt = 0; attempt < 3; ++attempt)
    {
        bn::fixed_point target(_position.x() + dx, _position.y() + dy);

        if(_fits(target.x(), target.y()))
        {
            _position = target;
            return attempt == 0;
        }

        dx /= 2;
        dy /= 2;
    }

    return false;
}

void player::_slide_around_corner(bn::fixed dx, bn::fixed dy, bn::fixed speed)
{
    // Walking straight into the edge of an obstacle: if a small sidestep would get past it,
    // nudge the player that way, like the top-down Zelda games do.
    bn::fixed x = _position.x();
    bn::fixed y = _position.y();
    bool horizontal = dx != 0;

    for(int offset = 1; offset <= corner_tolerance; ++offset)
    {
        bn::fixed ox = horizontal ? 0 : offset;
        bn::fixed oy = horizontal ? offset : 0;
        bool minus_free = _fits(x + dx - ox, y + dy - oy) && _fits(x - ox, y - oy);
        bool plus_free = _fits(x + dx + ox, y + dy + oy) && _fits(x + ox, y + oy);

        if(minus_free != plus_free)
        {
            bn::fixed step = bn::min(speed, bn::fixed(offset)) * (plus_free ? 1 : -1);
            _move_axis(horizontal ? 0 : step, horizontal ? step : 0);
            return;
        }

        if(minus_free && plus_free)
        {
            return;
        }
    }
}

void player::_update_facing(int input_x, int input_y)
{
    bool horizontal_ok = (input_x < 0 && _facing == facing::LEFT) || (input_x > 0 && _facing == facing::RIGHT);
    bool vertical_ok = (input_y < 0 && _facing == facing::UP) || (input_y > 0 && _facing == facing::DOWN);

    if(horizontal_ok || vertical_ok)
    {
        return;   // Keep facing the same way while sliding diagonally.
    }

    if(input_x)
    {
        _facing = input_x < 0 ? facing::LEFT : facing::RIGHT;
    }
    else
    {
        _facing = input_y < 0 ? facing::UP : facing::DOWN;
    }
}

void player::_update_sprite(bool moving)
{
    int first_frame = down_frames;

    switch(_facing)
    {

    case facing::UP:
        first_frame = up_frames;
        break;

    case facing::LEFT:
    case facing::RIGHT:
        first_frame = side_frames;
        break;

    default:
        break;
    }

    int frame = first_frame;

    if(moving)
    {
        frame += ((_walk_counter / walk_frame_ticks) + 1) % frames_per_direction;
    }

    if(frame != _frame)
    {
        _sprite.set_tiles(bn::sprite_items::warrior.tiles_item(), frame);
        _frame = frame;
    }

    _sprite.set_horizontal_flip(_facing == facing::RIGHT);

    // Whole pixels only, so the character never shimmers against the background.
    bn::fixed_point screen = world::to_screen_space(_position);
    _sprite.set_position(screen.x().floor_integer(), (screen.y() + sprite_offset_y).floor_integer());
}

}
