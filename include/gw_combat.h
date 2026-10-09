#ifndef GW_COMBAT_H
#define GW_COMBAT_H

#include "bn_fixed_point.h"
#include "bn_string.h"
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
    ARCANE_INTELLECT,
    MOLTEN_ARMOR,
    MAGE_ARMOR,
    FIRE_WARD,
    MANA_SHIELD,
    ARCANE_BLAST,   // value: stacks
    PRESENCE_OF_MIND,
    COMBUSTION,     // value: extra critical chance for the next fire spell
    ICE_BLOCK,
    RETALIATION,
    SWEEPING_STRIKES,   // value: hits left
    WHIRLING_BLADES,
    BERSERKER_RAGE,
    RECKLESSNESS,
    DEATH_WISH,
    SHIELD_BLOCK,
    SHIELD_WALL,
    ASPECT_OF_THE_MONKEY,
    ASPECT_OF_THE_CHEETAH,
    RAPID_FIRE,
    DETERRENCE,
    TRUESHOT_AURA,
    DAZED,          // slowed after being hit while running with the Cheetah
    COUNT
};

static_assert(int(buff_id::COUNT) <= 32, "the hud keeps a bit per buff");

// The bar whose button is held, which the hud shows around its cross.
enum class held_bar : uint8_t
{
    NONE,
    COMBAT,     // R
    UTILITY,    // L
    BUFFS,      // L and R
    ITEMS       // Select
};

// Lasts until death or until replaced (aspects).
constexpr int permanent_buff = 0x7FFFFFFF;

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

    // Called by enemies. percent scales the hit (special attacks).
    void enemy_attacks(int index, int percent = 100);
    void enemy_killed(int index);

    // Special abilities of elites and bosses, every frame while they fight. Returns true when the
    // enemy is busy and skips its normal movement and swings this frame.
    bool boss_update(int index);

    // Deals damage to an enemy with floating text. Returns true if it died.
    bool damage_enemy(int index, int amount, bool crit, bool periodic = false,
                      school damage_school = school::PHYSICAL);

    // Deals damage to the player (boss specials).
    void damage_player(int amount, const bn::fixed_point& from, school damage_school = school::PHYSICAL);

    // Movement speed in percent, from aspects and dazes; 0 while the player can't move.
    [[nodiscard]] int speed_percent() const;

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
};

}

#endif
