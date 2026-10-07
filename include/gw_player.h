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
    // feet_position is in world pixels: the point between the character's feet.
    player(const bn::fixed_point& feet_position, const bn::camera_ptr& camera);

    void update();

    [[nodiscard]] const bn::fixed_point& position() const
    {
        return _position;
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

    [[nodiscard]] bool _fits(bn::fixed x, bn::fixed y) const;
    bool _move_axis(bn::fixed dx, bn::fixed dy);
    void _slide_around_corner(bn::fixed dx, bn::fixed dy, bn::fixed speed);
    void _update_facing(int input_x, int input_y);
    void _update_sprite(bool moving);
};

}

#endif
