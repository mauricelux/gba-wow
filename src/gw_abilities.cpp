#include "gw_abilities.h"

namespace gw
{

namespace
{
    constexpr int seconds = 60;

    using c = class_id;
    using i = icon_id;
    using a = ability_target;
    using s = school;
    using p = projectile_kind;

    // name, description, class, icon, level, train cost, cost, cooldown, cast time, range, value,
    // value per level, duration, target, school, projectile
    constexpr ability_def abilities[] = {
        { "", "", c::WARRIOR, i::ATTACK, 0, 0, 0, 0, 0, 0, 0, 0, 0, a::SELF, s::PHYSICAL, p::NONE },

        // warrior (rage)
        { "Heroic Strike", "Your next swing hits harder.", c::WARRIOR, i::HEROIC_STRIKE, 1, 0, 15, 0, 0, 0,
          11, 2, 0, a::ENEMY, s::PHYSICAL, p::NONE },
        { "Battle Shout", "Raises your attack power for two minutes.", c::WARRIOR, i::BATTLE_SHOUT, 1, 10, 10,
          0, 0, 0, 15, 2, 120 * seconds, a::SELF, s::PHYSICAL, p::NONE },
        { "Charge", "Rush to an enemy, stunning it and gaining rage. Not in combat.", c::WARRIOR, i::CHARGE,
          4, 100, 0, 15 * seconds, 0, 110, 12, 0, 60, a::ENEMY, s::PHYSICAL, p::NONE },
        { "Rend", "Wounds the target, dealing damage over 9 seconds.", c::WARRIOR, i::REND, 4, 100, 10, 0,
          0, 0, 15, 3, 9 * seconds, a::ENEMY, s::PHYSICAL, p::NONE },
        { "Thunder Clap", "Damages and slows every enemy around you.", c::WARRIOR, i::THUNDER_CLAP, 6, 300,
          20, 4 * seconds, 0, 44, 10, 2, 10 * seconds, a::AREA, s::PHYSICAL, p::NONE },
        { "Hamstring", "Damages the target and slows its movement.", c::WARRIOR, i::HAMSTRING, 8, 600, 10,
          0, 0, 0, 5, 1, 15 * seconds, a::ENEMY, s::PHYSICAL, p::NONE },
        { "Execute", "Finish off a target below 20% health.", c::WARRIOR, i::EXECUTE, 10, 900, 15, 0, 0, 0,
          50, 6, 0, a::ENEMY, s::PHYSICAL, p::NONE },
        { "Mortal Strike", "A vicious strike for weapon damage plus more.", c::WARRIOR, i::MORTAL_STRIKE, 0, 0,
          30, 6 * seconds, 0, 0, 40, 4, 0, a::ENEMY, s::PHYSICAL, p::NONE },
        { "Bloodthirst", "Strike for damage based on attack power and heal yourself.", c::WARRIOR,
          i::BLOODTHIRST, 0, 0, 30, 6 * seconds, 0, 0, 45, 0, 0, a::ENEMY, s::PHYSICAL, p::NONE },
        { "Last Stand", "Gain 30% maximum health for 20 seconds.", c::WARRIOR, i::SHIELD_BLOCK, 0, 0, 0,
          180 * seconds, 0, 0, 30, 0, 20 * seconds, a::SELF, s::PHYSICAL, p::NONE },

        // mage (mana)
        { "Fireball", "Hurls a fiery ball that burns the target.", c::MAGE, i::FIREBALL, 1, 0, 25, 0,
          90, 140, 16, 4, 4 * seconds, a::ENEMY, s::FIRE, p::FIRE },
        { "Frost Armor", "Raises armor and slows enemies that hit you.", c::MAGE, i::FROST_ARMOR, 1, 0, 40,
          0, 0, 0, 30, 5, 600 * seconds, a::SELF, s::FROST, p::NONE },
        { "Frostbolt", "Damages and slows the target.", c::MAGE, i::FROSTBOLT, 4, 100, 25, 0, 90, 140, 18,
          3, 5 * seconds, a::ENEMY, s::FROST, p::FROST },
        { "Fire Blast", "Instantly blasts the target with fire.", c::MAGE, i::FIRE_BLAST, 6, 300, 40,
          8 * seconds, 0, 100, 26, 3, 0, a::ENEMY, s::FIRE, p::NONE },
        { "Arcane Missiles", "Channels three arcane missiles at the target.", c::MAGE, i::ARCANE_MISSILES, 8,
          600, 70, 0, 180, 140, 12, 2, 0, a::ENEMY, s::ARCANE, p::ARCANE },
        { "Frost Nova", "Freezes nearby enemies in place.", c::MAGE, i::FROST_NOVA, 10, 900, 50,
          25 * seconds, 0, 52, 15, 1, 8 * seconds, a::AREA, s::FROST, p::NONE },
        { "Arcane Explosion", "Damages every enemy around you.", c::MAGE, i::ARCANE_EXPLOSION, 14, 1800, 70,
          0, 0, 52, 32, 2, 0, a::AREA, s::ARCANE, p::NONE },
        { "Pyroblast", "A slow, huge fireball that keeps burning.", c::MAGE, i::PYROBLAST, 0, 0, 90, 0,
          210, 140, 90, 6, 12 * seconds, a::ENEMY, s::FIRE, p::FIRE },
        { "Ice Barrier", "A shield of ice absorbs damage for a minute.", c::MAGE, i::ICE_BARRIER, 0, 0, 80,
          30 * seconds, 0, 0, 120, 8, 60 * seconds, a::SELF, s::FROST, p::NONE },
        { "Arcane Power", "Your spells deal 30% more damage for 15 seconds.", c::MAGE, i::ARCANE_INTELLECT, 0,
          0, 0, 120 * seconds, 0, 0, 30, 0, 15 * seconds, a::SELF, s::ARCANE, p::NONE },

        // hunter (mana)
        { "Raptor Strike", "A strong melee attack.", c::HUNTER, i::RAPTOR_STRIKE, 1, 0, 15, 6 * seconds, 0, 0,
          10, 2, 0, a::ENEMY, s::PHYSICAL, p::NONE },
        { "Serpent Sting", "Poisons the target over 15 seconds.", c::HUNTER, i::SERPENT_STING, 4, 100, 15, 0,
          0, 140, 20, 4, 15 * seconds, a::ENEMY, s::NATURE, p::ARROW },
        { "Arcane Shot", "An instant shot of arcane energy.", c::HUNTER, i::ARCANE_SHOT, 6, 300, 25,
          6 * seconds, 0, 140, 14, 3, 0, a::ENEMY, s::ARCANE, p::ARCANE },
        { "Hunter's Mark", "The target takes 10% more damage for two minutes.", c::HUNTER, i::HUNTERS_MARK, 6,
          300, 15, 0, 0, 140, 10, 0, 120 * seconds, a::ENEMY, s::NATURE, p::NONE },
        { "Concussive Shot", "Dazes the target, slowing it for 4 seconds.", c::HUNTER, i::CONCUSSIVE_SHOT, 8,
          600, 20, 12 * seconds, 0, 140, 4, 1, 4 * seconds, a::ENEMY, s::PHYSICAL, p::ARROW },
        { "Aspect of the Hawk", "Raises ranged attack power until you leave the world.", c::HUNTER,
          i::ASPECT_HAWK, 10, 900, 20, 0, 0, 0, 20, 2, 0, a::SELF, s::NATURE, p::NONE },
        { "Multi-Shot", "Shoots the target and up to two enemies near it.", c::HUNTER, i::MULTI_SHOT, 14,
          1800, 50, 10 * seconds, 0, 140, 10, 2, 0, a::ENEMY, s::PHYSICAL, p::ARROW },
        { "Aimed Shot", "A slow, carefully aimed shot for heavy damage.", c::HUNTER, i::AIMED_SHOT, 0, 0, 60,
          6 * seconds, 150, 140, 70, 5, 0, a::ENEMY, s::PHYSICAL, p::ARROW },
        { "Counterattack", "Strike and pin the target in place.", c::HUNTER, i::RAPTOR_STRIKE, 0, 0, 30,
          5 * seconds, 0, 0, 30, 3, 5 * seconds, a::ENEMY, s::PHYSICAL, p::NONE },
        { "Bestial Wrath", "Attack 40% faster for 15 seconds.", c::HUNTER, i::ASPECT_HAWK, 0, 0, 0,
          120 * seconds, 0, 0, 40, 0, 15 * seconds, a::SELF, s::NATURE, p::NONE },
    };

    static_assert(sizeof(abilities) / sizeof(abilities[0]) == ability_count);
}

const ability_def& get_ability(ability_id ability)
{
    int index = int(ability);
    return abilities[index < ability_count ? index : 0];
}

int ability_value(ability_id ability, int level)
{
    const ability_def& def = get_ability(ability);
    return def.value + def.value_per_level * (level - 1);
}

}
