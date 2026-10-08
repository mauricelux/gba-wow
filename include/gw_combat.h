#ifndef GW_COMBAT_H
#define GW_COMBAT_H

#include "bn_fixed_point.h"
#include "bn_vector.h"

#include "gw_abilities.h"
#include "gw_character.h"
#include "gw_effects.h"

namespace gw
{

class player;
class enemies;
class floating_texts;
class hud;
struct enemy;

enum class buff_id : uint8_t
{
    BATTLE_SHOUT,
    FROST_ARMOR,
    LAST_STAND,
    ICE_BARRIER,
    ARCANE_POWER,
    ASPECT_OF_THE_HAWK,
    BESTIAL_WRATH,
    WELL_FED,       // eating or drinking: restores health and mana quickly
    COUNT
};

// The player's side of fighting: targeting, auto-attack, abilities, casting, buffs, resources,
// regeneration, experience and death. Enemies call back into it when they swing or die.
class combat
{

public:
    combat(player& player_ref, enemies& enemies_ref, floating_texts& texts, effects& fx, hud& hud_ref);

    // Reads targeting and ability input (unless disabled) and advances timers.
    void update(bool input_enabled);

    // A: target the nearest enemy and start attacking it.
    void engage();

    [[nodiscard]] int target() const
    {
        return _target;
    }

    void clear_target();

    [[nodiscard]] bool in_combat() const;

    [[nodiscard]] bool dead() const
    {
        return _dead;
    }

    [[nodiscard]] const stats& player_stats() const
    {
        return _stats;
    }

    // Frames until the ability can be used again (global cooldown included).
    [[nodiscard]] int cooldown(ability_id ability) const;

    [[nodiscard]] bool usable(ability_id ability) const;

    [[nodiscard]] ability_id casting() const
    {
        return _cast_ability;
    }

    [[nodiscard]] int cast_progress() const;    // 0..100

    [[nodiscard]] int buff_frames(buff_id buff) const
    {
        return _buffs[int(buff)];
    }

    // Called by enemies. percent scales the hit (special attacks).
    void enemy_attacks(int index, int percent = 100);
    void enemy_killed(int index);

    // Special abilities of elites and bosses, every frame while they fight. Returns true when the
    // enemy is busy and skips its normal movement and swings this frame.
    bool boss_update(int index);

    // Deals damage to an enemy with floating text. Returns true if it died.
    bool damage_enemy(int index, int amount, bool crit, bool periodic = false);

    // Deals damage to the player (boss specials).
    void damage_player(int amount, const bn::fixed_point& from);

    void heal_player(int amount);

    void gain_xp(int amount);

    // After dying: back to life at half health and mana.
    void revive();

    void on_map_change();

    // Recomputes stats after gear, level or buff changes.
    void refresh_stats();

    // Food and drink: restore health and mana over 18 seconds while standing still.
    void start_eating(int health, int mana);

    // Uses food, drink or a potion from the bags. When it can't, the reason goes to error if given,
    // otherwise to the HUD.
    bool use_item(item_id item, const char** error = nullptr);

    // Select: a potion in combat, otherwise food when hurt or a drink when low on mana.
    bool quick_use();

    // Called when an enemy dies with the killer being the player. Set by the game (quests, loot).
    void (*on_kill)(void* context, int index) = nullptr;
    void (*on_level_up)(void* context) = nullptr;
    void* callback_context = nullptr;

private:
    player& _player;
    enemies& _enemies;
    floating_texts& _texts;
    effects& _effects;
    hud& _hud;
    stats _stats;
    int _target = -1;
    bool _auto_attack = false;
    bool _dead = false;
    int _swing_timer = 0;
    int _ranged_timer = 0;
    int _gcd = 0;
    int _cooldowns[ability_count] = {};
    int _buffs[int(buff_id::COUNT)] = {};
    int _buff_values[int(buff_id::COUNT)] = {};
    ability_id _queued = ability_id::NONE;   // Heroic Strike waits for the next swing
    ability_id _cast_ability = ability_id::NONE;
    int _cast_frames = 0;
    int _cast_total = 0;
    int _combat_frames = 3600;               // frames since the last hit dealt or taken
    int _since_cast = 999;                   // frames since mana was spent (five second rule)
    int _regen_timer = 0;
    int _potion_cooldown = 0;
    int _out_of_reach_frames = 0;
    int _eat_health = 0;
    int _eat_mana = 0;
    bool _charging = false;
    int _charge_target = -1;
    bn::vector<projectile_hit, 8> _arrived;

    void _read_input();
    void _cycle_target();
    void _use_slot(int slot);
    bool _use_ability(ability_id ability);
    void _finish_cast();
    void _apply_ability(ability_id ability, int target);
    void _update_auto_attack();
    void _update_buffs();
    void _update_regen();
    void _update_projectiles();
    void _apply_hit(const projectile_hit& hit);
    void _melee_swing(int index);
    void _ranged_shot(int index);
    [[nodiscard]] int _roll_spell(ability_id ability, bool& crit) const;
    [[nodiscard]] int _roll_weapon(bool& crit) const;
    [[nodiscard]] bool _roll_miss(int index) const;
    [[nodiscard]] bool _target_valid() const;
    [[nodiscard]] int _target_distance() const;
    [[nodiscard]] bn::fixed_point _head(const bn::fixed_point& feet, int height) const;
    void _summon_add(const enemy& boss, enemy_id add);
    bool _update_telegraph(enemy& boss, bool around_boss, int radius);
    void _gain_rage(int damage, bool dealt);
    void _spend(int cost);
    [[nodiscard]] int _power() const;
    void _set_buff(buff_id buff, int frames, int value);
    void _die();
    void _area(ability_id ability, int radius);
};

}

#endif
