#ifndef GW_BUFFS_H
#define GW_BUFFS_H

#include <stdint.h>

namespace gw
{

// What the player has on: their own buffs, then what enemies put on them. Ids are append-only.
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
    // Debuffs from enemy abilities. value: damage per tick for the ones over time, otherwise a percent.
    CHILLED,        // moves slower (Frostbolt)
    ROOTED,         // can't move (Net, Frost Nova, Web)
    STUNNED,        // can't move or act (Knockdown, War Stomp, Charge)
    ASLEEP,         // can't move or act until hit (Sleep)
    POLYMORPHED,    // the same, as a sheep
    FEARED,         // runs away from whoever cast it
    WEAKENED,       // deals less damage (Curse of Weakness)
    SUNDERED,       // less armor (Sunder Armor), value: armor taken off
    WOUNDED,        // healing does half (Mortal Strike)
    BLEEDING,       // Rend
    POISONED,       // Poison
    DISEASED,       // Disease
    BURNING,        // Burning
    // The player's own again, from travel and pets on.
    MOUNTED,        // value: percent faster
    ASPECT_OF_THE_BEAST,
    WATER_ELEMENTAL,    // the elemental stays while it lasts
    COUNT
};

static_assert(int(buff_id::COUNT) <= 64, "the hud keeps a bit per buff");

// Lasts until death or until replaced (aspects).
constexpr int permanent_buff = 0x7FFFFFFF;

// Debuffs show on their own row under the buffs.
[[nodiscard]] constexpr bool is_debuff(buff_id buff)
{
    return buff == buff_id::DAZED || (buff >= buff_id::CHILLED && buff <= buff_id::BURNING);
}

// The debuffs enemy abilities put on the player, in order.
constexpr int enemy_debuff_count = int(buff_id::BURNING) + 1 - int(buff_id::CHILLED);

// Debuffs that hurt every few seconds.
[[nodiscard]] constexpr bool is_periodic(buff_id buff)
{
    return buff >= buff_id::BLEEDING && buff <= buff_id::BURNING;
}

}

#endif
