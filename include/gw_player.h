#ifndef GW_PLAYER_H
#define GW_PLAYER_H

#include "bn_camera_ptr.h"
#include "bn_fixed_point.h"

#include "gw_actor_sprite.h"

namespace gw
{

// The player character: free 8-direction movement with sub-pixel precision, collision against the
// world with corner sliding, and a 4-direction walk animation.
class player
{

public:
    // Hitbox around the feet, in pixels relative to the feet position. Small and low so the
    // character can walk close to walls and behind tree tops.
    static constexpr int hitbox_left = -5;
    static constexpr int hitbox_right = 4;
    static constexpr int hitbox_top = -5;

    // feet_position is in world pixels: the point between the character's feet.
    player(look_id look, const bn::fixed_point& feet_position, const bn::camera_ptr& camera);

    // Reads the keypad and moves the player. With input disabled the player stands still.
    // speed_percent scales walking and running (aspects, dazes).
    void update(bool input_enabled = true, bool can_run = true, int speed_percent = 100);

    [[nodiscard]] const bn::fixed_point& position() const
    {
        return _position;
    }

    // Moves the player instantly, for example through a door.
    void set_position(const bn::fixed_point& feet_position);

    [[nodiscard]] bool moving() const
    {
        return _moving;
    }

    [[nodiscard]] facing direction() const
    {
        return _facing;
    }

    void face(const bn::fixed_point& target);

    // Rushes towards target (Charge) until within stop_distance pixels or blocked.
    void dash_to(const bn::fixed_point& target, int stop_distance);

    // Jumps up to distance pixels the way the player last moved (Blink), stopping at walls.
    void blink(int distance);

    [[nodiscard]] bool dashing() const
    {
        return _dashing;
    }

    [[nodiscard]] actor_sprite& sprite()
    {
        return _sprite;
    }

private:
    bn::fixed_point _position;
    actor_sprite _sprite;
    bn::fixed_point _dash_target;
    facing _facing = facing::DOWN;
    int _last_x = 0;    // the last direction moved, from the keypad
    int _last_y = 1;
    int _walk_counter = 0;
    int _dash_stop = 0;
    int _dash_frames = 0;
    bool _moving = false;
    bool _dashing = false;

    [[nodiscard]] bool _fits(bn::fixed x, bn::fixed y) const;
    bool _move_axis(bn::fixed dx, bn::fixed dy);
    void _slide_around_corner(bn::fixed dx, bn::fixed dy, bn::fixed speed);
    void _update_facing(int input_x, int input_y);
    void _update_dash();
};

}

#endif
