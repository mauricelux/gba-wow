#ifndef GW_TALENTS_H
#define GW_TALENTS_H

namespace gw
{

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
};

[[nodiscard]] talent_bonus talent_bonuses();

}

#endif
