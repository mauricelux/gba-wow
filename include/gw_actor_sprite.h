#ifndef GW_ACTOR_SPRITE_H
#define GW_ACTOR_SPRITE_H

#include "bn_camera_ptr.h"
#include "bn_fixed_point.h"
#include "bn_sprite_ptr.h"

#include "gw_looks.h"
#include "gw_types.h"

namespace gw
{

// The animated sprite of a character (player, NPC or enemy) standing on the map. It picks the frame
// from the facing, walking, attacking and dead states and keeps characters lower on screen in front.
class actor_sprite
{

public:
    actor_sprite(look_id look, const bn::camera_ptr& camera, bn::fixed scale = 1);

    void set_look(look_id look);

    // feet is in world pixels.
    void update(const bn::fixed_point& feet, facing direction, bool moving, int walk_counter);

    // Shows the attack frame for a few frames.
    void play_attack();

    void set_casting(bool casting)
    {
        _casting = casting;
    }

    void set_dead(bool dead)
    {
        _dead = dead;
    }

    void set_visible(bool visible);

    // Flashes the sprite white for a few frames, for example when hit.
    void flash();

    [[nodiscard]] bn::sprite_ptr& sprite()
    {
        return _sprite;
    }

    // Pixels from the feet to the top of the head, for markers and floating text.
    [[nodiscard]] int height() const;

private:
    bn::sprite_ptr _sprite;
    const look_def* _look;
    bn::fixed _scale;
    int _frame = -1;
    int _attack_frames = 0;
    int _flash_frames = 0;
    bool _casting = false;
    bool _dead = false;
    bool _flipped = false;
};

}

#endif
