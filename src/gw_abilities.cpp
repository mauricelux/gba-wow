#include "gw_abilities.h"

#include "bn_math.h"

namespace gw
{

namespace
{
    constexpr int seconds = 60;
    constexpr int minutes = 60 * seconds;

    using c = class_id;
    using i = icon_id;
    using a = ability_target;
    using s = school;
    using p = projectile_kind;
    using v = ability_scaling;

    // Kits, by class.
    constexpr uint8_t ALL = kit::ALL;
    constexpr uint8_t ARMS = kit::FIRST;
    constexpr uint8_t FURY = kit::SECOND;
    constexpr uint8_t PROT = kit::THIRD;
    constexpr uint8_t ARCANE = kit::FIRST;
    constexpr uint8_t FIRE = kit::SECOND;
    constexpr uint8_t FROST = kit::THIRD;
    constexpr uint8_t BEAST = kit::FIRST;
    constexpr uint8_t MARKS = kit::SECOND;
    constexpr uint8_t SURV = kit::THIRD;

    constexpr uint8_t STARTER = ability_flag::STARTER;
    constexpr uint8_t TALENT = ability_flag::TALENT;
    constexpr uint8_t CHANNELED = ability_flag::CHANNELED;
    constexpr uint8_t SHIELD = ability_flag::SHIELD;
    constexpr uint8_t REACTIVE = ability_flag::REACTIVE;
    constexpr uint8_t ASPECT = ability_flag::ASPECT;
    constexpr uint8_t QUEST = ability_flag::QUEST;

    // name, description, class, kits, icon, flags, cost, cooldown, cast time, range, value, value per
    // level (or rank), scaling, duration, target, school, projectile, rank levels
    constexpr ability_def abilities[] = {
        { "", "", c::WARRIOR, 0, i::ATTACK, 0, 0, 0, 0, 0, 0, 0, v::LEVEL, 0, a::SELF, s::PHYSICAL, p::NONE, {} },

        // warrior (rage)
        { "Heroic Strike", "Your next swing hits harder.", c::WARRIOR, ALL, i::HEROIC_STRIKE, STARTER, 15, 0, 0,
          0, 11, 2, v::DAMAGE, 0, a::ENEMY, s::PHYSICAL, p::NONE, { 1, 8, 16, 24, 32, 40, 48, 56 } },
        { "Battle Shout", "Raises your attack power for two minutes.", c::WARRIOR, ALL, i::BATTLE_SHOUT, 0, 10,
          0, 0, 0, 15, 2, v::LEVEL, 2 * minutes, a::SELF, s::PHYSICAL, p::NONE, { 1, 12, 22, 32, 42, 52, 60 } },
        { "Charge", "Rush to an enemy, stunning it and gaining rage. Not in combat.", c::WARRIOR, ALL, i::CHARGE,
          0, 0, 15 * seconds, 0, 110, 12, 3, v::RANK, 60, a::ENEMY, s::PHYSICAL, p::NONE, { 4, 26, 46 } },
        { "Rend", "Wounds the target, dealing damage over 9 seconds.", c::WARRIOR, ARMS, i::REND, 0, 10, 0, 0, 0,
          15, 3, v::DAMAGE, 9 * seconds, a::ENEMY, s::PHYSICAL, p::NONE, { 4, 10, 20, 30, 40, 50, 60 } },
        { "Thunder Clap", "Damages and slows every enemy around you.", c::WARRIOR, ARMS, i::THUNDER_CLAP, 0, 20,
          4 * seconds, 0, 44, 10, 2, v::DAMAGE, 10 * seconds, a::AREA, s::PHYSICAL, p::NONE,
          { 6, 18, 28, 38, 48, 58 } },
        { "Hamstring", "Damages the target and slows its movement.", c::WARRIOR, ALL, i::HAMSTRING, 0, 10, 0, 0,
          0, 5, 1, v::DAMAGE, 15 * seconds, a::ENEMY, s::PHYSICAL, p::NONE, { 8, 32, 54 } },
        { "Execute", "Finish off a target below 20% health. Spends all your rage.", c::WARRIOR, ALL, i::EXECUTE,
          0, 15, 0, 0, 0, 50, 6, v::DAMAGE, 0, a::ENEMY, s::PHYSICAL, p::NONE, { 10, 24, 32, 40, 48, 56 } },
        { "Mortal Strike", "A vicious strike for weapon damage plus more.", c::WARRIOR, ARMS, i::MORTAL_STRIKE,
          TALENT, 30, 6 * seconds, 0, 0, 40, 4, v::DAMAGE, 0, a::ENEMY, s::PHYSICAL, p::NONE, { 40, 48, 54, 60 } },
        { "Bloodthirst", "Strike for a share of your attack power and heal yourself.", c::WARRIOR, FURY,
          i::BLOODTHIRST, TALENT, 30, 6 * seconds, 0, 0, 45, 3, v::RANK, 0, a::ENEMY, s::PHYSICAL, p::NONE,
          { 40, 48, 54, 60 } },
        { "Last Stand", "Gain 30% maximum health for 20 seconds.", c::WARRIOR, PROT, i::LAST_STAND, TALENT, 0,
          180 * seconds, 0, 0, 30, 0, v::LEVEL, 20 * seconds, a::SELF, s::PHYSICAL, p::NONE, { 20 } },

        // mage (mana)
        { "Fireball", "Hurls a fiery ball that burns the target.", c::MAGE, FIRE, i::FIREBALL, STARTER, 25, 0,
          90, 140, 16, 4, v::DAMAGE, 4 * seconds, a::ENEMY, s::FIRE, p::FIRE,
          { 1, 6, 12, 18, 24, 30, 36, 42, 48, 54, 60 } },
        { "Frost Armor", "Raises armor and slows enemies that hit you.", c::MAGE, FROST, i::FROST_ARMOR, STARTER,
          40, 0, 0, 0, 30, 5, v::LEVEL, 10 * minutes, a::SELF, s::FROST, p::NONE, { 1, 10, 20, 30, 40, 50, 60 } },
        { "Frostbolt", "Damages and slows the target.", c::MAGE, FROST, i::FROSTBOLT, STARTER, 25, 0, 90, 140, 18,
          3, v::DAMAGE, 5 * seconds, a::ENEMY, s::FROST, p::FROST, { 1, 8, 14, 20, 26, 32, 38, 44, 50, 56 } },
        { "Fire Blast", "Instantly blasts the target with fire.", c::MAGE, FIRE, i::FIRE_BLAST, 0, 40,
          8 * seconds, 0, 100, 26, 3, v::DAMAGE, 0, a::ENEMY, s::FIRE, p::NONE, { 6, 14, 22, 30, 38, 46, 54 } },
        { "Arcane Missiles", "Channels three arcane missiles at the target.", c::MAGE, ARCANE,
          i::ARCANE_MISSILES, STARTER | CHANNELED, 45, 0, 180, 140, 12, 2, v::DAMAGE, 0, a::ENEMY, s::ARCANE,
          p::ARCANE, { 1, 8, 16, 24, 32, 40, 48, 56 } },
        { "Frost Nova", "Freezes nearby enemies in place.", c::MAGE, FROST, i::FROST_NOVA, 0, 50, 25 * seconds, 0,
          52, 15, 1, v::DAMAGE, 8 * seconds, a::AREA, s::FROST, p::NONE, { 10, 26, 40, 54 } },
        { "Arcane Explosion", "Damages every enemy around you.", c::MAGE, ARCANE, i::ARCANE_EXPLOSION, 0, 70, 0,
          0, 52, 32, 2, v::DAMAGE, 0, a::AREA, s::ARCANE, p::NONE, { 14, 22, 30, 38, 46, 54 } },
        { "Pyroblast", "A slow, huge fireball that keeps burning.", c::MAGE, FIRE, i::PYROBLAST, TALENT, 90, 0,
          210, 140, 90, 6, v::DAMAGE, 12 * seconds, a::ENEMY, s::FIRE, p::FIRE,
          { 20, 24, 30, 36, 42, 48, 54, 60 } },
        { "Ice Barrier", "A shield of ice absorbs damage for a minute.", c::MAGE, FROST, i::ICE_BARRIER, TALENT,
          80, 30 * seconds, 0, 0, 120, 8, v::DAMAGE, 60 * seconds, a::SELF, s::FROST, p::NONE,
          { 30, 40, 46, 52, 58 } },
        { "Arcane Power", "Your spells deal 30% more damage for 15 seconds.", c::MAGE, ARCANE, i::ARCANE_POWER,
          TALENT, 0, 120 * seconds, 0, 0, 30, 0, v::LEVEL, 15 * seconds, a::SELF, s::ARCANE, p::NONE, { 40 } },

        // hunter (mana)
        { "Raptor Strike", "A strong melee attack.", c::HUNTER, ALL, i::RAPTOR_STRIKE, STARTER, 15, 6 * seconds, 0,
          0, 10, 2, v::DAMAGE, 0, a::ENEMY, s::PHYSICAL, p::NONE, { 1, 8, 16, 24, 32, 40, 48, 56 } },
        { "Serpent Sting", "Poisons the target over 15 seconds.", c::HUNTER, ALL, i::SERPENT_STING, 0, 15, 0, 0,
          140, 20, 4, v::DAMAGE, 15 * seconds, a::ENEMY, s::NATURE, p::ARROW, { 4, 10, 18, 26, 34, 42, 50, 58 } },
        { "Arcane Shot", "An instant shot of arcane energy.", c::HUNTER, BEAST | MARKS, i::ARCANE_SHOT, 0, 25,
          6 * seconds, 0, 140, 14, 3, v::DAMAGE, 0, a::ENEMY, s::ARCANE, p::ARCANE,
          { 6, 12, 20, 28, 36, 44, 52, 60 } },
        { "Hunter's Mark", "The target takes more damage for two minutes.", c::HUNTER, ALL, i::HUNTERS_MARK, 0,
          15, 0, 0, 140, 10, 2, v::RANK, 2 * minutes, a::ENEMY, s::NATURE, p::NONE, { 6, 22, 40, 58 } },
        { "Concussive Shot", "Dazes the target, slowing it for 4 seconds.", c::HUNTER, ALL, i::CONCUSSIVE_SHOT, 0,
          20, 12 * seconds, 0, 140, 4, 1, v::DAMAGE, 4 * seconds, a::ENEMY, s::PHYSICAL, p::ARROW, { 8 } },
        { "Aspect of the Hawk", "Raises ranged attack power until you change aspects.", c::HUNTER, ALL,
          i::ASPECT_HAWK, ASPECT, 20, 0, 0, 0, 20, 2, v::LEVEL, 0, a::SELF, s::NATURE, p::NONE,
          { 10, 18, 28, 38, 48, 58 } },
        { "Multi-Shot", "Shoots the target and up to two enemies near it.", c::HUNTER, MARKS, i::MULTI_SHOT, 0,
          50, 10 * seconds, 0, 140, 10, 2, v::DAMAGE, 0, a::ENEMY, s::PHYSICAL, p::ARROW, { 14, 26, 38, 50, 60 } },
        { "Aimed Shot", "A slow, carefully aimed shot for heavy damage.", c::HUNTER, MARKS, i::AIMED_SHOT, 0, 60,
          6 * seconds, 150, 140, 70, 5, v::DAMAGE, 0, a::ENEMY, s::PHYSICAL, p::ARROW,
          { 20, 28, 36, 44, 52, 60 } },
        { "Counterattack", "Right after you dodge: strike and pin the target in place.", c::HUNTER, SURV,
          i::COUNTERATTACK, TALENT | REACTIVE, 30, 5 * seconds, 0, 0, 30, 3, v::DAMAGE, 5 * seconds, a::ENEMY,
          s::PHYSICAL, p::NONE, { 30, 42, 54 } },
        { "Bestial Wrath", "Attack 40% faster for 15 seconds.", c::HUNTER, BEAST, i::BESTIAL_WRATH, TALENT, 0,
          120 * seconds, 0, 0, 40, 0, v::LEVEL, 15 * seconds, a::SELF, s::NATURE, p::NONE, { 40 } },

        // warrior, from the subclasses on
        { "Pummel", "Strikes the target and interrupts what it is doing.", c::WARRIOR, ARMS | FURY, i::PUMMEL, 0,
          10, 10 * seconds, 0, 0, 6, 1, v::DAMAGE, 4 * seconds, a::ENEMY, s::PHYSICAL, p::NONE, { 12, 38 } },
        { "Shield Bash", "Bashes the target with your shield, interrupting and dazing it.", c::WARRIOR, PROT,
          i::SHIELD_BASH, SHIELD, 10, 12 * seconds, 0, 0, 6, 1, v::DAMAGE, 2 * seconds, a::ENEMY, s::PHYSICAL,
          p::NONE, { 12, 38 } },
        { "Intimidating Shout", "Enemies around you flee in fear. Damage can break it.", c::WARRIOR, ALL,
          i::INTIMIDATING_SHOUT, 0, 25, 120 * seconds, 0, 48, 0, 0, v::LEVEL, 8 * seconds, a::AREA, s::PHYSICAL,
          p::NONE, { 22 } },
        { "Overpower", "Right after the target dodges: a strike that can't miss.", c::WARRIOR, ARMS,
          i::OVERPOWER, REACTIVE, 5, 5 * seconds, 0, 0, 5, 1, v::DAMAGE, 0, a::ENEMY, s::PHYSICAL, p::NONE,
          { 12, 28, 44, 60 } },
        { "Retaliation", "For 15 seconds you strike back at every melee hit.", c::WARRIOR, ARMS, i::RETALIATION,
          0, 0, 300 * seconds, 0, 0, 0, 0, v::LEVEL, 15 * seconds, a::SELF, s::PHYSICAL, p::NONE, { 20 } },
        { "Sweeping Strikes", "Your next 5 melee hits also strike a second enemy.", c::WARRIOR, ARMS,
          i::SWEEPING_STRIKES, TALENT, 30, 30 * seconds, 0, 0, 5, 0, v::LEVEL, 10 * seconds, a::SELF,
          s::PHYSICAL, p::NONE, { 30 } },
        { "Whirling Blades", "Spin for 6 seconds, striking every enemy around you.", c::WARRIOR, ARMS,
          i::WHIRLING_BLADES, TALENT, 25, 90 * seconds, 0, 36, 0, 0, v::LEVEL, 6 * seconds, a::SELF, s::PHYSICAL,
          p::NONE, { 50 } },
        { "Cleave", "Your next swing hits the target and an enemy beside it.", c::WARRIOR, FURY, i::CLEAVE, 0, 20,
          0, 0, 0, 5, 1, v::DAMAGE, 0, a::ENEMY, s::PHYSICAL, p::NONE, { 6, 20, 30, 40, 50 } },
        { "Slam", "A short wind-up, then a heavy blow.", c::WARRIOR, FURY, i::SLAM, 0, 15, 0, 72, 0, 12, 2,
          v::DAMAGE, 0, a::ENEMY, s::PHYSICAL, p::NONE, { 10, 20, 34, 46, 54 } },
        { "Whirlwind", "Strikes every enemy around you with your weapon.", c::WARRIOR, FURY, i::WHIRLWIND, 0, 25,
          10 * seconds, 0, 36, 0, 0, v::DAMAGE, 0, a::AREA, s::PHYSICAL, p::NONE, { 24 } },
        { "Berserker Rage", "Gain 10 rage. Nothing can scare you for 10 seconds.", c::WARRIOR, FURY,
          i::BERSERKER_RAGE, 0, 0, 30 * seconds, 0, 0, 10, 0, v::LEVEL, 10 * seconds, a::SELF, s::PHYSICAL,
          p::NONE, { 22 } },
        { "Demoralizing Shout", "Enemies around you deal less damage for 30 seconds.", c::WARRIOR, FURY | PROT,
          i::DEMORALIZING_SHOUT, 0, 10, 0, 0, 48, 10, 2, v::RANK, 30 * seconds, a::AREA, s::PHYSICAL, p::NONE,
          { 14, 24, 34, 44, 54 } },
        { "Recklessness", "Every hit crits for 15 seconds, but you take 20% more damage.", c::WARRIOR, FURY,
          i::RECKLESSNESS, 0, 0, 300 * seconds, 0, 0, 20, 0, v::LEVEL, 15 * seconds, a::SELF, s::PHYSICAL,
          p::NONE, { 50 } },
        { "Death Wish", "You deal 20% more damage for 30 seconds.", c::WARRIOR, FURY, i::DEATH_WISH, TALENT, 10,
          180 * seconds, 0, 0, 20, 0, v::LEVEL, 30 * seconds, a::SELF, s::PHYSICAL, p::NONE, { 30 } },
        { "Sunder Armor", "Cracks the target's armor so it takes more damage. Stacks five times.", c::WARRIOR,
          PROT, i::SUNDER_ARMOR, 0, 15, 0, 0, 0, 2, 1, v::RANK, 30 * seconds, a::ENEMY, s::PHYSICAL, p::NONE,
          { 6, 18, 30, 42, 54 } },
        { "Revenge", "Right after you dodge or block: a strong counterattack.", c::WARRIOR, PROT, i::REVENGE,
          REACTIVE, 5, 5 * seconds, 0, 0, 12, 3, v::DAMAGE, 0, a::ENEMY, s::PHYSICAL, p::NONE,
          { 10, 20, 30, 40, 50, 60 } },
        { "Shield Block", "Raise your shield: a 75% chance to block hits for 5 seconds.", c::WARRIOR, PROT,
          i::SHIELD_BLOCK, SHIELD, 10, 10 * seconds, 0, 0, 75, 0, v::LEVEL, 5 * seconds, a::SELF, s::PHYSICAL,
          p::NONE, { 16 } },
        { "Disarm", "Knocks the target's weapon away: half damage for 10 seconds.", c::WARRIOR, PROT, i::DISARM,
          0, 20, 60 * seconds, 0, 0, 50, 0, v::LEVEL, 10 * seconds, a::ENEMY, s::PHYSICAL, p::NONE, { 18 } },
        { "Shield Wall", "You take 75% less damage for 10 seconds.", c::WARRIOR, PROT, i::SHIELD_WALL, SHIELD, 0,
          300 * seconds, 0, 0, 75, 0, v::LEVEL, 10 * seconds, a::SELF, s::PHYSICAL, p::NONE, { 28 } },
        { "Concussion Blow", "Stuns the target for 5 seconds.", c::WARRIOR, PROT, i::CONCUSSION_BLOW, TALENT, 15,
          45 * seconds, 0, 0, 5, 1, v::DAMAGE, 5 * seconds, a::ENEMY, s::PHYSICAL, p::NONE, { 30 } },
        { "Shield Slam", "Slams the target with your shield for heavy damage.", c::WARRIOR, PROT, i::SHIELD_SLAM,
          TALENT | SHIELD, 20, 6 * seconds, 0, 0, 40, 5, v::DAMAGE, 0, a::ENEMY, s::PHYSICAL, p::NONE,
          { 40, 48, 54, 60 } },

        // mage
        { "Arcane Intellect", "Raises your intellect for 30 minutes.", c::MAGE, ALL, i::ARCANE_INTELLECT, 0, 30,
          0, 0, 0, 2, 7, v::RANK, 30 * minutes, a::SELF, s::ARCANE, p::NONE, { 1, 14, 28, 42, 56 } },
        { "Conjure Water", "Conjures drinks that restore mana.", c::MAGE, ALL, i::CONJURE_WATER, 0, 30, 0, 180, 0,
          2, 2, v::RANK, 0, a::SELF, s::ARCANE, p::NONE, { 4, 10, 20, 30, 40, 50, 60 } },
        { "Conjure Food", "Conjures food that restores health.", c::MAGE, ALL, i::CONJURE_FOOD, 0, 30, 0, 180, 0,
          2, 2, v::RANK, 0, a::SELF, s::ARCANE, p::NONE, { 6, 12, 22, 32, 42, 52 } },
        { "Blink", "Teleports you a short way forward.", c::MAGE, ALL, i::BLINK, 0, 40, 15 * seconds, 0, 0, 40,
          0, v::LEVEL, 0, a::SELF, s::ARCANE, p::NONE, { 20 } },
        { "Counterspell", "Interrupts the target and stops its spells for 8 seconds.", c::MAGE, ALL,
          i::COUNTERSPELL, 0, 30, 24 * seconds, 0, 140, 0, 0, v::LEVEL, 8 * seconds, a::ENEMY, s::ARCANE,
          p::NONE, { 24 } },
        { "Polymorph", "Turns the target into a sheep for a while. Damage breaks it.", c::MAGE, ALL,
          i::POLYMORPH, 0, 40, 0, 90, 120, 20, 10, v::RANK, 0, a::ENEMY, s::ARCANE, p::NONE, { 8, 20, 40, 60 } },
        { "Teleport: Stormwind", "Teleports you to Stormwind. Uses a Rune of Teleportation.", c::MAGE, ALL,
          i::TELEPORT, 0, 60, 0, 600, 0, 0, 0, v::LEVEL, 0, a::SELF, s::ARCANE, p::NONE, { 20 } },
        { "Scorch", "Scorches the target, which then takes more fire damage.", c::MAGE, FIRE, i::SCORCH, 0, 30, 0,
          90, 140, 6, 2, v::DAMAGE, 30 * seconds, a::ENEMY, s::FIRE, p::NONE, { 22, 28, 34, 40, 46, 52, 58 } },
        { "Flamestrike", "Fire falls on the target's area, then the ground keeps burning.", c::MAGE, FIRE,
          i::FLAMESTRIKE, 0, 60, 0, 120, 120, 14, 3, v::DAMAGE, 8 * seconds, a::ENEMY, s::FIRE, p::NONE,
          { 16, 24, 32, 40, 48, 56 } },
        { "Fire Ward", "Absorbs fire damage for 30 seconds.", c::MAGE, FIRE, i::FIRE_WARD, 0, 40, 30 * seconds, 0,
          0, 20, 7, v::DAMAGE, 30 * seconds, a::SELF, s::FIRE, p::NONE, { 20, 30, 40, 50, 60 } },
        { "Molten Armor", "Raises spell critical chance by 3% and burns enemies that hit you.", c::MAGE, FIRE,
          i::MOLTEN_ARMOR, 0, 50, 0, 0, 0, 3, 1, v::DAMAGE, 30 * minutes, a::SELF, s::FIRE, p::NONE,
          { 10, 30, 50 } },
        { "Blast Wave", "A ring of fire damages, pushes back and slows enemies around you.", c::MAGE, FIRE,
          i::BLAST_WAVE, TALENT, 80, 45 * seconds, 0, 48, 30, 4, v::DAMAGE, 6 * seconds, a::AREA, s::FIRE,
          p::NONE, { 30, 36, 44, 52, 60 } },
        { "Combustion", "Each fire spell is more likely to crit, until three crits.", c::MAGE, FIRE,
          i::COMBUSTION, TALENT, 0, 180 * seconds, 0, 0, 10, 0, v::LEVEL, 60 * seconds, a::SELF, s::FIRE,
          p::NONE, { 40 } },
        { "Cone of Cold", "Blasts the enemies in front of you with frost and slows them.", c::MAGE, FROST,
          i::CONE_OF_COLD, 0, 60, 10 * seconds, 0, 52, 18, 4, v::DAMAGE, 8 * seconds, a::AREA, s::FROST, p::NONE,
          { 26, 34, 42, 50, 58 } },
        { "Blizzard", "Ice rains on the target's area while you channel.", c::MAGE, FROST, i::BLIZZARD,
          CHANNELED, 100, 0, 480, 120, 5, 1, v::DAMAGE, 0, a::ENEMY, s::FROST, p::NONE,
          { 20, 28, 36, 44, 52, 60 } },
        { "Ice Lance", "An instant shard of ice. Triple damage to frozen targets.", c::MAGE, FROST, i::ICE_LANCE,
          0, 20, 0, 0, 140, 6, 2, v::DAMAGE, 0, a::ENEMY, s::FROST, p::FROST, { 16, 32, 48 } },
        { "Cold Snap", "Ends the cooldowns of your frost spells.", c::MAGE, FROST, i::COLD_SNAP, TALENT, 0,
          480 * seconds, 0, 0, 0, 0, v::LEVEL, 0, a::SELF, s::FROST, p::NONE, { 30 } },
        { "Ice Block", "No damage for 10 seconds, but you can't act. Use again to break out.", c::MAGE, FROST,
          i::ICE_BLOCK, 0, 0, 300 * seconds, 0, 0, 0, 0, v::LEVEL, 10 * seconds, a::SELF, s::FROST, p::NONE,
          { 30 } },
        { "Arcane Blast", "A quick blast. Each one in a row hits harder and costs more.", c::MAGE, ARCANE,
          i::ARCANE_BLAST, 0, 35, 0, 60, 140, 14, 3, v::DAMAGE, 8 * seconds, a::ENEMY, s::ARCANE, p::ARCANE,
          { 10, 30, 50 } },
        { "Mana Shield", "Damage drains your mana instead of your health.", c::MAGE, ARCANE, i::MANA_SHIELD, 0,
          50, 0, 0, 0, 20, 6, v::DAMAGE, 60 * seconds, a::SELF, s::ARCANE, p::NONE, { 20, 28, 36, 44, 52, 60 } },
        { "Mage Armor", "Your mana keeps coming back while you cast.", c::MAGE, ARCANE, i::MAGE_ARMOR, 0, 60, 0,
          0, 0, 30, 5, v::RANK, 30 * minutes, a::SELF, s::ARCANE, p::NONE, { 34, 46, 58 } },
        { "Slow", "Halves the target's speed for 15 seconds.", c::MAGE, ARCANE, i::SLOW, 0, 60, 0, 0, 140, 50, 0,
          v::LEVEL, 15 * seconds, a::ENEMY, s::ARCANE, p::NONE, { 20 } },
        { "Evocation", "Channel for 8 seconds to restore most of your mana.", c::MAGE, ARCANE, i::EVOCATION,
          CHANNELED, 0, 480 * seconds, 480, 0, 10, 0, v::LEVEL, 0, a::SELF, s::ARCANE, p::NONE, { 20 } },
        { "Presence of Mind", "Your next spell with a cast time is instant.", c::MAGE, ARCANE,
          i::PRESENCE_OF_MIND, TALENT, 0, 180 * seconds, 0, 0, 0, 0, v::LEVEL, 0, a::SELF, s::ARCANE, p::NONE,
          { 30 } },

        // hunter
        { "Aspect of the Monkey", "Raises your chance to dodge by 8% until you change aspects.", c::HUNTER, ALL,
          i::ASPECT_MONKEY, ASPECT, 20, 0, 0, 0, 8, 0, v::LEVEL, 0, a::SELF, s::NATURE, p::NONE, { 4 } },
        { "Aspect of the Cheetah", "Run 30% faster, but getting hit dazes you.", c::HUNTER, ALL,
          i::ASPECT_CHEETAH, ASPECT, 40, 0, 0, 0, 30, 0, v::LEVEL, 0, a::SELF, s::NATURE, p::NONE, { 20 } },
        { "Wing Clip", "Damages the target and slows its movement.", c::HUNTER, ALL, i::WING_CLIP, 0, 30, 0, 0, 0,
          5, 1, v::DAMAGE, 10 * seconds, a::ENEMY, s::PHYSICAL, p::NONE, { 12, 38, 60 } },
        { "Feign Death", "Play dead: enemies lose interest and walk away.", c::HUNTER, ALL, i::FEIGN_DEATH, 0, 40,
          30 * seconds, 0, 0, 0, 0, v::LEVEL, 0, a::SELF, s::PHYSICAL, p::NONE, { 30 } },
        { "Rapid Fire", "Shoot 40% faster for 15 seconds.", c::HUNTER, MARKS, i::RAPID_FIRE, 0, 50,
          300 * seconds, 0, 0, 40, 0, v::LEVEL, 15 * seconds, a::SELF, s::PHYSICAL, p::NONE, { 26 } },
        { "Volley", "Arrows rain on the target's area while you channel.", c::HUNTER, MARKS, i::VOLLEY, CHANNELED,
          120, 0, 360, 140, 8, 2, v::DAMAGE, 0, a::ENEMY, s::PHYSICAL, p::NONE, { 40, 50, 58 } },
        { "Scatter Shot", "A short-range shot that disorients the target for 4 seconds.", c::HUNTER, MARKS,
          i::SCATTER_SHOT, TALENT, 30, 30 * seconds, 0, 60, 4, 1, v::DAMAGE, 4 * seconds, a::ENEMY, s::PHYSICAL,
          p::ARROW, { 30 } },
        { "Trueshot Aura", "Raises your attack power and ranged attack power.", c::HUNTER, MARKS,
          i::TRUESHOT_AURA, TALENT, 50, 0, 0, 0, 20, 10, v::RANK, 0, a::SELF, s::NATURE, p::NONE,
          { 40, 50, 60 } },
        { "Mongoose Bite", "Right after you dodge: a fierce counterattack.", c::HUNTER, SURV, i::MONGOOSE_BITE,
          REACTIVE, 30, 5 * seconds, 0, 0, 15, 3, v::DAMAGE, 0, a::ENEMY, s::PHYSICAL, p::NONE,
          { 16, 30, 44, 58 } },
        { "Immolation Trap", "A trap at your feet. The enemy that steps on it burns.", c::HUNTER, SURV,
          i::IMMOLATION_TRAP, 0, 50, 15 * seconds, 0, 0, 20, 5, v::DAMAGE, 15 * seconds, a::SELF, s::FIRE,
          p::NONE, { 16, 26, 36, 46, 56 } },
        { "Freezing Trap", "A trap at your feet. The enemy that steps on it freezes; damage breaks it.",
          c::HUNTER, SURV, i::FREEZING_TRAP, 0, 50, 15 * seconds, 0, 0, 10, 5, v::RANK, 0, a::SELF, s::FROST,
          p::NONE, { 20, 40, 60 } },
        { "Frost Trap", "A trap at your feet that turns into ice, slowing enemies on it.", c::HUNTER, SURV,
          i::FROST_TRAP, 0, 60, 15 * seconds, 0, 0, 60, 0, v::LEVEL, 30 * seconds, a::SELF, s::FROST, p::NONE,
          { 28 } },
        { "Explosive Trap", "A trap at your feet that explodes in fire and keeps burning.", c::HUNTER, SURV,
          i::EXPLOSIVE_TRAP, 0, 70, 15 * seconds, 0, 0, 20, 5, v::DAMAGE, 20 * seconds, a::SELF, s::FIRE,
          p::NONE, { 34, 44, 54 } },
        { "Deterrence", "You dodge every melee attack for 10 seconds.", c::HUNTER, SURV, i::DETERRENCE, 0, 0,
          300 * seconds, 0, 0, 0, 0, v::LEVEL, 10 * seconds, a::SELF, s::PHYSICAL, p::NONE, { 20 } },
        { "Wyvern Sting", "Puts the target to sleep, then poisons it. Damage wakes it.", c::HUNTER, SURV,
          i::WYVERN_STING, TALENT, 60, 120 * seconds, 0, 140, 30, 6, v::DAMAGE, 12 * seconds, a::ENEMY,
          s::NATURE, p::ARROW, { 40, 50, 60 } },

        // riding
        { "Mount", "Ride your horse, ram or nightsaber: 60% faster out of combat.", any_class, ALL, i::MOUNT, 0, 0,
          0, 90, 0, 60, 0, v::LEVEL, 0, a::SELF, s::PHYSICAL, p::NONE, { 30 } },

        // hunter: Beast Mastery's pet
        { "Tame Beast", "Channel 6 seconds to tame a beast up to your level as your pet.", c::HUNTER, BEAST,
          i::TAME_BEAST, QUEST | CHANNELED, 0, 0, 360, 60, 0, 0, v::LEVEL, 0, a::ENEMY, s::NATURE, p::NONE,
          { 10 } },
        { "Call Pet", "Calls your pet to your side, or sends it away.", c::HUNTER, BEAST, i::CALL_PET, QUEST, 0,
          0, 0, 0, 0, 0, v::LEVEL, 0, a::SELF, s::NATURE, p::NONE, { 10 } },
        { "Revive Pet", "Brings your dead pet back to life with a third of its health.", c::HUNTER, BEAST,
          i::REVIVE_PET, QUEST, 60, 0, 300, 0, 33, 0, v::LEVEL, 0, a::SELF, s::NATURE, p::NONE, { 10 } },
        { "Mend Pet", "Heals your pet over 15 seconds.", c::HUNTER, BEAST, i::MEND_PET, 0, 40, 0, 0, 0, 24, 5,
          v::DAMAGE, 15 * seconds, a::SELF, s::NATURE, p::NONE, { 12, 20, 28, 36, 44, 52, 60 } },
        { "Kill Command", "Your pet's next bite hits its target hard.", c::HUNTER, BEAST, i::KILL_COMMAND, 0, 30,
          5 * seconds, 0, 140, 20, 4, v::DAMAGE, 0, a::ENEMY, s::PHYSICAL, p::NONE, { 20, 40, 60 } },
        { "Intimidation", "Your pet's next bite stuns its target for 3 seconds.", c::HUNTER, BEAST,
          i::INTIMIDATION, TALENT, 15, 60 * seconds, 0, 140, 0, 0, v::LEVEL, 3 * seconds, a::ENEMY,
          s::PHYSICAL, p::NONE, { 30 } },
        { "Aspect of the Beast", "You and your pet deal 10% more damage until you change aspects.", c::HUNTER,
          BEAST, i::ASPECT_BEAST, ASPECT, 40, 0, 0, 0, 10, 0, v::LEVEL, 0, a::SELF, s::NATURE, p::NONE, { 30 } },
        { "Pet Passive", "Your pet stops fighting and only follows you. Again: it fights.", c::HUNTER, BEAST,
          i::PET_PASSIVE, QUEST, 0, 0, 0, 0, 0, 0, v::LEVEL, 0, a::SELF, s::NATURE, p::NONE, { 10 } },

        // mage: the Frost capstone
        { "Water Elemental", "A water elemental fights for you for 45 seconds, casting Frostbolt.", c::MAGE,
          FROST, i::WATER_ELEMENTAL, TALENT, 90, 180 * seconds, 0, 0, 20, 3, v::DAMAGE, 45 * seconds, a::SELF,
          s::FROST, p::FROST, { 40 } },
    };

    static_assert(sizeof(abilities) / sizeof(abilities[0]) == ability_count);

    // Enemy health over level, which damage keeps up with from level 20 on (see enemy_base_health).
    [[nodiscard]] constexpr int health_curve(int level)
    {
        return 30 + 10 * level + level * level;
    }
}

const ability_def& get_ability(ability_id ability)
{
    int index = int(ability);
    return abilities[index < ability_count ? index : 0];
}

int rank_count(ability_id ability)
{
    const ability_def& def = get_ability(ability);
    int count = 0;

    while(count < max_ranks && def.ranks[count])
    {
        ++count;
    }

    return count;
}

int rank_level(ability_id ability, int rank)
{
    if(rank < 1 || rank > max_ranks)
    {
        return 0;
    }

    return get_ability(ability).ranks[rank - 1];
}

int ability_level(ability_id ability)
{
    return get_ability(ability).ranks[0];
}

int rank_value(ability_id ability, int rank, int level)
{
    const ability_def& def = get_ability(ability);
    rank = bn::clamp(rank, 1, bn::max(1, rank_count(ability)));

    int next = rank_level(ability, rank + 1);
    int effective = next ? bn::min(level, next - 1) : level;
    effective = bn::max(effective, rank_level(ability, rank));
    effective = bn::max(effective, 1);

    switch(def.scaling)
    {

    case ability_scaling::RANK:
        return def.value + def.value_per_level * (rank - 1);

    case ability_scaling::DAMAGE:
    {
        int result = def.value + def.value_per_level * (bn::min(effective, 20) - 1);

        if(effective > 20)
        {
            result = result * health_curve(effective) / health_curve(20);
        }

        return result;
    }

    default:
        return def.value + def.value_per_level * (effective - 1);
    }
}

int rank_cost(ability_id ability, int rank)
{
    const ability_def& def = get_ability(ability);

    if(def.player_class == class_id::WARRIOR)
    {
        return def.cost;
    }

    int level = bn::max(1, rank_level(ability, bn::clamp(rank, 1, bn::max(1, rank_count(ability)))));
    return def.cost * (10 + level) / 11;
}

int rank_train_cost(ability_id ability, int rank)
{
    const ability_def& def = get_ability(ability);

    if(rank == 1 && (def.flags & (ability_flag::STARTER | ability_flag::TALENT | ability_flag::QUEST)))
    {
        return 0;
    }

    if(def.player_class == any_class)
    {
        return riding_cost;
    }

    int level = rank_level(ability, rank);
    return level <= 1 ? 10 : 4 * level * level;
}

bool in_kit(ability_id ability, subclass_id subclass)
{
    const ability_def& def = get_ability(ability);

    if(ability == ability_id::NONE || subclass == subclass_id::NONE)
    {
        return false;
    }

    if(def.player_class == any_class)
    {
        return true;
    }

    if(def.player_class != subclass_class(subclass))
    {
        return false;
    }

    return def.kits & (1 << subclass_index(subclass));
}

bar_id default_bar(ability_id ability)
{
    switch(ability)
    {

    case ability_id::BATTLE_SHOUT:
    case ability_id::FROST_ARMOR:
    case ability_id::ARCANE_INTELLECT:
    case ability_id::CONJURE_WATER:
    case ability_id::CONJURE_FOOD:
    case ability_id::TELEPORT_STORMWIND:
    case ability_id::MOLTEN_ARMOR:
    case ability_id::MAGE_ARMOR:
    case ability_id::ASPECT_OF_THE_HAWK:
    case ability_id::ASPECT_OF_THE_MONKEY:
    case ability_id::ASPECT_OF_THE_CHEETAH:
    case ability_id::TRUESHOT_AURA:
    case ability_id::MOUNT:
    case ability_id::ASPECT_OF_THE_BEAST:
        return bar_id::BUFFS;

    case ability_id::HAMSTRING:
    case ability_id::LAST_STAND:
    case ability_id::PUMMEL:
    case ability_id::SHIELD_BASH:
    case ability_id::INTIMIDATING_SHOUT:
    case ability_id::RETALIATION:
    case ability_id::SWEEPING_STRIKES:
    case ability_id::WHIRLING_BLADES:
    case ability_id::BERSERKER_RAGE:
    case ability_id::DEMORALIZING_SHOUT:
    case ability_id::RECKLESSNESS:
    case ability_id::DEATH_WISH:
    case ability_id::DISARM:
    case ability_id::SHIELD_WALL:
    case ability_id::ICE_BARRIER:
    case ability_id::ARCANE_POWER:
    case ability_id::BLINK:
    case ability_id::COUNTERSPELL:
    case ability_id::POLYMORPH:
    case ability_id::FIRE_WARD:
    case ability_id::COMBUSTION:
    case ability_id::COLD_SNAP:
    case ability_id::ICE_BLOCK:
    case ability_id::MANA_SHIELD:
    case ability_id::EVOCATION:
    case ability_id::PRESENCE_OF_MIND:
    case ability_id::BESTIAL_WRATH:
    case ability_id::FEIGN_DEATH:
    case ability_id::RAPID_FIRE:
    case ability_id::SCATTER_SHOT:
    case ability_id::FREEZING_TRAP:
    case ability_id::FROST_TRAP:
    case ability_id::DETERRENCE:
    case ability_id::WYVERN_STING:
    case ability_id::TAME_BEAST:
    case ability_id::CALL_PET:
    case ability_id::REVIVE_PET:
    case ability_id::MEND_PET:
    case ability_id::INTIMIDATION:
    case ability_id::PET_PASSIVE:
    case ability_id::WATER_ELEMENTAL:
        return bar_id::UTILITY;

    default:
        return bar_id::COMBAT;
    }
}

}
