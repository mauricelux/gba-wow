#ifndef GW_PLAYER_H
#define GW_PLAYER_H

#include "bn_camera_ptr.h"
#include "bn_fixed_point.h"
#include "bn_sprite_ptr.h"

namespace gw
{

// The player character: free 8-direction movement with sub-pixel precision, collision
// against the world with corner sliding, and a 4-direction walk animation.
class player
{

public:
    // Hitbox around the feet, in pixels relative to the feet position. Small and low so the
    // character can walk close to walls and behind tree tops.
    static constexpr int hitbox_left = -5;
    static constexpr int hitbox_right = 4;
    static constexpr int hitbox_top = -5;

    // feet_position is in world pixels: the point between the character's feet.
    player(const bn::fixed_point& feet_position, const bn::camera_ptr& camera);

    // Reads the keypad and moves the player. With input disabled the player stands still.
    void update(bool input_enabled = true);

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

private:
    enum class facing
    {
        DOWN,
        UP,
        LEFT,
        RIGHT
    };

    bn::fixed_point _position;
    bn::sprite_ptr _sprite;
    facing _facing = facing::DOWN;
    int _walk_counter = 0;
    int _frame = -1;
    bool _moving = false;

    [[nodiscard]] bool _fits(bn::fixed x, bn::fixed y) const;
    bool _move_axis(bn::fixed dx, bn::fixed dy);
    void _slide_around_corner(bn::fixed dx, bn::fixed dy, bn::fixed speed);
    void _update_facing(int input_x, int input_y);
    void _update_sprite(bool moving);
};

}

#endif
