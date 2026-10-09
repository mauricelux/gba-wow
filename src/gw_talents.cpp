#include "gw_talents.h"

#include "gw_character.h"

namespace gw
{

namespace
{
    using e = talent_effect;
    using a = ability_id;

    // name, description, effect, value per rank, ranks, tier, ability
    constexpr talent_def talents[int(subclass_id::COUNT) - 1][talents_per_tree] = {
        // arms
        {
            { "Deflection", "Raises your chance to dodge by 1%.", e::DODGE, 1, 5, 0, a::NONE },
            { "Improved Rend", "Rend deals 15% more damage.", e::ABILITY_DAMAGE, 15, 3, 0, a::REND },
            { "Tactical Mastery", "You gain 10% more rage.", e::RAGE_PERCENT, 10, 3, 0, a::NONE },
            { "Deep Wounds", "Critical hits deal 5% more damage.", e::CRIT_DAMAGE_PERCENT, 5, 3, 1, a::NONE },
            { "Weapon Mastery", "Raises attack power by 5.", e::ATTACK_POWER, 5, 5, 1, a::NONE },
            { "Improved Overpower", "Overpower is 25% more likely to crit.", e::ABILITY_CRIT, 25, 2, 1,
              a::OVERPOWER },
            { "Two-Hand Mastery", "You deal 2% more damage.", e::DAMAGE_PERCENT, 2, 3, 2, a::NONE },
            { "Impale", "Critical hits deal 10% more damage.", e::CRIT_DAMAGE_PERCENT, 10, 2, 2, a::NONE },
            { "Imp. Thunder Clap", "Thunder Clap deals 20% more damage.", e::ABILITY_DAMAGE, 20, 3, 2,
              a::THUNDER_CLAP },
            { "Sword Specialization", "Raises your critical hit chance by 1%.", e::CRIT, 1, 5, 3, a::NONE },
            { "Imp. Heroic Strike", "Heroic Strike costs 7% less rage.", e::ABILITY_COST, 7, 3, 3,
              a::HEROIC_STRIKE },
            { "Blood Frenzy", "You deal 2% more damage.", e::DAMAGE_PERCENT, 2, 3, 3, a::NONE },
            { "Sweeping Strikes", "Learn Sweeping Strikes: your next 5 hits also strike a second enemy.",
              e::ABILITY, 0, 1, 4, a::SWEEPING_STRIKES },
            { "Improved Execute", "Execute costs 15% less rage.", e::ABILITY_COST, 15, 2, 4, a::EXECUTE },
            { "Weapon Expertise", "Raises attack power by 6.", e::ATTACK_POWER, 6, 5, 5, a::NONE },
            { "Endless Rage", "You gain 5% more rage.", e::RAGE_PERCENT, 5, 2, 5, a::NONE },
            { "Mortal Strike", "Learn Mortal Strike: a vicious strike for weapon damage plus more.",
              e::ABILITY, 0, 1, 6, a::MORTAL_STRIKE },
            { "Whirling Blades", "Learn Whirling Blades: spin for 6 seconds, hitting everything. Level 50.",
              e::ABILITY, 0, 1, 6, a::WHIRLING_BLADES },
        },
        // fury
        {
            { "Cruelty", "Raises your critical hit chance by 1%.", e::CRIT, 1, 5, 0, a::NONE },
            { "Booming Voice", "Raises attack power by 4.", e::ATTACK_POWER, 4, 3, 0, a::NONE },
            { "Unbridled Wrath", "You gain 10% more rage.", e::RAGE_PERCENT, 10, 2, 0, a::NONE },
            { "Imp. Battle Shout", "Battle Shout gives 5% more attack power.", e::ABILITY_DAMAGE, 5, 5, 1,
              a::BATTLE_SHOUT },
            { "Enrage", "You deal 2% more damage.", e::DAMAGE_PERCENT, 2, 3, 1, a::NONE },
            { "Improved Cleave", "Cleave adds 40% more damage.", e::ABILITY_DAMAGE, 40, 3, 1, a::CLEAVE },
            { "Flurry", "You attack 4% faster.", e::HASTE_PERCENT, 4, 3, 2, a::NONE },
            { "Brute Force", "Raises strength by 2%.", e::STRENGTH_PERCENT, 2, 2, 2, a::NONE },
            { "Improved Slam", "Slam winds up faster.", e::CAST_TIME, 12, 3, 2, a::SLAM },
            { "Imp. Demoralizing", "Demoralizing Shout weakens 8% more.", e::ABILITY_DAMAGE, 8, 5, 3,
              a::DEMORALIZING_SHOUT },
            { "Imp. Berserker Rage", "Berserker Rage gives 50% more rage.", e::ABILITY_DAMAGE, 50, 2, 3,
              a::BERSERKER_RAGE },
            { "Bloodlust", "Critical hits deal 5% more damage.", e::CRIT_DAMAGE_PERCENT, 5, 3, 3, a::NONE },
            { "Death Wish", "Learn Death Wish: deal 20% more damage for 30 seconds.", e::ABILITY, 0, 1, 4,
              a::DEATH_WISH },
            { "Improved Whirlwind", "Whirlwind deals 10% more damage.", e::ABILITY_DAMAGE, 10, 3, 4,
              a::WHIRLWIND },
            { "Rampage", "Raises attack power by 10.", e::ATTACK_POWER, 10, 3, 5, a::NONE },
            { "Savage Fury", "You deal 2% more damage.", e::DAMAGE_PERCENT, 2, 3, 5, a::NONE },
            { "Bloodthirst", "Learn Bloodthirst: strike for a share of your attack power and heal.",
              e::ABILITY, 0, 1, 6, a::BLOODTHIRST },
            { "Unending Fury", "You attack 5% faster.", e::HASTE_PERCENT, 5, 1, 6, a::NONE },
        },
        // protection
        {
            { "Shield Specialty", "Raises your chance to block by 1%.", e::BLOCK, 1, 5, 0, a::NONE },
            { "Anticipation", "Raises your chance to dodge by 1%.", e::DODGE, 1, 5, 0, a::NONE },
            { "Toughness", "Raises armor by 2%.", e::ARMOR_PERCENT, 2, 5, 0, a::NONE },
            { "Defiance", "Raises stamina by 3%.", e::STAMINA_PERCENT, 3, 3, 1, a::NONE },
            { "Improved Bloodrage", "You gain 10% more rage.", e::RAGE_PERCENT, 10, 2, 1, a::NONE },
            { "Improved Revenge", "Revenge deals 15% more damage.", e::ABILITY_DAMAGE, 15, 3, 1, a::REVENGE },
            { "Last Stand", "Learn Last Stand: gain 30% maximum health for 20 seconds.", e::ABILITY, 0, 1, 2,
              a::LAST_STAND },
            { "Vigor", "Raises maximum health by 3%.", e::HEALTH_PERCENT, 3, 2, 2, a::NONE },
            { "Imp. Sunder Armor", "Sunder Armor costs 15% less rage.", e::ABILITY_COST, 15, 3, 2,
              a::SUNDER_ARMOR },
            { "One-Hand Mastery", "You deal 2% more damage.", e::DAMAGE_PERCENT, 2, 5, 3, a::NONE },
            { "Shield Mastery", "Raises armor by 5%.", e::ARMOR_PERCENT, 5, 2, 3, a::NONE },
            { "Iron Will", "You take 1% less damage.", e::DAMAGE_TAKEN, 1, 3, 3, a::NONE },
            { "Concussion Blow", "Learn Concussion Blow: stun the target for 5 seconds.", e::ABILITY, 0, 1, 4,
              a::CONCUSSION_BLOW },
            { "Improved Shield Bash", "Shield Bash deals 50% more damage.", e::ABILITY_DAMAGE, 50, 2, 4,
              a::SHIELD_BASH },
            { "Vitality", "Raises stamina by 2%.", e::STAMINA_PERCENT, 2, 5, 5, a::NONE },
            { "Focused Rage", "Your abilities cost 5% less rage.", e::COST_PERCENT, 5, 3, 5, a::NONE },
            { "Shield Slam", "Learn Shield Slam: slam the target with your shield for heavy damage.",
              e::ABILITY, 0, 1, 6, a::SHIELD_SLAM },
            { "Bulwark", "You take 3% less damage.", e::DAMAGE_TAKEN, 3, 1, 6, a::NONE },
        },
        // arcane
        {
            { "Arcane Subtlety", "Mana comes back 10% faster.", e::REGEN_PERCENT, 10, 3, 0, a::NONE },
            { "Arcane Focus", "Raises spell power by 4.", e::SPELL_POWER, 4, 3, 0, a::NONE },
            { "Imp. Arcane Missiles", "Arcane Missiles deals 5% more damage.", e::ABILITY_DAMAGE, 5, 5, 0,
              a::ARCANE_MISSILES },
            { "Arcane Mind", "Raises intellect by 3%.", e::INTELLECT_PERCENT, 3, 5, 1, a::NONE },
            { "Arcane Reserves", "Raises maximum mana by 4%.", e::MANA_PERCENT, 4, 2, 1, a::NONE },
            { "Clearcasting", "Your spells cost 2% less mana.", e::COST_PERCENT, 2, 5, 1, a::NONE },
            { "Arcane Impact", "Arcane Explosion is 4% more likely to crit.", e::ABILITY_CRIT, 4,
              3, 2, a::ARCANE_EXPLOSION },
            { "Imp. Arcane Blast", "Arcane Blast deals 5% more damage.", e::ABILITY_DAMAGE, 5, 3, 2,
              a::ARCANE_BLAST },
            { "Arcane Meditation", "Mana comes back 5% faster.", e::REGEN_PERCENT, 5, 3, 2, a::NONE },
            { "Arcane Instability", "You deal 2% more damage.", e::DAMAGE_PERCENT, 2, 3, 3, a::NONE },
            { "Arcane Potency", "Raises spell critical chance by 2%.", e::SPELL_CRIT, 2, 2, 3, a::NONE },
            { "Improved Mana Shield", "Mana Shield absorbs 10% more.", e::ABILITY_DAMAGE, 10, 2, 3,
              a::MANA_SHIELD },
            { "Presence of Mind", "Learn Presence of Mind: your next spell with a cast is instant.",
              e::ABILITY, 0, 1, 4, a::PRESENCE_OF_MIND },
            { "Prismatic Cloak", "You take 2% less damage.", e::DAMAGE_TAKEN, 2, 2, 4, a::NONE },
            { "Mind Mastery", "Raises intellect by 2%.", e::INTELLECT_PERCENT, 2, 5, 5, a::NONE },
            { "Empowered Arcane", "Critical hits deal 5% more damage.", e::CRIT_DAMAGE_PERCENT, 5, 3, 5,
              a::NONE },
            { "Arcane Power", "Learn Arcane Power: spells deal 30% more damage for 15 seconds.", e::ABILITY,
              0, 1, 6, a::ARCANE_POWER },
            { "Arcane Mastery", "Raises spell power by 5.", e::SPELL_POWER, 5, 3, 6, a::NONE },
        },
        // fire
        {
            { "Improved Fireball", "Fireball casts faster.", e::CAST_TIME, 6, 5, 0, a::FIREBALL },
            { "Ignite", "Critical hits deal 8% more damage.", e::CRIT_DAMAGE_PERCENT, 8, 5, 0, a::NONE },
            { "Incinerate", "Raises spell critical chance by 1%.", e::SPELL_CRIT, 1, 3, 0, a::NONE },
            { "Burning Soul", "Raises maximum health by 4%.", e::HEALTH_PERCENT, 4, 2, 1, a::NONE },
            { "Improved Flamestrike", "Flamestrike is 5% more likely to crit.", e::ABILITY_CRIT, 5, 3, 1,
              a::FLAMESTRIKE },
            { "Improved Fire Blast", "Fire Blast is ready a second sooner.", e::COOLDOWN, 1, 3, 1,
              a::FIRE_BLAST },
            { "Pyroblast", "Learn Pyroblast: a slow, huge fireball that keeps burning.", e::ABILITY, 0, 1, 2,
              a::PYROBLAST },
            { "Impact", "Fire Blast deals 10% more damage.", e::ABILITY_DAMAGE, 10, 3, 2, a::FIRE_BLAST },
            { "Improved Scorch", "Scorch deals 10% more damage.", e::ABILITY_DAMAGE, 10, 3, 2, a::SCORCH },
            { "Master of Elements", "Your spells cost 3% less mana.", e::COST_PERCENT, 3, 3, 3, a::NONE },
            { "Critical Mass", "Raises spell critical chance by 2%.", e::SPELL_CRIT, 2, 3, 3, a::NONE },
            { "Fire Power", "You deal 2% more damage.", e::DAMAGE_PERCENT, 2, 5, 3, a::NONE },
            { "Blast Wave", "Learn Blast Wave: a ring of fire that pushes back and slows.", e::ABILITY, 0, 1,
              4, a::BLAST_WAVE },
            { "Molten Fury", "You deal 3% more damage.", e::DAMAGE_PERCENT, 3, 2, 4, a::NONE },
            { "Empowered Fireball", "Raises spell power by 4.", e::SPELL_POWER, 4, 5, 5, a::NONE },
            { "Playing with Fire", "Raises spell critical chance by 1%.", e::SPELL_CRIT, 1, 3, 5, a::NONE },
            { "Combustion", "Learn Combustion: fire spells crit more and more, until three crits.", e::ABILITY,
              0, 1, 6, a::COMBUSTION },
            { "Pyromaniac", "Critical hits deal 5% more damage.", e::CRIT_DAMAGE_PERCENT, 5, 3, 6, a::NONE },
        },
        // frost
        {
            { "Improved Frostbolt", "Frostbolt casts faster.", e::CAST_TIME, 6, 5, 0, a::FROSTBOLT },
            { "Ice Shards", "Critical hits deal 10% more damage.", e::CRIT_DAMAGE_PERCENT, 10, 5, 0, a::NONE },
            { "Frost Warding", "Raises armor by 10%.", e::ARMOR_PERCENT, 10, 2, 0, a::NONE },
            { "Frostbite", "Frostbolt and Cone of Cold have a 5% chance to freeze.", e::FREEZE_CHANCE, 5, 3, 1,
              a::NONE },
            { "Improved Frost Nova", "Frost Nova is ready 2 seconds sooner.", e::COOLDOWN, 2, 2, 1,
              a::FROST_NOVA },
            { "Piercing Ice", "You deal 2% more damage.", e::DAMAGE_PERCENT, 2, 3, 1, a::NONE },
            { "Shatter", "10% more likely to crit frozen targets.", e::FROZEN_CRIT, 10, 5, 2, a::NONE },
            { "Frost Channeling", "Your spells cost 5% less mana.", e::COST_PERCENT, 5, 3, 2, a::NONE },
            { "Improved Blizzard", "Blizzard deals 10% more damage.", e::ABILITY_DAMAGE, 10, 3, 2,
              a::BLIZZARD },
            { "Permafrost", "Ice Lance deals 10% more damage.", e::ABILITY_DAMAGE, 10, 3, 3, a::ICE_LANCE },
            { "Ice Floes", "Cone of Cold is ready 2 seconds sooner.", e::COOLDOWN, 2, 2, 3, a::CONE_OF_COLD },
            { "Winter's Chill", "Raises spell critical chance by 1%.", e::SPELL_CRIT, 1, 5, 3, a::NONE },
            { "Ice Barrier", "Learn Ice Barrier: a shield of ice absorbs damage.", e::ABILITY, 0, 1, 4,
              a::ICE_BARRIER },
            { "Cold Snap", "Learn Cold Snap: end the cooldowns of your frost spells.", e::ABILITY, 0, 1, 4,
              a::COLD_SNAP },
            { "Arctic Reach", "Cone of Cold deals 10% more damage.", e::ABILITY_DAMAGE, 10, 3, 5,
              a::CONE_OF_COLD },
            { "Frozen Core", "You take 2% less damage.", e::DAMAGE_TAKEN, 2, 3, 5, a::NONE },
            { "Water Elemental", "A frost elemental fights for you. Arrives with pets.", e::COMING, 0, 1,
              6, a::NONE },
            { "Ice Mastery", "Critical hits deal 5% more damage.", e::CRIT_DAMAGE_PERCENT, 5, 5, 6, a::NONE },
        },
        // beast mastery
        {
            { "Improved Hawk", "You attack 2% faster.", e::HASTE_PERCENT, 2, 5, 0, a::NONE },
            { "Endurance Training", "Raises maximum health by 2%.", e::HEALTH_PERCENT, 2, 5, 0, a::NONE },
            { "Thick Hide", "Raises armor by 5%.", e::ARMOR_PERCENT, 5, 3, 0, a::NONE },
            { "Improved Monkey", "Raises your chance to dodge by 1%.", e::DODGE, 1, 3, 1,
              a::NONE },
            { "Unleashed Fury", "You deal 2% more damage.", e::DAMAGE_PERCENT, 2, 5, 1, a::NONE },
            { "Imp. Serpent Sting", "Serpent Sting deals 5% more damage.", e::ABILITY_DAMAGE, 5, 5, 1,
              a::SERPENT_STING },
            { "Ferocity", "Raises your critical hit chance by 1%.", e::CRIT, 1, 5, 2, a::NONE },
            { "Spirit Bond", "Raises stamina by 3%.", e::STAMINA_PERCENT, 3, 2, 2, a::NONE },
            { "Bestial Discipline", "Mana comes back 10% faster.", e::REGEN_PERCENT, 10, 2, 2, a::NONE },
            { "Frenzy", "You attack 2% faster.", e::HASTE_PERCENT, 2, 3, 3, a::NONE },
            { "Thrill of the Hunt", "Your abilities cost 5% less mana.", e::COST_PERCENT, 5, 3, 3, a::NONE },
            { "Kindred Spirits", "You deal 1% more damage.", e::DAMAGE_PERCENT, 1, 3, 3, a::NONE },
            { "Intimidation", "Your pet stuns the target. Arrives with pets.", e::COMING, 0, 1, 4, a::NONE },
            { "Bestial Fortitude", "Raises stamina by 2%.", e::STAMINA_PERCENT, 2, 3, 4, a::NONE },
            { "Serpent's Swiftness", "You attack 2% faster.", e::HASTE_PERCENT, 2, 5, 5, a::NONE },
            { "Animal Handler", "Raises attack power and ranged attack power by 8.", e::ATTACK_POWER, 8, 3, 5,
              a::NONE },
            { "Bestial Wrath", "Learn Bestial Wrath: attack 40% faster for 15 seconds.", e::ABILITY, 0, 1, 6,
              a::BESTIAL_WRATH },
            { "The Beast Within", "You deal 3% more damage.", e::DAMAGE_PERCENT, 3, 2, 6, a::NONE },
        },
        // marksmanship
        {
            { "Lethal Shots", "Raises your critical hit chance by 1%.", e::CRIT, 1, 5, 0, a::NONE },
            { "Efficiency", "Your shots cost 2% less mana.", e::COST_PERCENT, 2, 5, 0, a::NONE },
            { "Aimed Precision", "Raises ranged attack power by 8.", e::RANGED_ATTACK_POWER, 8, 3, 0,
              a::NONE },
            { "Imp. Hunter's Mark", "Hunter's Mark is 20% stronger.", e::ABILITY_DAMAGE, 20, 3, 1,
              a::HUNTERS_MARK },
            { "Hawk Eye", "Raises ranged attack power by 10.", e::RANGED_ATTACK_POWER, 10, 2, 1, a::NONE },
            { "Improved Arcane Shot", "Arcane Shot is ready a second sooner.", e::COOLDOWN, 1, 2, 1,
              a::ARCANE_SHOT },
            { "Mortal Shots", "Critical hits deal 6% more damage.", e::CRIT_DAMAGE_PERCENT, 6, 5, 2, a::NONE },
            { "Improved Aimed Shot", "Aimed Shot is quicker to aim.", e::CAST_TIME, 10, 3, 2, a::AIMED_SHOT },
            { "Imp. Serpent Sting", "Serpent Sting deals 5% more damage.", e::ABILITY_DAMAGE, 5, 3, 2,
              a::SERPENT_STING },
            { "Barrage", "Multi-Shot deals 5% more damage.", e::ABILITY_DAMAGE, 5, 3, 3, a::MULTI_SHOT },
            { "Ranged Mastery", "You deal 1% more damage.", e::DAMAGE_PERCENT, 1, 5, 3,
              a::NONE },
            { "Careful Aim", "Raises ranged attack power by 5.", e::RANGED_ATTACK_POWER, 5, 3, 3, a::NONE },
            { "Scatter Shot", "Learn Scatter Shot: disorient the target for 4 seconds.", e::ABILITY, 0, 1, 4,
              a::SCATTER_SHOT },
            { "Rapid Killing", "Rapid Fire is ready a minute sooner.", e::COOLDOWN, 60, 2, 4, a::RAPID_FIRE },
            { "Master Marksman", "Raises ranged attack power by 6.", e::RANGED_ATTACK_POWER, 6, 5, 5,
              a::NONE },
            { "Improved Volley", "Volley deals 10% more damage.", e::ABILITY_DAMAGE, 10, 3, 5, a::VOLLEY },
            { "Trueshot Aura", "Learn Trueshot Aura: raises attack power while it lasts.", e::ABILITY, 0, 1, 6,
              a::TRUESHOT_AURA },
            { "Marked for Death", "You deal 2% more damage.", e::DAMAGE_PERCENT, 2, 3, 6, a::NONE },
        },
        // survival
        {
            { "Monster Slaying", "You deal 1% more damage.", e::DAMAGE_PERCENT, 1, 3, 0, a::NONE },
            { "Savage Strikes", "Raptor Strike is 10% more likely to crit.", e::ABILITY_CRIT, 10, 2, 0,
              a::RAPTOR_STRIKE },
            { "Deflection", "Raises your chance to dodge by 1%.", e::DODGE, 1, 5, 0, a::NONE },
            { "Survivalist", "Raises maximum health by 2%.", e::HEALTH_PERCENT, 2, 5, 1, a::NONE },
            { "Clever Traps", "Traps hit 15% harder and hold 15% longer.", e::TRAP_PERCENT, 15, 2, 1, a::NONE },
            { "Iron Hide", "Raises armor by 5%.", e::ARMOR_PERCENT, 5, 3, 1, a::NONE },
            { "Surefooted", "You take 1% less damage.", e::DAMAGE_TAKEN, 1, 3, 2, a::NONE },
            { "Killer Instinct", "Raises your critical hit chance by 1%.", e::CRIT, 1, 3, 2, a::NONE },
            { "Imp. Mongoose Bite", "Mongoose Bite deals 10% more damage.", e::ABILITY_DAMAGE, 10, 3, 2,
              a::MONGOOSE_BITE },
            { "Lightning Reflexes", "Raises agility by 3%.", e::AGILITY_PERCENT, 3, 5, 3, a::NONE },
            { "Vicious Strikes", "Critical hits deal 5% more damage.", e::CRIT_DAMAGE_PERCENT, 5, 3, 3,
              a::NONE },
            { "Trap Mastery", "Traps hit 10% harder and hold 10% longer.", e::TRAP_PERCENT, 10, 2, 3,
              a::NONE },
            { "Counterattack", "Learn Counterattack: after a dodge, strike and pin the target.", e::ABILITY,
              0, 1, 4, a::COUNTERATTACK },
            { "Resourcefulness", "Your abilities cost 3% less mana.", e::COST_PERCENT, 3, 3, 4, a::NONE },
            { "Expose Weakness", "Raises attack power by 8.", e::ATTACK_POWER, 8, 3, 5, a::NONE },
            { "Survival Instincts", "You take 2% less damage.", e::DAMAGE_TAKEN, 2, 2, 5, a::NONE },
            { "Wyvern Sting", "Learn Wyvern Sting: put the target to sleep, then poison it.", e::ABILITY, 0, 1,
              6, a::WYVERN_STING },
            { "Master Tactician", "Raises your critical hit chance by 1%.", e::CRIT, 1, 5, 6, a::NONE },
        },
    };

    [[nodiscard]] subclass_id own_subclass()
    {
        return character().subclass;
    }
}

const talent_def& get_talent(subclass_id subclass, int index)
{
    int tree = subclass == subclass_id::NONE ? 0 : int(subclass) - 1;
    return talents[tree][index];
}

int talent_rank(int index)
{
    return character().talents[index];
}

int talent_points_total()
{
    int result = 0;

    for(int index = 0; index < talents_per_tree; ++index)
    {
        result += talent_rank(index);
    }

    return result;
}

int talent_points_available()
{
    int earned = character().level - first_talent_level + 1;
    return earned > 0 ? earned - talent_points_total() : 0;
}

talent_check can_learn_talent(int index)
{
    const talent_def& def = get_talent(own_subclass(), index);

    if(def.effect == talent_effect::COMING)
    {
        return talent_check::COMING;
    }

    if(talent_rank(index) >= def.ranks)
    {
        return talent_check::MAXED;
    }

    if(talent_points_total() < def.tier * points_per_tier)
    {
        return talent_check::TIER_LOCKED;
    }

    if(talent_points_available() <= 0)
    {
        return talent_check::NO_POINTS;
    }

    if(def.effect == talent_effect::ABILITY && character().level < ability_level(def.ability))
    {
        return talent_check::LEVEL_TOO_LOW;
    }

    return talent_check::OK;
}

bool learn_talent(int index)
{
    if(can_learn_talent(index) != talent_check::OK)
    {
        return false;
    }

    ++character().talents[index];
    character().talent_points_spent = uint8_t(talent_points_total());

    const talent_def& def = get_talent(own_subclass(), index);

    if(def.effect == talent_effect::ABILITY)
    {
        learn_ability(def.ability);
    }

    return true;
}

void reset_talents()
{
    character_data& data = character();

    for(int index = 0; index < talents_per_tree; ++index)
    {
        const talent_def& def = get_talent(data.subclass, index);

        if(def.effect == talent_effect::ABILITY && talent_rank(index))
        {
            forget_ability(def.ability);
        }
    }

    for(uint8_t& rank : data.talents)
    {
        rank = 0;
    }

    data.talent_points_spent = 0;
}

talent_bonus talent_bonuses()
{
    talent_bonus result;
    subclass_id subclass = own_subclass();

    if(subclass == subclass_id::NONE)
    {
        return result;
    }

    for(int index = 0; index < talents_per_tree; ++index)
    {
        int rank = talent_rank(index);

        if(! rank)
        {
            continue;
        }

        const talent_def& def = get_talent(subclass, index);
        int value = def.value * rank;

        switch(def.effect)
        {

        case e::STRENGTH_PERCENT:
            result.strength_percent += value;
            break;

        case e::AGILITY_PERCENT:
            result.agility_percent += value;
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

            // Animal Handler helps shots too.
            if(subclass == subclass_id::BEAST_MASTERY)
            {
                result.ranged_attack_power += value;
            }
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

        case e::BLOCK:
            result.block += value;
            break;

        case e::DAMAGE_PERCENT:
            result.damage_percent += value;
            break;

        case e::DAMAGE_TAKEN:
            result.damage_taken += value;
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

        case e::COST_PERCENT:
            result.cost_percent += value;
            break;

        default:
            break;
        }
    }

    return result;
}

int talent_value(talent_effect effect, ability_id ability)
{
    subclass_id subclass = own_subclass();
    int result = 0;

    if(subclass == subclass_id::NONE)
    {
        return 0;
    }

    for(int index = 0; index < talents_per_tree; ++index)
    {
        int rank = talent_rank(index);

        if(rank)
        {
            const talent_def& def = get_talent(subclass, index);

            if(def.effect == effect && def.ability == ability)
            {
                result += def.value * rank;
            }
        }
    }

    return result;
}

}
