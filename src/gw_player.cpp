#include "gw_player.h"

#include "bn_keypad.h"
#include "bn_math.h"
#include "bn_sprite_items_mount_horse.h"
#include "bn_sprite_items_mount_ram.h"
#include "bn_sprite_items_mount_saber.h"

#include "gw_night.h"
#include "gw_world.h"

namespace gw
{

namespace
{
    constexpr bn::fixed walk_speed = 1.25;
    constexpr bn::fixed run_speed = 2;
    constexpr bn::fixed dash_speed = 5;
    constexpr bn::fixed diagonal_factor = 0.7071;

    // How far (in pixels) the player is nudged around a corner they walk into.
    constexpr int corner_tolerance = 6;

    struct mount_look
    {
        const bn::sprite_item& sheet;
        int lift;       // how much higher the rider sits
    };

    // By race: a horse for humans, a ram for dwarves and a nightsaber for night elves.
    constexpr mount_look mount_looks[] = {
        { bn::sprite_items::mount_horse, 8 },
        { bn::sprite_items::mount_ram, 9 },
        { bn::sprite_items::mount_saber, 6 },
    };

    static_assert(sizeof(mount_looks) / sizeof(mount_looks[0]) == int(race_id::COUNT));

    // Mount sheet frames (tools/gen_characters.py): a side view, then towards and away from the viewer.
    constexpr int mount_side = 0;
    constexpr int mount_down = 4;
    constexpr int mount_up = 6;
    constexpr int mount_frame_ticks = 6;

    // Seen from the side the rider sits behind the mount's neck.
    constexpr int rider_back = 4;
    constexpr int feet_to_center = 13;
}

player::player(look_id look, const bn::fixed_point& feet_position, const bn::camera_ptr& camera) :
    _position(feet_position),
    _sprite(look, camera)
{
    _update_sprites(false);
}

void player::set_position(const bn::fixed_point& feet_position)
{
    _position = feet_position;
    _dashing = false;
    _update_sprites(false);
}

void player::mount(race_id race)
{
    const mount_look& look = mount_looks[int(race)];
    bn::sprite_ptr mount = look.sheet.create_sprite(0, 0);

    if(const bn::optional<bn::camera_ptr>& camera = _sprite.sprite().camera())
    {
        mount.set_camera(*camera);
    }

    mount.set_bg_priority(_sprite.sprite().bg_priority());
    _mount = bn::move(mount);
    _mount_item = &look.sheet;
    _mount_lift = look.lift;
    _mount_frame = 0;
    _update_sprites(false);
}

void player::dismount()
{
    _mount.reset();
    _sprite.set_offset(0, 0);
    _update_sprites(false);
}

void player::_update_sprites(bool moving)
{
    if(! _mount)
    {
        _sprite.update(_position, _facing, moving, _walk_counter);
        return;
    }

    // The rider sits still; the mount walks.
    bool side = _facing == facing::LEFT || _facing == facing::RIGHT;
    _sprite.set_offset(! side ? 0 : _facing == facing::LEFT ? rider_back : -rider_back, -_mount_lift);
    _sprite.update(_position, _facing, false, 0);

    int step = moving ? _walk_counter / mount_frame_ticks : 0;
    int frame = side ? mount_side + step % 4 : (_facing == facing::UP ? mount_up : mount_down) + step % 2;

    if(frame != _mount_frame)
    {
        _mount->set_tiles(_mount_item->tiles_item(), frame);
        _mount_frame = frame;
    }

    bn::fixed_point screen = world::to_screen_space(_position);
    _mount->set_horizontal_flip(_facing == facing::RIGHT);
    _mount->set_position(screen.x().floor_integer(), screen.y().floor_integer() - feet_to_center);
    _mount->set_visible(_sprite.sprite().visible());

    // In front of the rider, who is drawn at the same feet.
    _mount->set_z_order(-_position.y().floor_integer() - 1);
    _mount->set_blending_enabled(night::enabled());
}

void player::face(const bn::fixed_point& target)
{
    _facing = facing_towards(_position, target);
}

void player::dash_to(const bn::fixed_point& target, int stop_distance)
{
    _dash_target = target;
    _dash_stop = stop_distance;
    _dash_frames = 0;
    _dashing = true;
    face(target);
}

void player::blink(int distance)
{
    bn::fixed step_x = _last_x;
    bn::fixed step_y = _last_y;

    if(_last_x && _last_y)
    {
        step_x *= diagonal_factor;
        step_y *= diagonal_factor;
    }

    for(int moved = 0; moved < distance; moved += 2)
    {
        bn::fixed_point next(_position.x() + step_x * 2, _position.y() + step_y * 2);

        if(! _fits(next.x(), next.y()))
        {
            break;
        }

        _position = next;
    }

    _dashing = false;
    _update_sprites(false);
}

void player::knock_back(const bn::fixed_point& from, int distance)
{
    // Only the direction matters: kept small so its square stays in range.
    bn::fixed dx = bn::clamp(_position.x() - from.x(), bn::fixed(-256), bn::fixed(256));
    bn::fixed dy = bn::clamp(_position.y() - from.y(), bn::fixed(-256), bn::fixed(256));
    bn::fixed length = bn::sqrt(dx * dx + dy * dy);

    if(length < 1)
    {
        dx = 0;
        dy = 1;
        length = 1;
    }

    bn::fixed step_x = dx * 2 / length;
    bn::fixed step_y = dy * 2 / length;

    for(int moved = 0; moved < distance; moved += 2)
    {
        bn::fixed_point next(_position.x() + step_x, _position.y() + step_y);

        if(! _fits(next.x(), next.y()))
        {
            break;
        }

        _position = next;
    }

    _dashing = false;
    _update_sprites(false);
}

void player::update(bool input_enabled, bool can_run, int speed_percent, const bn::fixed_point* flee_from)
{
    if(_dashing)
    {
        _update_dash();
        _update_sprites(true);
        return;
    }

    int input_x = 0;
    int input_y = 0;

    if(flee_from)
    {
        bn::fixed dx = _position.x() - flee_from->x();
        bn::fixed dy = _position.y() - flee_from->y();
        input_x = dx > 4 ? 1 : dx < -4 ? -1 : 0;
        input_y = dy > 4 ? 1 : dy < -4 ? -1 : 0;

        if(! input_x && ! input_y)
        {
            input_x = _last_x;
            input_y = _last_y;
        }
    }
    else if(input_enabled)
    {
        input_x = int(bn::keypad::right_held()) - int(bn::keypad::left_held());
        input_y = int(bn::keypad::down_held()) - int(bn::keypad::up_held());
    }

    bool moving = input_x || input_y;
    bool running = can_run && ! flee_from && bn::keypad::b_held();
    _moving = moving;

    if(moving)
    {
        _last_x = input_x;
        _last_y = input_y;
        bn::fixed speed = running ? run_speed : walk_speed;

        if(speed_percent != 100)
        {
            speed = speed * speed_percent / 100;
        }

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
        _walk_counter += running ? 2 : 1;
    }
    else
    {
        _walk_counter = 0;
    }

    _update_sprites(moving);
}

void player::_update_dash()
{
    bn::fixed dx = _dash_target.x() - _position.x();
    bn::fixed dy = _dash_target.y() - _position.y();
    bn::fixed length = bn::sqrt(dx * dx + dy * dy);
    ++_dash_frames;
    _walk_counter += 3;

    if(length <= _dash_stop || _dash_frames > 60)
    {
        _dashing = false;
        return;
    }

    bn::fixed step = bn::min(dash_speed, length - _dash_stop);
    bn::fixed sx = dx * step / length;
    bn::fixed sy = dy * step / length;
    bool moved_x = sx == 0 || _move_axis(sx, 0);
    bool moved_y = sy == 0 || _move_axis(0, sy);

    if(! moved_x && ! moved_y)
    {
        _dashing = false;
    }
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

}
