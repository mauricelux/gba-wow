#include "gw_enemy_abilities.h"

namespace gw
{

namespace
{
    using e = enemy_effect;
    using t = enemy_target;
    using b = buff_id;
    using s = school;
    using p = projectile_kind;
    using namespace enemy_ability_flag;

    constexpr uint8_t melee = melee_range;
    constexpr uint8_t CAST = SPELL | INTERRUPTIBLE;
    constexpr enemy_id none = enemy_id::NONE;

    // name, effect, target, debuff, school, projectile, flags, range, radius, cast time (tenths), cooldown,
    // duration, value, debuff value, health below, hits, summon
    constexpr enemy_ability_def abilities[] = {
        { "", e::HIT, t::PLAYER, b::COUNT, s::PHYSICAL, p::NONE, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, none },

        // Caster: casts from range that an interrupt stops.
        { "Fireball", e::HIT, t::PLAYER, b::COUNT, s::FIRE, p::FIRE, CAST, 96, 0, 25, 3, 0, 180, 0, 0, 1, none },
        { "Frostbolt", e::HIT, t::PLAYER, b::CHILLED, s::FROST, p::FROST, CAST, 96, 0, 20, 3, 5, 140, 40, 0, 1,
          none },
        { "Lightning Bolt", e::HIT, t::PLAYER, b::COUNT, s::NATURE, p::NATURE, CAST, 96, 0, 20, 3, 0, 160, 0, 0, 1,
          none },
        { "Healing Wave", e::HEAL, t::PLAYER, b::COUNT, s::NATURE, p::NONE, CAST, 96, 0, 25, 10, 0, 35, 0, 50, 1,
          none },
        { "Shadow Bolt", e::HIT, t::PLAYER, b::COUNT, s::SHADOW, p::SHADOW, CAST, 96, 0, 25, 3, 0, 180, 0, 0, 1,
          none },
        { "Holy Smite", e::HIT, t::PLAYER, b::COUNT, s::HOLY, p::HOLY, CAST, 96, 0, 20, 3, 0, 150, 0, 0, 1, none },
        { "Heal", e::HEAL, t::PLAYER, b::COUNT, s::HOLY, p::NONE, CAST, 96, 0, 25, 10, 0, 40, 0, 50, 1, none },

        // Melee: strikes in place of a swing.
        { "Rend", e::HIT, t::PLAYER, b::BLEEDING, s::PHYSICAL, p::NONE, SWING, melee, 0, 0, 12, 15, 100, 25, 0, 1,
          none },
        { "Sunder Armor", e::HIT, t::PLAYER, b::SUNDERED, s::PHYSICAL, p::NONE, SWING, melee, 0, 0, 6, 20, 100,
          10, 0, 1, none },
        { "Thrash", e::HIT, t::PLAYER, b::COUNT, s::PHYSICAL, p::NONE, SWING, melee, 0, 0, 12, 0, 80, 0, 0, 3,
          none },
        { "Mortal Strike", e::HIT, t::PLAYER, b::WOUNDED, s::PHYSICAL, p::NONE, SWING, melee, 0, 0, 10, 10, 200,
          50, 0, 1, none },
        { "Cleave", e::HIT, t::PLAYER, b::COUNT, s::PHYSICAL, p::NONE, SWING, melee, 0, 0, 8, 0, 160, 0, 0, 1,
          none },
        { "Knockdown", e::HIT, t::PLAYER, b::STUNNED, s::PHYSICAL, p::NONE, SWING, melee, 0, 0, 15, 2, 80, 0, 0, 1,
          none },

        // Control
        { "Net", e::HIT, t::PLAYER, b::ROOTED, s::PHYSICAL, p::NONE, INTERRUPTIBLE, 64, 0, 10, 20, 5, 0, 0, 0, 1,
          none },
        { "Frost Nova", e::HIT, t::AROUND_SELF, b::ROOTED, s::FROST, p::NONE, CAST, 40, 40, 10, 15, 5, 50, 0, 0, 1,
          none },
        { "Fear", e::HIT, t::PLAYER, b::FEARED, s::SHADOW, p::NONE, CAST, 80, 0, 15, 20, 4, 0, 0, 0, 1, none },
        { "Sleep", e::HIT, t::PLAYER, b::ASLEEP, s::NATURE, p::NONE, CAST, 80, 0, 15, 25, 6, 0, 0, 0, 1, none },
        { "Polymorph", e::HIT, t::PLAYER, b::POLYMORPHED, s::ARCANE, p::NONE, CAST, 80, 0, 15, 25, 6, 0, 0, 0, 1,
          none },
        { "Web", e::HIT, t::PLAYER, b::ROOTED, s::NATURE, p::NONE, 0, 64, 0, 0, 20, 6, 0, 0, 0, 1, none },

        // Over time: a strike or a curse that keeps hurting every three seconds.
        { "Poison", e::HIT, t::PLAYER, b::POISONED, s::NATURE, p::NONE, SWING, melee, 0, 0, 10, 15, 100, 20, 0, 1,
          none },
        { "Disease", e::HIT, t::PLAYER, b::DISEASED, s::NATURE, p::NONE, SWING, melee, 0, 0, 12, 21, 100, 20, 0, 1,
          none },
        { "Curse of Weakness", e::HIT, t::PLAYER, b::WEAKENED, s::SHADOW, p::NONE, SPELL, 80, 0, 0, 30, 30, 0, 20,
          0, 1, none },
        { "Burning", e::HIT, t::PLAYER, b::BURNING, s::FIRE, p::NONE, SWING, melee, 0, 0, 8, 9, 100, 30, 0, 1,
          none },

        // Movement
        { "Charge", e::CHARGE, t::PLAYER, b::STUNNED, s::PHYSICAL, p::NONE, 0, 110, 0, 0, 15, 1, 100, 0, 0, 1,
          none },
        { "Leap", e::CHARGE, t::PLAYER, b::COUNT, s::PHYSICAL, p::NONE, 0, 90, 0, 0, 12, 0, 120, 0, 0, 1, none },
        { "War Stomp", e::HIT, t::AROUND_SELF, b::STUNNED, s::PHYSICAL, p::NONE, 0, 36, 36, 10, 15, 2, 60, 0, 0, 1,
          none },
        { "Blink", e::BLINK, t::PLAYER, b::COUNT, s::ARCANE, p::NONE, SPELL, 64, 0, 0, 15, 0, 0, 0, 0, 1, none },

        // Support
        { "Call for Help", e::CALL_FOR_HELP, t::PLAYER, b::COUNT, s::PHYSICAL, p::NONE, INTERRUPTIBLE | ONCE, 0,
          100, 10, 0, 0, 0, 0, 60, 1, none },
        { "Enrage", e::ENRAGE, t::PLAYER, b::COUNT, s::PHYSICAL, p::NONE, ONCE, 0, 0, 0, 0, 0, 30, 0, 30, 1, none },
        { "Battle Shout", e::RALLY, t::PLAYER, b::COUNT, s::PHYSICAL, p::NONE, 0, 0, 80, 0, 60, 30, 20, 0, 0, 1,
          none },
        { "Shield Wall", e::GUARD, t::PLAYER, b::COUNT, s::PHYSICAL, p::NONE, ONCE, 0, 0, 0, 0, 10, 50, 0, 30, 1,
          none },
        { "Summon Skeleton", e::SUMMON, t::PLAYER, b::COUNT, s::SHADOW, p::NONE, CAST, 0, 0, 20, 20, 0, 0, 0, 0, 1,
          enemy_id::SKELETAL_SERVANT },

        // Defense
        { "Shield Block", e::GUARD, t::PLAYER, b::COUNT, s::PHYSICAL, p::NONE, PHYSICAL, melee, 0, 0, 15, 6, 40, 0,
          0, 1, none },
        { "Evasion", e::EVASION, t::PLAYER, b::COUNT, s::PHYSICAL, p::NONE, ONCE, 0, 0, 0, 0, 10, 50, 0, 50, 1,
          none },
        { "Stoneskin", e::GUARD, t::PLAYER, b::COUNT, s::NATURE, p::NONE, PHYSICAL, 0, 0, 0, 30, 15, 30, 0, 0, 1,
          none },
        { "Mana Shield", e::ABSORB, t::PLAYER, b::COUNT, s::ARCANE, p::NONE, SPELL | ONCE, 0, 0, 0, 0, 0, 25, 0, 70,
          1, none },

        // Kobold Tunnelers throw the candles off their helmets.
        { "Candle Throw", e::HIT, t::PLAYER, b::COUNT, s::FIRE, p::FIRE, INTERRUPTIBLE, 64, 0, 10, 8, 0, 120, 0, 0,
          1, none },

        // Lady Sarevess: lightning that strikes where the player stood.
        { "Forked Lightning", e::HIT, t::AT_PLAYER, b::COUNT, s::NATURE, p::NONE, CAST, 96, 32, 15, 8, 0, 200, 0, 0,
          1, none },
        // Walking Bombs run up to the player and blow up; step out of the circle.
        { "Self-Destruct", e::HIT, t::AROUND_SELF, b::COUNT, s::FIRE, p::NONE, ONCE, 24, 36, 10, 0, 0, 250, 0, 0, 1,
          none },
        { "Toxic Volley", e::HIT, t::AROUND_SELF, b::POISONED, s::NATURE, p::NONE, CAST, 48, 48, 15, 12, 15, 80, 25,
          0, 1, none },
        { "Crowd Pummel", e::HIT, t::AROUND_SELF, b::STUNNED, s::PHYSICAL, p::NONE, 0, 40, 44, 15, 12, 2, 150, 0, 0,
          1, none },
        { "Throw Dynamite", e::HIT, t::AT_PLAYER, b::COUNT, s::FIRE, p::NONE, INTERRUPTIBLE, 80, 28, 12, 10, 0, 140,
          0, 0, 1, none },

        // Bloodmage Thalnos: a spike of fire where the player stood.
        { "Flame Spike", e::HIT, t::AT_PLAYER, b::BURNING, s::FIRE, p::NONE, CAST, 96, 28, 15, 7, 6, 180, 30, 0, 1,
          none },
        // Arcanist Doan, from half health: nothing can stop it, so get out of the circle.
        { "Detonation", e::HIT, t::AROUND_SELF, b::COUNT, s::ARCANE, p::NONE, SPELL, 64, 60, 30, 20, 0, 255, 0, 50,
          1, none },
        { "Arcane Explosion", e::HIT, t::AROUND_SELF, b::COUNT, s::ARCANE, p::NONE, CAST, 36, 40, 10, 8, 0, 110, 0,
          0, 1, none },
        { "Psychic Scream", e::HIT, t::AROUND_SELF, b::FEARED, s::SHADOW, p::NONE, SPELL, 40, 44, 10, 25, 4, 0, 0,
          0, 1, none },
        { "Poison Cloud", e::HIT, t::AT_PLAYER, b::POISONED, s::NATURE, p::NONE, CAST, 80, 36, 15, 12, 15, 60, 30,
          0, 1, none },

        // Herod spins in place after a long wind-up: nothing stops it, so step out of the circle.
        { "Whirlwind", e::HIT, t::AROUND_SELF, b::COUNT, s::PHYSICAL, p::NONE, 0, 40, 44, 20, 14, 0, 170, 0, 0, 1,
          none },
        // High Inquisitor Whitemane, from half health: everyone near her falls asleep.
        { "Deep Sleep", e::HIT, t::AROUND_SELF, b::ASLEEP, s::HOLY, p::NONE, SPELL | ONCE, 64, 72, 15, 0, 4, 0, 0,
          50, 1, none },
    };

    static_assert(sizeof(abilities) / sizeof(abilities[0]) == enemy_ability_count, "an entry per enemy ability");
}

const enemy_ability_def& get_enemy_ability(enemy_ability_id ability)
{
    return abilities[int(ability)];
}

}
