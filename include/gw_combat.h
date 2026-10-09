#ifndef GW_COMBAT_H
#define GW_COMBAT_H

#include "bn_fixed_point.h"
#include "bn_string.h"
#include "bn_vector.h"

#include "gw_abilities.h"
#include "gw_buffs.h"
#include "gw_character.h"
#include "gw_effects.h"

namespace gw
{

class player;
class pet;
class enemies;
class floating_texts;
class hud;
struct enemy;

// The bar whose button is held, which the hud shows around its cross.
enum class held_bar : uint8_t
{
    NONE,
    COMBAT,     // R
    UTILITY,    // L
    BUFFS,      // L and R
    ITEMS       // Select
};

// Where a spell or a trap acts on the ground, for a while.
struct ground_zone
{
    bn::fixed_point position;
    ability_id ability = ability_id::NONE;
    int radius = 0;
    int frames = 0;         // left
    int tick = 0;           // frames until the next tick
    int value = 0;          // damage per tick, or what a trap does
    int circle = 0;         // effects circle id
    bool channel = false;   // ends when the channel stops
    bool trap = false;      // waits for an enemy to step on it
};

// The player's side of fighting: targeting, auto-attack, abilities, casting, buffs, resources,
// regeneration, experience and death. Enemies call back into it when they swing or die.
class combat
{

public:
    combat(player& player_ref, enemies& enemies_ref, floating_texts& texts, effects& fx, hud& hud_ref);

    void set_pet(pet& pet_ref)
    {
        _pet = &pet_ref;
    }

    [[nodiscard]] const pet* companion() const
    {
        return _pet;
    }

    // Reads targeting, bar and item input (unless disabled) and advances timers.
    void update(bool input_enabled);

    [[nodiscard]] held_bar shown_bar() const
    {
        return _held_bar;
    }

    // Whether L, R or Select hold the buttons for a bar, so they don't move or talk.
    [[nodiscard]] static bool bar_keys_held();

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

    // What a buff gives while it lasts (Bestial Wrath's haste), 0 when it's off.
    [[nodiscard]] int buff_value(buff_id buff) const
    {
        return _buffs[int(buff)] > 0 ? _buff_values[int(buff)] : 0;
    }

    // Called by enemies. percent scales the hit (special attacks). Returns true if it landed.
    bool enemy_attacks(int index, int percent = 100);

    // An enemy fighting the pet swings at it. Returns true if it landed.
    bool enemy_attacks_pet(int index);

    // The pet bites (damage 0 misses). Returns true if the enemy died.
    bool pet_hits(int index, int damage, bool crit);

    // The Water Elemental casts Frostbolt at the enemy.
    void pet_casts(int index, int damage, bool crit);

    // The Water Elemental's Freeze: roots the enemies around center.
    void pet_freeze(const bn::fixed_point& center, int radius);
    void enemy_killed(int index);

    // Special abilities of elites and bosses, every frame while they fight. Returns true when the
    // enemy is busy and skips its normal movement and swings this frame.
    bool boss_update(int index);

    // Table abilities of an enemy fighting the player, every frame (gw_enemy_ai.cpp): casts, strikes,
    // charges, buffs and running for help. Returns true while the enemy is busy and skips its normal
    // movement and swings this frame.
    bool enemy_ai_update(int index);

    // Stops what the enemy is casting. Interrupted, the ability waits a few seconds and the player
    // sees it.
    void stop_enemy_cast(int index, bool interrupted);

    // Asleep or polymorphed: enemies hold their swings so they don't wake the player.
    [[nodiscard]] bool player_incapacitated() const
    {
        return _buffs[int(buff_id::ASLEEP)] || _buffs[int(buff_id::POLYMORPHED)];
    }

    // While feared, where the player runs from; nullptr otherwise.
    [[nodiscard]] const bn::fixed_point* fear_source() const
    {
        return _buffs[int(buff_id::FEARED)] ? &_fear_from : nullptr;
    }

    // Deals damage to an enemy with floating text. Returns true if it died.
    bool damage_enemy(int index, int amount, bool crit, bool periodic = false,
                      school damage_school = school::PHYSICAL);

    // Deals damage to the player (boss specials).
    void damage_player(int amount, const bn::fixed_point& from, school damage_school = school::PHYSICAL);

    // Movement speed in percent, from aspects, the mount and dazes; 0 while the player can't move.
    [[nodiscard]] int speed_percent() const;

    // Gets off the mount: fighting, getting hit, using an ability, going indoors or taking a flight.
    void dismount();

    // Set by Teleport: Stormwind; the game moves the player there and clears it.
    map_id teleport_map = map_id::NONE;
    bn::fixed_point teleport_point;

    void heal_player(int amount);

    void gain_xp(int amount);

    // After dying: back to life at half health and mana.
    void revive();

    void on_map_change();

    // Recomputes stats after gear, level or buff changes.
    void refresh_stats();

    // Food and drink: restore health and mana over 18 seconds while standing still.
    void start_eating(int health, int mana);

    // Casts a line out of combat: a three second channel that moving or a hit stops. Returns false
    // (with a message) when it can't.
    bool start_fishing();

    // Set when a Fishing channel ends; the game rolls the catch and clears it.
    bool fish_caught = false;

    // Uses food, drink, a potion or the hearthstone from the bags. When it can't, the reason goes to
    // error if given, otherwise to the HUD.
    bool use_item(item_id item, const char** error = nullptr);

    // Frames until a potion can be drunk again.
    [[nodiscard]] int potion_cooldown() const
    {
        return _potion_cooldown;
    }

    // A long buff (shout, intellect, armor, aspect) the player knows but lacks, outside combat; NONE
    // otherwise. The hud blinks its icon as a reminder.
    [[nodiscard]] ability_id missing_buff() const;

    // Tapping Select: a potion in combat, otherwise food when hurt or a drink when low on mana.
    bool quick_use();

    // Holding Select and pressing a direction: the item of that Items bar slot.
    bool use_item_slot(int slot);

    // Called when an enemy dies with the killer being the player. Set by the game (quests, loot).
    void (*on_kill)(void* context, int index) = nullptr;
    void (*on_level_up)(void* context) = nullptr;
    void (*on_quest_progress)(void* context) = nullptr;    // taming a beast
    void* callback_context = nullptr;

private:
    player& _player;
    pet* _pet = nullptr;
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
    ability_id _queued = ability_id::NONE;   // Heroic Strike and Cleave wait for the next swing
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
    int _dodged_frames = 0;                  // the player dodged or blocked: Revenge, Mongoose Bite
    int _overpower_frames = 0;               // the target dodged: Overpower
    int _feign_frames = 0;                   // lying still after Feign Death
    int _polymorph_target = -1;
    int _combustion_crits = 0;
    int _tame_target = -1;                   // the beast Tame Beast channels on
    bool _pet_hit = false;                   // damage_enemy: the pet deals it
    bn::vector<projectile_hit, 8> _arrived;
    bn::vector<ground_zone, 4> _zones;

    held_bar _held_bar = held_bar::NONE;
    int _r_frames = 0;                       // frames R has been held
    int _l_frames = 0;                       // the same for L and Select
    int _select_frames = 0;
    bool _chord = false;                     // L and R pressed together: the Buffs bar
    bool _l_used = true;                     // L did something while held, so releasing it doesn't target
    bool _select_used = true;                // the same for Select and its quick use
    bn::string<24> _hearth_text;             // "Ready in N minutes", kept for use_item's error
    bn::fixed_point _fear_from;              // who feared the player

    void _read_input();
    void _cycle_target();
    void _use_slot(bar_id bar, int slot);
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
    bool _update_telegraph(enemy& boss, bool around_boss, int radius, const char* name, projectile_kind kind);
    bool _update_wind_up(int index, bool in_melee, const char* name, const char* message);
    void _update_frenzy(enemy& boss, int health_percent, int below, int phase, const char* name);
    void _shadow_port(int index);
    void _launch_bomb();
    void _boss_greeting(enemy& boss, const char* message);
    void _boss_killed(const enemy& boss);
    [[nodiscard]] int _find_enemy(enemy_id id) const;
    void _gain_rage(int damage, bool dealt);
    void _spend(int cost);
    [[nodiscard]] int _power() const;
    [[nodiscard]] int _cost(ability_id ability) const;
    [[nodiscard]] int _cast_time(ability_id ability) const;
    [[nodiscard]] int _cooldown_frames(ability_id ability) const;
    [[nodiscard]] const char* _unusable_reason(ability_id ability) const;
    void _set_buff(buff_id buff, int frames, int value);
    void _end_buff(buff_id buff);
    void _die();
    void _area(ability_id ability, int radius);
    void _weapon_strike(int index, ability_id ability, int bonus, bool can_miss = true);
    void _spell_hit(int index, ability_id ability, int damage, bool crit);
    [[nodiscard]] bool _roll_crit(ability_id ability, int index) const;
    void _update_channel();
    void _update_zones();
    void _add_zone(const ground_zone& zone);
    void _spring_trap(ground_zone& trap, int index);
    void _interrupt(int index);
    void _polymorph(int index, int frames);
    void _set_armor(buff_id armor, ability_id ability);
    void _set_aspect(buff_id aspect, ability_id ability);
    void _conjure(ability_id ability);
    void _counter_hit(int index);
    [[nodiscard]] const char* _pet_reason(ability_id ability) const;
    [[nodiscard]] const char* _tame_reason(const enemy& target) const;
    void _tame(int index);

    // Enemy abilities (gw_enemy_ai.cpp)
    [[nodiscard]] bool _enemy_can_use(int index, int slot, int distance_squared) const;
    bool _start_enemy_ability(int index, int slot);
    void _enemy_ability_goes_off(int index, enemy_ability_id ability);
    void _enemy_ability_lands(int caster, enemy_ability_id ability, int damage, const bn::fixed_point& from);
    void _enemy_strike(int index, enemy_ability_id ability);
    void _enemy_debuff(int caster, enemy_ability_id ability, const bn::fixed_point& from);
    void _update_enemy_charge(int index);
    void _start_flee(int index);
    void _update_flee(int index);
    void _call_for_help(int index, int radius);
    void _enemy_blink(int index, int distance);
    [[nodiscard]] int _hurt_friend(int index, int range, int below) const;
    [[nodiscard]] int _enemy_swing(const enemy& item) const;
    [[nodiscard]] bool _enemy_sees_player(const enemy& item) const;

    // The player's debuffs
    void _apply_debuff(buff_id debuff, int frames, int value, const bn::fixed_point& from);
    [[nodiscard]] bool _controlled() const;
    [[nodiscard]] const char* _control_reason(ability_id ability) const;
    void _break_free(ability_id ability);
    void _periodic_tick(buff_id debuff);
};

}

#endif
