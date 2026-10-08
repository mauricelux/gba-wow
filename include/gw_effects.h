#ifndef GW_EFFECTS_H
#define GW_EFFECTS_H

#include "bn_camera_ptr.h"
#include "bn_fixed_point.h"
#include "bn_optional.h"
#include "bn_sprite_ptr.h"
#include "bn_vector.h"

#include "gw_abilities.h"

namespace gw
{

// A spell or arrow on its way to an enemy. When it arrives the game applies its payload.
struct projectile_hit
{
    int target;
    int damage;
    bool crit;
    ability_id ability;
};

// Short-lived sprites: the target ring, hit sparks, bursts, projectiles and area circles.
class effects
{

public:
    explicit effects(const bn::camera_ptr& camera);

    // Shows the ring under the target's feet, or hides it with nullptr.
    void set_target(const bn::fixed_point* feet, bool hostile);

    void spark(const bn::fixed_point& world_position);

    void burst(const bn::fixed_point& world_position, projectile_kind kind);

    // An area circle of the given radius in pixels that lasts for frames.
    void circle(const bn::fixed_point& world_position, int radius, int frames, bool frost);

    void launch(const bn::fixed_point& from, projectile_kind kind, const projectile_hit& hit);

    // Moves projectiles towards their targets. target_position returns where a target is, or nullptr
    // if it is gone; arrived hits are appended to hits.
    template<typename TargetPosition>
    void update(TargetPosition target_position, bn::ivector<projectile_hit>& hits)
    {
        for(auto it = _projectiles.begin(); it != _projectiles.end();)
        {
            const bn::fixed_point* target = target_position(it->hit.target);

            if(! target)
            {
                it = _projectiles.erase(it);
                continue;
            }

            if(_move_projectile(*it, *target))
            {
                hits.push_back(it->hit);
                burst(it->position, it->kind);
                it = _projectiles.erase(it);
                continue;
            }

            ++it;
        }

        _update_effects();
    }

    void clear();

private:
    struct projectile
    {
        bn::sprite_ptr sprite;
        bn::fixed_point position;
        projectile_kind kind;
        projectile_hit hit;
        int frames = 0;
    };

    struct timed_sprite
    {
        bn::sprite_ptr sprite;
        bn::fixed_point position;
        int frames = 0;
        int first_frame = 0;
        int frame_count = 1;
        int lifetime = 0;
    };

    bn::camera_ptr _camera;
    bn::optional<bn::sprite_ptr> _target_ring;
    bn::vector<projectile, 8> _projectiles;
    bn::vector<timed_sprite, 8> _sparks;
    bn::vector<timed_sprite, 3> _circles;

    bool _move_projectile(projectile& item, const bn::fixed_point& target);
    void _update_effects();
    void _place(bn::sprite_ptr& sprite, const bn::fixed_point& world_position);
};

}

#endif
