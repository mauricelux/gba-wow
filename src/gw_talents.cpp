#include "gw_talents.h"

#include "gw_character.h"

namespace gw
{

namespace
{
    using e = talent_effect;
    using a = ability_id;

    constexpr const char* tree_names[3][talent_trees] = {
        { "Arms", "Fury", "Protection" },
        { "Arcane", "Fire", "Frost" },
        { "Beast Mastery", "Marksmanship", "Survival" },
    };

    // name, description, effect, value per rank, ranks, tier, ability
    constexpr talent_def talents[3][talent_trees][talents_per_tree] = {
        // warrior
        {
            {
                { "Tactical Mastery", "You gain 10% more rage.", e::RAGE_PERCENT, 10, 3, 0, a::NONE },
                { "Deep Wounds", "Critical hits deal 5% more damage.", e::CRIT_DAMAGE_PERCENT, 5, 3, 0, a::NONE },
                { "Two-Hand Mastery", "You deal 2% more damage.", e::DAMAGE_PERCENT, 2, 3, 1, a::NONE },
                { "Weapon Mastery", "Raises attack power by 8.", e::ATTACK_POWER, 8, 2, 1, a::NONE },
                { "Impale", "Critical hits deal 10% more damage.", e::CRIT_DAMAGE_PERCENT, 10, 2, 2, a::NONE },
                { "Sword Specialization", "Raises your critical hit chance by 1%.", e::CRIT, 1, 3, 2, a::NONE },
                { "Mortal Strike", "Learn Mortal Strike: a vicious strike for weapon damage plus more.",
                  e::ABILITY, 0, 1, 3, a::MORTAL_STRIKE },
                { "Sweeping Strikes", "You attack 5% faster.", e::HASTE_PERCENT, 5, 1, 3, a::NONE },
            },
            {
                { "Cruelty", "Raises your critical hit chance by 1%.", e::CRIT, 1, 3, 0, a::NONE },
                { "Booming Voice", "Raises attack power by 6.", e::ATTACK_POWER, 6, 3, 0, a::NONE },
                { "Enrage", "You deal 2% more damage.", e::DAMAGE_PERCENT, 2, 3, 1, a::NONE },
                { "Unbridled Wrath", "You gain 10% more rage.", e::RAGE_PERCENT, 10, 2, 1, a::NONE },
                { "Flurry", "You attack 4% faster.", e::HASTE_PERCENT, 4, 3, 2, a::NONE },
                { "Brute Force", "Raises strength by 2%.", e::STRENGTH_PERCENT, 2, 2, 2, a::NONE },
                { "Bloodthirst", "Learn Bloodthirst: strike based on attack power and heal yourself.",
                  e::ABILITY, 0, 1, 3, a::BLOODTHIRST },
                { "Death Wish", "You deal 5% more damage.", e::DAMAGE_PERCENT, 5, 1, 3, a::NONE },
            },
            {
                { "Toughness", "Raises armor from items by 3%.", e::ARMOR_PERCENT, 3, 3, 0, a::NONE },
                { "Anticipation", "Raises your chance to dodge by 1%.", e::DODGE, 1, 3, 0, a::NONE },
                { "Defiance", "Raises stamina by 3%.", e::STAMINA_PERCENT, 3, 3, 1, a::NONE },
                { "Improved Bloodrage", "You gain 10% more rage.", e::RAGE_PERCENT, 10, 2, 1, a::NONE },
                { "One-Hand Mastery", "You deal 2% more damage.", e::DAMAGE_PERCENT, 2, 3, 2, a::NONE },
                { "Vigor", "Raises maximum health by 3%.", e::HEALTH_PERCENT, 3, 2, 2, a::NONE },
                { "Last Stand", "Learn Last Stand: gain 30% maximum health for 20 seconds.", e::ABILITY, 0, 1,
                  3, a::LAST_STAND },
                { "Shield Mastery", "Raises armor from items by 10%.", e::ARMOR_PERCENT, 10, 1, 3, a::NONE },
            },
        },
        // mage
        {
            {
                { "Arcane Subtlety", "Mana comes back 10% faster.", e::REGEN_PERCENT, 10, 3, 0, a::NONE },
                { "Arcane Focus", "Raises spell power by 4.", e::SPELL_POWER, 4, 3, 0, a::NONE },
                { "Arcane Mind", "Raises intellect by 3%.", e::INTELLECT_PERCENT, 3, 3, 1, a::NONE },
                { "Arcane Reserves", "Raises maximum mana by 4%.", e::MANA_PERCENT, 4, 2, 1, a::NONE },
                { "Arcane Instability", "You deal 2% more damage.", e::DAMAGE_PERCENT, 2, 3, 2, a::NONE },
                { "Arcane Potency", "Raises spell critical chance by 2%.", e::SPELL_CRIT, 2, 2, 2, a::NONE },
                { "Arcane Power", "Learn Arcane Power: spells deal 30% more damage for 15 seconds.",
                  e::ABILITY, 0, 1, 3, a::ARCANE_POWER },
                { "Mind Mastery", "Raises intellect by 5%.", e::INTELLECT_PERCENT, 5, 1, 3, a::NONE },
            },
            {
                { "Ignite", "Critical hits deal 10% more damage.", e::CRIT_DAMAGE_PERCENT, 10, 3, 0, a::NONE },
                { "Incinerate", "Raises spell critical chance by 1%.", e::SPELL_CRIT, 1, 3, 0, a::NONE },
                { "Improved Fireball", "Raises spell power by 4.", e::SPELL_POWER, 4, 3, 1, a::NONE },
                { "Burning Soul", "Raises maximum health by 4%.", e::HEALTH_PERCENT, 4, 2, 1, a::NONE },
                { "Critical Mass", "Raises spell critical chance by 2%.", e::SPELL_CRIT, 2, 2, 2, a::NONE },
                { "Fire Power", "You deal 2% more damage.", e::DAMAGE_PERCENT, 2, 3, 2, a::NONE },
                { "Pyroblast", "Learn Pyroblast: a slow, huge fireball that keeps burning.", e::ABILITY, 0, 1,
                  3, a::PYROBLAST },
                { "Combustion", "Critical hits deal 15% more damage.", e::CRIT_DAMAGE_PERCENT, 15, 1, 3,
                  a::NONE },
            },
            {
                { "Improved Frostbolt", "Raises spell power by 3.", e::SPELL_POWER, 3, 3, 0, a::NONE },
                { "Ice Shards", "Critical hits deal 10% more damage.", e::CRIT_DAMAGE_PERCENT, 10, 3, 0,
                  a::NONE },
                { "Frost Warding", "Raises armor by 10%.", e::ARMOR_PERCENT, 10, 2, 1, a::NONE },
                { "Piercing Ice", "You deal 2% more damage.", e::DAMAGE_PERCENT, 2, 3, 1, a::NONE },
                { "Shatter", "Raises spell critical chance by 2%.", e::SPELL_CRIT, 2, 3, 2, a::NONE },
                { "Frost Channeling", "Mana comes back 10% faster.", e::REGEN_PERCENT, 10, 2, 2, a::NONE },
                { "Ice Barrier", "Learn Ice Barrier: a shield of ice absorbs damage for a minute.", e::ABILITY,
                  0, 1, 3, a::ICE_BARRIER },
                { "Winter's Chill", "Raises spell critical chance by 3%.", e::SPELL_CRIT, 3, 1, 3, a::NONE },
            },
        },
        // hunter
        {
            {
                { "Endurance Training", "Raises maximum health by 3%.", e::HEALTH_PERCENT, 3, 3, 0, a::NONE },
                { "Improved Hawk", "You attack 3% faster.", e::HASTE_PERCENT, 3, 3, 0, a::NONE },
                { "Thick Hide", "Raises armor by 5%.", e::ARMOR_PERCENT, 5, 3, 1, a::NONE },
                { "Ferocity", "Raises your critical hit chance by 1%.", e::CRIT, 1, 3, 1, a::NONE },
                { "Bestial Discipline", "Mana comes back 10% faster.", e::REGEN_PERCENT, 10, 2, 2, a::NONE },
                { "Frenzy", "You deal 2% more damage.", e::DAMAGE_PERCENT, 2, 3, 2, a::NONE },
                { "Bestial Wrath", "Learn Bestial Wrath: attack 40% faster for 15 seconds.", e::ABILITY, 0, 1,
                  3, a::BESTIAL_WRATH },
                { "Spirit Bond", "Raises stamina by 5%.", e::STAMINA_PERCENT, 5, 1, 3, a::NONE },
            },
            {
                { "Lethal Shots", "Raises your critical hit chance by 1%.", e::CRIT, 1, 3, 0, a::NONE },
                { "Efficiency", "Mana comes back 10% faster.", e::REGEN_PERCENT, 10, 3, 0, a::NONE },
                { "Aimed Precision", "Raises ranged attack power by 8.", e::RANGED_ATTACK_POWER, 8, 3, 1,
                  a::NONE },
                { "Mortal Shots", "Critical hits deal 10% more damage.", e::CRIT_DAMAGE_PERCENT, 10, 2, 1,
                  a::NONE },
                { "Barrage", "You deal 2% more damage.", e::DAMAGE_PERCENT, 2, 3, 2, a::NONE },
                { "Hawk Eye", "Raises ranged attack power by 10.", e::RANGED_ATTACK_POWER, 10, 2, 2, a::NONE },
                { "Aimed Shot", "Learn Aimed Shot: a slow, carefully aimed shot for heavy damage.", e::ABILITY,
                  0, 1, 3, a::AIMED_SHOT },
                { "Trueshot Aura", "Raises ranged attack power by 20.", e::RANGED_ATTACK_POWER, 20, 1, 3,
                  a::NONE },
            },
            {
                { "Deflection", "Raises your chance to dodge by 1%.", e::DODGE, 1, 3, 0, a::NONE },
                { "Savage Strikes", "Critical hits deal 10% more damage.", e::CRIT_DAMAGE_PERCENT, 10, 3, 0,
                  a::NONE },
                { "Survivalist", "Raises maximum health by 3%.", e::HEALTH_PERCENT, 3, 3, 1, a::NONE },
                { "Killer Instinct", "Raises your critical hit chance by 1%.", e::CRIT, 1, 3, 1, a::NONE },
                { "Lightning Reflexes", "Raises attack power by 10.", e::ATTACK_POWER, 10, 3, 2, a::NONE },
                { "Iron Hide", "Raises armor by 10%.", e::ARMOR_PERCENT, 10, 2, 2, a::NONE },
                { "Counterattack", "Learn Counterattack: strike and pin the target in place.", e::ABILITY, 0, 1,
                  3, a::COUNTERATTACK },
                { "Deterrence", "Raises your chance to dodge by 5%.", e::DODGE, 5, 1, 3, a::NONE },
            },
        },
    };

    [[nodiscard]] uint8_t& rank_ref(int tree, int index)
    {
        return character().talents[tree * talents_per_tree + index];
    }
}

const char* talent_tree_name(class_id player_class, int tree)
{
    return tree_names[int(player_class)][tree];
}

const talent_def& get_talent(class_id player_class, int tree, int index)
{
    return talents[int(player_class)][tree][index];
}

int talent_rank(int tree, int index)
{
    return rank_ref(tree, index);
}

int talent_points_in_tree(int tree)
{
    int result = 0;

    for(int index = 0; index < talents_per_tree; ++index)
    {
        result += talent_rank(tree, index);
    }

    return result;
}

int talent_points_total()
{
    int result = 0;

    for(int tree = 0; tree < talent_trees; ++tree)
    {
        result += talent_points_in_tree(tree);
    }

    return result;
}

int talent_points_available()
{
    int earned = character().level - first_talent_level + 1;
    return earned > 0 ? earned - talent_points_total() : 0;
}

talent_check can_learn_talent(int tree, int index)
{
    const talent_def& def = get_talent(character().player_class, tree, index);

    if(talent_rank(tree, index) >= def.ranks)
    {
        return talent_check::MAXED;
    }

    if(talent_points_in_tree(tree) < def.tier * points_per_tier)
    {
        return talent_check::TIER_LOCKED;
    }

    if(talent_points_available() <= 0)
    {
        return talent_check::NO_POINTS;
    }

    return talent_check::OK;
}

bool learn_talent(int tree, int index)
{
    if(can_learn_talent(tree, index) != talent_check::OK)
    {
        return false;
    }

    ++rank_ref(tree, index);
    character().talent_points_spent = uint8_t(talent_points_total());

    const talent_def& def = get_talent(character().player_class, tree, index);

    if(def.effect == talent_effect::ABILITY)
    {
        learn_ability(def.ability);
    }

    return true;
}

void reset_talents()
{
    character_data& data = character();

    for(int tree = 0; tree < talent_trees; ++tree)
    {
        for(int index = 0; index < talents_per_tree; ++index)
        {
            const talent_def& def = get_talent(data.player_class, tree, index);

            if(def.effect == talent_effect::ABILITY && talent_rank(tree, index))
            {
                forget_ability(def.ability);
            }

            rank_ref(tree, index) = 0;
        }
    }

    data.talent_points_spent = 0;
}

talent_bonus talent_bonuses()
{
    talent_bonus result;
    class_id player_class = character().player_class;

    for(int tree = 0; tree < talent_trees; ++tree)
    {
        for(int index = 0; index < talents_per_tree; ++index)
        {
            int rank = talent_rank(tree, index);

            if(! rank)
            {
                continue;
            }

            const talent_def& def = get_talent(player_class, tree, index);
            int value = def.value * rank;

            switch(def.effect)
            {

            case e::STRENGTH_PERCENT:
                result.strength_percent += value;
                break;

            case e::STAMINA_PERCENT:
                result.stamina_percent += value;
                break;

            case e::INTELLECT_PERCENT:
                result.intellect_percent += value;
                break;

            case e::HEALTH_PERCENT:
                result.health_percent += value;
                break;

            case e::MANA_PERCENT:
                result.mana_percent += value;
                break;

            case e::ARMOR_PERCENT:
                result.armor_percent += value;
                break;

            case e::ATTACK_POWER:
                result.attack_power += value;
                break;

            case e::RANGED_ATTACK_POWER:
                result.ranged_attack_power += value;
                break;

            case e::HASTE_PERCENT:
                result.haste_percent += value;
                break;

            case e::CRIT:
                result.crit += value;
                break;

            case e::SPELL_CRIT:
                result.spell_crit += value;
                break;

            case e::DODGE:
                result.dodge += value;
                break;

            case e::DAMAGE_PERCENT:
                result.damage_percent += value;
                break;

            case e::SPELL_POWER:
                result.spell_power += value;
                break;

            case e::CRIT_DAMAGE_PERCENT:
                result.crit_damage_percent += value;
                break;

            case e::RAGE_PERCENT:
                result.rage_percent += value;
                break;

            case e::REGEN_PERCENT:
                result.regen_percent += value;
                break;

            default:
                break;
            }
        }
    }

    return result;
}

}
