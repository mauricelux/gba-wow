#ifndef GW_TALENTS_H
#define GW_TALENTS_H

#include "gw_abilities.h"
#include "gw_ids.h"

namespace gw
{

// Each class has three trees of eight talents; character_data::talents stores the rank of talent
// tree * talents_per_tree + index. Keep the order: saves store ranks by position.
constexpr int talent_trees = 3;
constexpr int talents_per_tree = 8;

// Points spent in a tree before its next tier opens.
constexpr int points_per_tier = 3;

// The first talent point comes at this level, then one per level.
constexpr int first_talent_level = 10;

enum class talent_effect : uint8_t
{
    STRENGTH_PERCENT,
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
    DAMAGE_PERCENT,
    SPELL_POWER,
    CRIT_DAMAGE_PERCENT,
    RAGE_PERCENT,
    REGEN_PERCENT,
    ABILITY             // teaches the ability
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
    int damage_percent = 0;
    int spell_power = 0;
    int crit_damage_percent = 0;
    int rage_percent = 0;
    int regen_percent = 0;
};

[[nodiscard]] const char* talent_tree_name(class_id player_class, int tree);

[[nodiscard]] const talent_def& get_talent(class_id player_class, int tree, int index);

[[nodiscard]] int talent_rank(int tree, int index);

[[nodiscard]] int talent_points_in_tree(int tree);

[[nodiscard]] int talent_points_total();

[[nodiscard]] int talent_points_available();

enum class talent_check : uint8_t
{
    OK,
    NO_POINTS,
    MAXED,
    TIER_LOCKED
};

[[nodiscard]] talent_check can_learn_talent(int tree, int index);

// Spends a point on the talent; returns false when it can't.
bool learn_talent(int tree, int index);

// Takes every point back, forgetting talent abilities.
void reset_talents();

[[nodiscard]] talent_bonus talent_bonuses();

}

#endif
