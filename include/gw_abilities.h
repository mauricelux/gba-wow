#ifndef GW_ABILITIES_H
#define GW_ABILITIES_H

#include "gw_icons.h"
#include "gw_ids.h"

namespace gw
{

// Saves store known abilities as a bit mask of these values: keep the order, append only.
enum class ability_id : uint8_t
{
    NONE,
    // warrior
    HEROIC_STRIKE,
    BATTLE_SHOUT,
    CHARGE,
    REND,
    THUNDER_CLAP,
    HAMSTRING,
    EXECUTE,
    MORTAL_STRIKE,
    BLOODTHIRST,
    LAST_STAND,
    // mage
    FIREBALL,
    FROST_ARMOR,
    FROSTBOLT,
    FIRE_BLAST,
    ARCANE_MISSILES,
    FROST_NOVA,
    ARCANE_EXPLOSION,
    PYROBLAST,
    ICE_BARRIER,
    ARCANE_POWER,
    // hunter
    RAPTOR_STRIKE,
    SERPENT_STING,
    ARCANE_SHOT,
    HUNTERS_MARK,
    CONCUSSIVE_SHOT,
    ASPECT_OF_THE_HAWK,
    MULTI_SHOT,
    AIMED_SHOT,
    COUNTERATTACK,
    BESTIAL_WRATH,
    COUNT
};

enum class ability_target : uint8_t
{
    ENEMY,      // needs a hostile target in range
    SELF,       // buffs
    AREA        // hits every enemy around the player
};

enum class school : uint8_t
{
    PHYSICAL,
    FIRE,
    FROST,
    ARCANE,
    NATURE
};

enum class projectile_kind : uint8_t
{
    NONE,
    FIRE,
    FROST,
    ARCANE,
    ARROW
};

struct ability_def
{
    const char* name;
    const char* description;
    class_id player_class;
    icon_id icon;
    uint8_t level;              // trainers teach it from this level; 0 = learned through talents
    uint16_t train_cost;        // copper
    uint8_t cost;               // rage or mana
    uint16_t cooldown;          // frames
    uint8_t cast_time;          // frames; 0 = instant
    uint8_t range;              // pixels; 0 = melee
    int16_t value;              // damage, healing or effect strength at level 1
    int8_t value_per_level;     // added per character level above 1
    uint16_t duration;          // frames, for damage over time, buffs and debuffs
    ability_target target;
    school damage_school;
    projectile_kind projectile;
};

constexpr int ability_count = int(ability_id::COUNT);

[[nodiscard]] const ability_def& get_ability(ability_id ability);

// The ability's main value for a character of the given level.
[[nodiscard]] int ability_value(ability_id ability, int level);

// Pixels counted as melee range between two characters' feet.
constexpr int melee_range = 22;

// Frames between two abilities (the global cooldown).
constexpr int global_cooldown = 60;

}

#endif
