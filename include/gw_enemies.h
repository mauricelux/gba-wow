#ifndef GW_ENEMIES_H
#define GW_ENEMIES_H

#include "bn_camera_ptr.h"
#include "bn_fixed_point.h"
#include "bn_optional.h"
#include "bn_sprite_affine_mat_ptr.h"
#include "bn_sprite_ptr.h"
#include "bn_vector.h"

#include "gw_actor_sprite.h"
#include "gw_enemy_data.h"
#include "gw_items.h"
#include "gw_maps.h"

namespace gw
{

class combat;

enum class enemy_state : uint8_t
{
    IDLE,       // standing around its spawn point, sometimes wandering
    CHASE,      // fighting the player
    EVADE,      // running home after being pulled too far; immune
    DEAD,       // a corpse that can be looted
    GONE        // waiting to respawn
};

// Why an enemy stands there doing nothing; damage ends all of them.
enum class incapacitate_kind : uint8_t
{
    NONE,
    POLYMORPH,
    FROZEN,     // Freezing Trap
    DISORIENTED,
    ASLEEP      // Wyvern Sting: poisons it when it wakes
};

struct loot_slot
{
    item_id item = item_id::NONE;
    uint8_t count = 0;
};

struct enemy
{
    enemy_id id = enemy_id::NONE;
    const enemy_def* def = nullptr;
    bn::fixed_point spawn;
    bn::fixed_point position;
    bn::fixed_point wander_target;
    int health = 0;
    int max_health = 0;
    int damage = 0;             // average damage per swing
    uint8_t level = 1;
    enemy_state state = enemy_state::IDLE;
    facing direction = facing::DOWN;
    bool moving = false;
    bool wandering = false;
    bool tapped = false;        // the player damaged it, so it gives experience and loot
    int walk_counter = 0;
    int attack_timer = 0;
    int state_timer = 0;
    int stuck_frames = 0;
    // Debuffs
    int stun_frames = 0;
    int root_frames = 0;
    int slow_frames = 0;
    int slow_percent = 0;
    int dot_damage = 0;         // per tick
    int dot_ticks = 0;
    int dot_timer = 0;
    int marked_frames = 0;      // Hunter's Mark
    int marked_percent = 0;
    int incapacitate_frames = 0;
    incapacitate_kind incapacitated = incapacitate_kind::NONE;
    int sting_damage = 0;       // Wyvern Sting's poison, waiting for the target to wake
    int fear_frames = 0;        // runs from the player
    int weaken_frames = 0;      // deals less damage (Demoralizing Shout)
    int weaken_percent = 0;
    int disarm_frames = 0;      // deals half damage
    int sunder_frames = 0;      // takes more physical damage
    int sunder_stacks = 0;
    int sunder_percent = 0;     // per stack
    int scorch_frames = 0;      // takes more fire damage
    int scorch_stacks = 0;
    int silence_frames = 0;     // can't cast
    // Boss fights
    int phase = 0;
    int special_timer = 0;
    int telegraph_frames = 0;   // a marked area goes off when this reaches zero
    bn::fixed_point special_position;
    bool summoned = false;      // added during a fight, removed when it ends
    // Table abilities (gw_enemy_abilities): casting, charging, fleeing and its own buffs
    enemy_ability_state ai;
    // Loot, rolled on death
    loot_slot loot[4];
    int loot_money = 0;
    bn::optional<actor_sprite> sprite;
    bn::optional<bn::sprite_ptr> sparkle;   // over a corpse that still has loot
    bn::optional<bn::sprite_ptr> status;    // over an enemy that can't act: what holds it
    bn::optional<bn::sprite_ptr> cast_bar;  // over an enemy casting

    [[nodiscard]] bool alive() const
    {
        return state == enemy_state::IDLE || state == enemy_state::CHASE || state == enemy_state::EVADE;
    }

    [[nodiscard]] bool elite() const
    {
        return def->flags & enemy_flag::ELITE;
    }

    [[nodiscard]] bool boss() const
    {
        return def->flags & enemy_flag::BOSS;
    }

    [[nodiscard]] bool casting() const
    {
        return ai.casting != enemy_ability_id::NONE;
    }

    [[nodiscard]] int health_percent() const
    {
        return health * 100 / max_health;
    }

    // Frames between swings; enraged enemies swing faster.
    [[nodiscard]] int swing_frames() const
    {
        return def->attack_speed * (ai.enraged ? 4 : 6);
    }

    [[nodiscard]] bool has_loot() const;

    // Polymorphed, frozen, asleep or feared: it doesn't fight back.
    [[nodiscard]] bool controlled() const
    {
        return incapacitate_frames > 0 || fear_frames > 0;
    }

    // Frozen in place (Frost Nova, Freezing Trap), for shatter combos.
    [[nodiscard]] bool frozen() const
    {
        return root_frames > 0 || (incapacitate_frames > 0 && incapacitated == incapacitate_kind::FROZEN);
    }
};

// Every enemy on the current map: spawning, wandering, chasing the player, leashing back home,
// dying and respawning. Only enemies near the player think and have sprites.
class enemies
{

public:
    static constexpr int max_enemies = 104;

    explicit enemies(const bn::camera_ptr& camera);

    void set_combat(combat& combat_ref)
    {
        _combat = &combat_ref;
    }

    void load(const map_info& map);

    void update(const bn::fixed_point& player_feet, bool player_alive);

    [[nodiscard]] int count() const
    {
        return _enemies.size();
    }

    [[nodiscard]] enemy& at(int index)
    {
        return _enemies[index];
    }

    [[nodiscard]] const enemy& at(int index) const
    {
        return _enemies[index];
    }

    // The closest living enemy within max_distance pixels, skipping exclude; -1 if none. With
    // fighting_only, only enemies fighting the player count.
    [[nodiscard]] int nearest(const bn::fixed_point& from, int max_distance, int exclude = -1,
                              bool fighting_only = false) const;

    // The closest lootable corpse within max_distance pixels; -1 if none.
    [[nodiscard]] int nearest_corpse(const bn::fixed_point& from, int max_distance) const;

    // Makes the enemy (and its friends nearby) fight the player.
    void aggro(int index);

    // Applies damage to the enemy. Returns true if it died.
    bool damage(int index, int amount);

    // Holds the enemy for frames; damage breaks it. Bosses can't be held. Returns false if immune.
    bool incapacitate(int index, incapacitate_kind kind, int frames);

    // Ends a hold early, as damage does (Wyvern Sting's poison then starts).
    void break_control(int index);

    // Adds an enemy during a fight (boss adds). Returns its index or -1.
    int summon(enemy_id id, const bn::fixed_point& position);

    // Whether an enemy could stand with its feet at the position.
    [[nodiscard]] static bool fits(bn::fixed x, bn::fixed y);

    // Walks the enemy towards target until within stop_distance pixels on both axes. Returns true
    // when there.
    bool move_towards(enemy& item, const bn::fixed_point& target, bn::fixed speed, int stop_distance);

    // How fast the enemy runs while fighting, slows included.
    [[nodiscard]] bn::fixed chase_speed(const enemy& item) const
    {
        return _speed(item, true);
    }

    // The closest enemy of the same family as index standing around (not fighting) within radius
    // pixels of it; -1 if none.
    [[nodiscard]] int idle_friend(int index, int radius) const;

    // True while any enemy is fighting the player.
    [[nodiscard]] bool any_in_combat() const;

    // True while an elite or a boss is fighting the player.
    [[nodiscard]] bool elite_in_combat() const;

    // Called when the player dies: everyone goes home.
    void reset_combat();

    // Hides corpses that were looted.
    void corpse_looted(int index);

private:
    bn::camera_ptr _camera;
    combat* _combat = nullptr;
    bn::vector<enemy, max_enemies> _enemies;
    bn::optional<bn::sprite_affine_mat_ptr> _small;
    int _frame = 0;

    void _spawn(enemy& item);
    void _update_enemy(int index, const bn::fixed_point& player_feet, bool player_alive);
    void _update_sprite(enemy& item, const bn::fixed_point& player_feet);
    void _update_status(enemy& item);
    void _update_cast_bar(enemy& item, const bn::fixed_point& screen);
    void _start_fight(enemy& item, int first_swing);
    [[nodiscard]] int _reach(const enemy& item, const bn::fixed_point& player_feet) const;
    [[nodiscard]] bn::fixed _speed(const enemy& item, bool chasing) const;
    [[nodiscard]] int _aggro_radius(const enemy& item) const;
    void _die(int index);
};

}

#endif
