#ifndef GW_ABILITIES_H
#define GW_ABILITIES_H

#include "gw_icons.h"
#include "gw_ids.h"

namespace gw
{

// Saves store a rank for each of these values: keep the order, append only.
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
    // warrior, from the subclasses on
    PUMMEL,
    SHIELD_BASH,
    INTIMIDATING_SHOUT,
    OVERPOWER,
    RETALIATION,
    SWEEPING_STRIKES,
    WHIRLING_BLADES,
    CLEAVE,
    SLAM,
    WHIRLWIND,
    BERSERKER_RAGE,
    DEMORALIZING_SHOUT,
    RECKLESSNESS,
    DEATH_WISH,
    SUNDER_ARMOR,
    REVENGE,
    SHIELD_BLOCK,
    DISARM,
    SHIELD_WALL,
    CONCUSSION_BLOW,
    SHIELD_SLAM,
    // mage
    ARCANE_INTELLECT,
    CONJURE_WATER,
    CONJURE_FOOD,
    BLINK,
    COUNTERSPELL,
    POLYMORPH,
    TELEPORT_STORMWIND,
    SCORCH,
    FLAMESTRIKE,
    FIRE_WARD,
    MOLTEN_ARMOR,
    BLAST_WAVE,
    COMBUSTION,
    CONE_OF_COLD,
    BLIZZARD,
    ICE_LANCE,
    COLD_SNAP,
    ICE_BLOCK,
    ARCANE_BLAST,
    MANA_SHIELD,
    MAGE_ARMOR,
    SLOW,
    EVOCATION,
    PRESENCE_OF_MIND,
    // hunter
    ASPECT_OF_THE_MONKEY,
    ASPECT_OF_THE_CHEETAH,
    WING_CLIP,
    FEIGN_DEATH,
    RAPID_FIRE,
    VOLLEY,
    SCATTER_SHOT,
    TRUESHOT_AURA,
    MONGOOSE_BITE,
    IMMOLATION_TRAP,
    FREEZING_TRAP,
    FROST_TRAP,
    EXPLOSIVE_TRAP,
    DETERRENCE,
    WYVERN_STING,
    COUNT
};

// The three ability bars: Combat (hold R), Utility (hold L) and Buffs (hold L and R).
enum class bar_id : uint8_t
{
    COMBAT,
    UTILITY,
    BUFFS,
    COUNT
};

constexpr int bar_count = int(bar_id::COUNT);

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

// How an ability's value grows.
enum class ability_scaling : uint8_t
{
    LEVEL,      // value_per_level for every character level
    DAMAGE,     // the same up to level 20, then as fast as enemy health
    RANK        // value_per_level for every rank above the first
};

// Which of its class's subclasses get an ability: a bit per subclass, in talent tree order.
namespace kit
{
    constexpr uint8_t FIRST = 1;
    constexpr uint8_t SECOND = 2;
    constexpr uint8_t THIRD = 4;
    constexpr uint8_t ALL = 7;
}

namespace ability_flag
{
    constexpr uint8_t STARTER = 1;      // known from level 1, without a trainer
    constexpr uint8_t TALENT = 2;       // the first rank comes from a talent, the others from trainers
    constexpr uint8_t CHANNELED = 4;    // pays up front and works every second of its cast time
    constexpr uint8_t SHIELD = 8;       // needs a shield
    constexpr uint8_t REACTIVE = 16;    // only right after a dodge (or a block)
    constexpr uint8_t ASPECT = 32;      // one aspect at a time, until changed
}

constexpr int max_ranks = 12;

struct ability_def
{
    const char* name;
    const char* description;
    class_id player_class;
    uint8_t kits;               // kit bits
    icon_id icon;
    uint8_t flags;              // ability_flag bits
    uint8_t cost;               // rage, or mana at rank 1 (higher ranks cost more)
    uint16_t cooldown;          // frames
    uint16_t cast_time;         // frames; 0 = instant
    uint8_t range;              // pixels; 0 = melee
    int16_t value;              // damage, healing or effect strength at level 1
    int8_t value_per_level;     // see scaling
    ability_scaling scaling;
    int32_t duration;           // frames, for damage over time, buffs and debuffs
    ability_target target;
    school damage_school;
    projectile_kind projectile;
    uint8_t ranks[max_ranks];   // the level each rank is trained at, 0 after the last
};

constexpr int ability_count = int(ability_id::COUNT);

[[nodiscard]] const ability_def& get_ability(ability_id ability);

[[nodiscard]] int rank_count(ability_id ability);

// The level rank (from 1) is trained at, or 0 past the last rank.
[[nodiscard]] int rank_level(ability_id ability, int rank);

// The level the ability first becomes available.
[[nodiscard]] int ability_level(ability_id ability);

// The ability's main value at a rank for a character of the given level. A rank grows with its
// owner's level until the level of the next rank.
[[nodiscard]] int rank_value(ability_id ability, int rank, int level);

// Rage, or mana: higher ranks cost more mana.
[[nodiscard]] int rank_cost(ability_id ability, int rank);

// What trainers ask for the rank, in copper.
[[nodiscard]] int rank_train_cost(ability_id ability, int rank);

// Whether the subclass gets the ability.
[[nodiscard]] bool in_kit(ability_id ability, subclass_id subclass);

// The bar a newly learned ability goes on: long buffs and travel on Buffs, interrupts, crowd
// control and long cooldowns on Utility, the rest on Combat.
[[nodiscard]] bar_id default_bar(ability_id ability);

// Pixels counted as melee range between two characters' feet.
constexpr int melee_range = 22;

// Frames between two abilities (the global cooldown).
constexpr int global_cooldown = 60;

}

#endif
