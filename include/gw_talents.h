#ifndef GW_TALENTS_H
#define GW_TALENTS_H

#include "gw_abilities.h"
#include "gw_ids.h"

namespace gw
{

// Each subclass has one tree of eighteen talents in seven tiers; character_data::talents stores the
// rank of each by its position in the tree. Keep the order: saves store ranks by position.
constexpr int talents_per_tree = 18;
constexpr int talent_tiers = 7;

// Points spent in the tree before its next tier opens.
constexpr int points_per_tier = 5;

// The first talent point comes at this level, then one per level: 51 by level 60.
constexpr int first_talent_level = 10;

enum class talent_effect : uint8_t
{
    STRENGTH_PERCENT,
    AGILITY_PERCENT,
    STAMINA_PERCENT,
    INTELLECT_PERCENT,
    HEALTH_PERCENT,
    MANA_PERCENT,
    ARMOR_PERCENT,
    ATTACK_POWER,
    RANGED_ATTACK_POWER,
    HASTE_PERCENT,
    CRIT,
    SPELL_CRIT,
    DODGE,
    BLOCK,
    DAMAGE_PERCENT,
    DAMAGE_TAKEN,       // percent less damage taken
    SPELL_POWER,
    CRIT_DAMAGE_PERCENT,
    RAGE_PERCENT,
    REGEN_PERCENT,
    COST_PERCENT,       // every ability costs this much less, in percent
    ABILITY_DAMAGE,     // the ability's value is this much higher, in percent
    ABILITY_CRIT,       // the ability is this much more likely to crit, in percent
    ABILITY_COST,       // the ability costs this much less, in percent
    CAST_TIME,          // the ability's cast is this many frames shorter
    COOLDOWN,           // the ability's cooldown is this many seconds shorter
    FREEZE_CHANCE,      // Frostbolt and Cone of Cold freeze the target, chance in percent
    FROZEN_CRIT,        // more likely to crit frozen targets, in percent
    TRAP_PERCENT,       // traps hit harder and hold longer, in percent
    ABILITY,            // teaches the ability
    COMING              // not in the game yet
};

struct talent_def
{
    const char* name;
    const char* description;    // what one rank does
    talent_effect effect;
    uint8_t value;              // per rank
    uint8_t ranks;
    uint8_t tier;               // needs tier * points_per_tier points in the tree
    ability_id ability;
};

// Passive bonuses from the talents the character has spent points on.
struct talent_bonus
{
    int strength_percent = 0;
    int agility_percent = 0;
    int stamina_percent = 0;
    int intellect_percent = 0;
    int health_percent = 0;
    int mana_percent = 0;
    int armor_percent = 0;
    int attack_power = 0;
    int ranged_attack_power = 0;
    int haste_percent = 0;
    int crit = 0;
    int spell_crit = 0;
    int dodge = 0;
    int block = 0;
    int damage_percent = 0;
    int damage_taken = 0;
    int spell_power = 0;
    int crit_damage_percent = 0;
    int rage_percent = 0;
    int regen_percent = 0;
    int cost_percent = 0;
};

[[nodiscard]] const talent_def& get_talent(subclass_id subclass, int index);

[[nodiscard]] int talent_rank(int index);

[[nodiscard]] int talent_points_total();

[[nodiscard]] int talent_points_available();

enum class talent_check : uint8_t
{
    OK,
    NO_POINTS,
    MAXED,
    TIER_LOCKED,
    LEVEL_TOO_LOW,      // a talent ability that needs a higher level
    COMING
};

[[nodiscard]] talent_check can_learn_talent(int index);

// Spends a point on the talent; returns false when it can't.
bool learn_talent(int index);

// Takes every point back, forgetting talent abilities.
void reset_talents();

[[nodiscard]] talent_bonus talent_bonuses();

// The sum of the character's talents of the effect that name the ability (ABILITY_DAMAGE,
// ABILITY_CRIT, ABILITY_COST, CAST_TIME, COOLDOWN), or of every talent of the effect for the effects
// that name none (FREEZE_CHANCE, FROZEN_CRIT, TRAP_PERCENT).
[[nodiscard]] int talent_value(talent_effect effect, ability_id ability = ability_id::NONE);

}

#endif
