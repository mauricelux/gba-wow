#ifndef GW_PET_H
#define GW_PET_H

#include "bn_camera_ptr.h"
#include "bn_fixed_point.h"
#include "bn_optional.h"

#include "gw_abilities.h"
#include "gw_actor_sprite.h"
#include "gw_ids.h"

namespace gw
{

class combat;
class enemies;
class floating_texts;
class hud;
class player;

// The player's companion: a Beast Mastery hunter's tamed beast, which is saved and levels with them,
// or a Frost mage's Water Elemental for 45 seconds. It follows the player, fights what they fight
// (unless passive) and holds the attention of what it bites. Enemies fighting it swing at it instead
// of the player.
class pet
{

public:
    pet(const bn::camera_ptr& camera, player& player_ref, enemies& enemies_ref, floating_texts& texts,
        hud& hud_ref);

    void set_combat(combat& combat_ref)
    {
        _combat = &combat_ref;
    }

    // Every frame, after the player and the enemies moved.
    void update(bool player_alive);

    // Puts the pet beside the player: a new map, a call or a revive.
    void place();

    // Out, alive and beside the player: it fights, and enemies can fight it.
    [[nodiscard]] bool active() const;

    // A hunter's pet exists (alive or dead, out or away).
    [[nodiscard]] bool tamed() const;

    [[nodiscard]] bool dead() const;

    [[nodiscard]] bool elemental() const
    {
        return _elemental_frames > 0;
    }

    [[nodiscard]] const bn::fixed_point& position() const
    {
        return _position;
    }

    // Pixels from the feet to the top of the head, for floating text.
    [[nodiscard]] int height() const;

    [[nodiscard]] int health() const;
    [[nodiscard]] int max_health() const;

    // Hits the pet. Returns true if that killed it.
    bool damage(int amount);

    // Taming the Beast: the beast becomes the pet, at full health.
    void tame(enemy_id species);

    // Call Pet: out if it's away, away if it's out. Returns false with a reason when it can't.
    bool call(const char** reason);

    // Revive Pet: back with a share of its health.
    void revive(int percent);

    // Returns true when the pet is now passive.
    bool toggle_passive();

    // Mend Pet: heals this much over 15 seconds.
    void mend(int total);

    // Kill Command and Intimidation: the pet's next bite on target hits for bonus or stuns.
    void command(ability_id ability, int target, int value);

    // Summon Water Elemental: a frost pet that casts at the player's target for frames.
    void summon_elemental(int frames);

    // The player died or left: a Water Elemental goes away.
    void end_elemental();

private:
    bn::camera_ptr _camera;
    player& _player;
    enemies& _enemies;
    floating_texts& _texts;
    hud& _hud;
    combat* _combat = nullptr;
    bn::optional<actor_sprite> _sprite;
    look_id _look = look_id::YOUNG_WOLF;
    bn::fixed_point _position;
    facing _direction = facing::DOWN;
    bool _moving = false;
    bool _placed = false;
    bool _visible = false;      // the player is alive
    int _walk_counter = 0;
    int _attack_timer = 0;
    int _target = -1;
    int _regen_timer = 0;
    int _corpse_frames = 0;     // a dead pet lies there a few seconds
    int _mend_total = 0;        // left to heal
    int _mend_ticks = 0;
    int _mend_timer = 0;
    ability_id _command = ability_id::NONE;
    int _command_target = -1;
    int _command_value = 0;
    int _command_frames = 0;
    int _elemental_frames = 0;
    int _elemental_health = 0;
    int _freeze_cooldown = 0;

    [[nodiscard]] int _level() const;
    void _set_health(int health);
    [[nodiscard]] int _pick_target() const;
    void _follow();
    bool _move_towards(const bn::fixed_point& target, bn::fixed speed, int stop_distance);
    [[nodiscard]] bn::fixed _speed() const;
    void _bite(int index);
    void _cast(int index);
    void _update_sprite();
};

}

#endif
