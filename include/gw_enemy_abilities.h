#ifndef GW_ENEMY_ABILITIES_H
#define GW_ENEMY_ABILITIES_H

#include "bn_fixed_point.h"

#include "gw_abilities.h"
#include "gw_buffs.h"
#include "gw_ids.h"

namespace gw
{

// The shared list of what enemies can do besides swinging. An enemy type names up to two of them
// (enemy_def::abilities), so new enemies get abilities by data. Ids are append-only.
enum class enemy_ability_id : uint8_t
{
    NONE,
    // Caster
    FIREBALL,
    FROSTBOLT,
    LIGHTNING_BOLT,
    HEALING_WAVE,
    SHADOW_BOLT,
    HOLY_SMITE,
    HEAL,
    // Melee
    REND,
    SUNDER_ARMOR,
    THRASH,
    MORTAL_STRIKE,
    CLEAVE,
    KNOCKDOWN,
    // Control
    NET,
    FROST_NOVA,
    FEAR,
    SLEEP,
    POLYMORPH,
    WEB,
    // Over time
    POISON,
    DISEASE,
    CURSE_OF_WEAKNESS,
    BURNING,
    // Movement
    CHARGE,
    LEAP,
    WAR_STOMP,
    BLINK,
    // Support
    CALL_FOR_HELP,
    ENRAGE,
    BATTLE_SHOUT,
    SHIELD_WALL,
    SUMMON_SKELETON,
    // Defense
    SHIELD_BLOCK,
    EVASION,
    STONESKIN,
    MANA_SHIELD,
    // One enemy's own
    CANDLE_THROW,
    // Added with the Wetlands, Blackfathom Deeps and Gnomeregan
    FORKED_LIGHTNING,
    SELF_DESTRUCT,      // the caster blows up with it: no corpse, no loot
    TOXIC_VOLLEY,
    CROWD_PUMMEL,
    THROW_DYNAMITE,
    // Added with Hillsbrad and the Scarlet Monastery
    FLAME_SPIKE,
    DETONATION,         // a huge circle with a long cast: run out of it
    ARCANE_EXPLOSION,
    PSYCHIC_SCREAM,
    POISON_CLOUD,
    // Added with Stranglethorn and the Scarlet Monastery's last wings
    WHIRLWIND,
    DEEP_SLEEP,
    COUNT
};

// How an enemy fights.
enum class ai_style : uint8_t
{
    MELEE,      // walks up and swings
    CASTER,     // stays at casting range and casts; swings when the player comes close
    RANGED,     // the same with shots
    HEALER,     // a caster that heals itself and friends
    RUNNER      // melee that runs off to bring friends when nearly dead (murlocs, gnolls, kobolds)
};

// Who helps whom: Call for Help, fleeing and Battle Shout only reach the same family.
enum class enemy_family : uint8_t
{
    BEAST,
    KOBOLD,
    MURLOC,
    GNOLL,
    DEFIAS,
    CONSTRUCT,
    ORC,
    UNDEAD,
    WORGEN,
    OGRE,
    NAGA,
    TROGG,
    DWARF,
    CULTIST,
    GNOME,
    OOZE,
    SYNDICATE,
    SCARLET,
    TROLL,
    PIRATE,
    QUILBOAR,
    CENTAUR,
    SATYR,
    ELEMENTAL,
    PLANT,
    TAUREN,
    HIGHBORNE,
    DRAGONKIN,
    TITAN
};

enum class enemy_effect : uint8_t
{
    HIT,            // damage and a debuff on the player: a strike, a projectile or an area
    HEAL,           // heals itself or the most hurt friend
    CHARGE,         // rushes at the player from range, then hits
    BLINK,          // jumps away from the player
    CALL_FOR_HELP,  // friends of its family nearby join the fight
    SUMMON,         // calls an add
    ENRAGE,         // hits harder and faster for the rest of the fight
    RALLY,          // it and its friends nearby hit harder (Battle Shout)
    GUARD,          // takes value percent less damage
    EVASION,        // dodges value percent of the attacks
    ABSORB          // a shield that soaks value percent of its health
};

// Where a HIT lands.
enum class enemy_target : uint8_t
{
    PLAYER,         // the player: in melee, by a projectile or at once
    AROUND_SELF,    // a red circle around the enemy, then everything in it
    AT_PLAYER       // a red circle where the player stood when the cast began
};

namespace enemy_ability_flag
{
    constexpr uint8_t SPELL = 1;            // silence stops it
    constexpr uint8_t INTERRUPTIBLE = 2;    // the player's interrupts stop the cast
    constexpr uint8_t SWING = 4;            // a strike that takes the place of a swing
    constexpr uint8_t ONCE = 8;             // once per fight
    constexpr uint8_t PHYSICAL = 16;        // GUARD: only against physical damage
}

struct enemy_ability_def
{
    const char* name;
    enemy_effect effect;
    enemy_target target;
    buff_id debuff;             // put on the player when it lands; COUNT for none
    school damage_school;       // of the hit and of a debuff over time
    projectile_kind projectile; // NONE: it lands when the cast ends
    uint8_t flags;
    uint8_t range;              // pixels to the player (BLINK: how far it jumps)
    uint8_t radius;             // pixels, of an area or of who hears a call
    uint8_t cast_time;          // tenths of a second, 0 for instant
    uint8_t cooldown;           // seconds
    uint8_t duration;           // seconds, of the debuff or of the buff
    uint8_t value;              // damage in percent of a swing; heal, buff and guard percents
    uint8_t debuff_value;       // per tick in percent of a swing over time; otherwise the debuff's percent
    uint8_t health_below;       // only at or under this health percent (HEAL: the one healed), 0 any time
    uint8_t hits;               // strikes in a row (Thrash)
    enemy_id summon;            // SUMMON: who comes
};

constexpr int enemy_ability_count = int(enemy_ability_id::COUNT);
constexpr int enemy_ability_slots = 2;

[[nodiscard]] const enemy_ability_def& get_enemy_ability(enemy_ability_id ability);

// What an enemy is doing with its abilities. Reset when it spawns, goes home or starts a fight.
struct enemy_ability_state
{
    enemy_ability_id casting = enemy_ability_id::NONE;
    int8_t cast_slot = -1;
    int16_t cast_frames = 0;        // left
    int16_t cast_total = 0;
    int16_t cooldowns[enemy_ability_slots] = {};
    int16_t heal_target = -1;       // HEAL: the enemy index it heals
    int circle = 0;                 // the red circle of an area being cast, to take away early
    bn::fixed_point cast_position;  // where an area lands
    int16_t flee_frames = 0;        // running for help
    int16_t flee_friend = -1;
    int16_t rally_frames = 0;
    int16_t guard_frames = 0;
    int16_t evasion_frames = 0;
    int16_t absorb = 0;             // Mana Shield left
    uint8_t rally_percent = 0;
    uint8_t guard_percent = 0;
    uint8_t evasion_percent = 0;
    bool guard_physical = false;
    bool fled = false;              // runners flee once per fight
    bool charging = false;
    bool enraged = false;
};

}

#endif
