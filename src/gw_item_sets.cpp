#include "gw_item_sets.h"

#include "gw_character.h"
#include "gw_items.h"

namespace gw
{

namespace
{
    using i = item_id;

    struct set_def
    {
        const char* name;
        item_id pieces[item_set_pieces];    // hands, chest, head, feet, legs: in the order they drop
        item_set_bonus bonuses[item_set_bonuses];
    };

    constexpr set_def sets[] = {
        { "", {}, {} },
        { "Battlegear of Valor",
          { i::GAUNTLETS_OF_VALOR, i::BREASTPLATE_OF_VALOR, i::HELM_OF_VALOR, i::BOOTS_OF_VALOR,
            i::LEGPLATES_OF_VALOR },
          { { 2, "+150 armor" }, { 3, "+30 attack power" }, { 5, "+2% crit, +5% health" } } },
        { "Magister's Regalia",
          { i::MAGISTERS_GLOVES, i::MAGISTERS_ROBES, i::MAGISTERS_CROWN, i::MAGISTERS_BOOTS,
            i::MAGISTERS_LEGGINGS },
          { { 2, "+20 intellect" }, { 3, "+2% spell crit" }, { 5, "+6% damage" } } },
        { "Beaststalker Armor",
          { i::BEASTSTALKERS_GLOVES, i::BEASTSTALKERS_TUNIC, i::BEASTSTALKERS_CAP, i::BEASTSTALKERS_BOOTS,
            i::BEASTSTALKERS_PANTS },
          { { 2, "+30 ranged attack power" }, { 3, "+2% crit" }, { 5, "+8% ranged haste" } } },
    };

    static_assert(sizeof(sets) / sizeof(sets[0]) == int(item_set::COUNT));

    // The bosses that drop a piece, in the order of the pieces.
    constexpr enemy_id set_bosses[item_set_pieces] = {
        enemy_id::EMPEROR_DAGRAN_THAURISSAN, enemy_id::GENERAL_DRAKKISATH, enemy_id::DARKMASTER_GANDLING,
        enemy_id::BALNAZZAR, enemy_id::BARON_RIVENDARE
    };

    [[nodiscard]] item_set class_set(class_id player_class)
    {
        switch(player_class)
        {

        case class_id::WARRIOR:
            return item_set::VALOR;

        case class_id::MAGE:
            return item_set::MAGISTERS;

        case class_id::HUNTER:
            return item_set::BEASTSTALKER;

        default:
            return item_set::NONE;
        }
    }
}

item_set get_item_set(item_id item)
{
    for(int set = 1; set < int(item_set::COUNT); ++set)
    {
        for(item_id piece : sets[set].pieces)
        {
            if(piece == item)
            {
                return item_set(set);
            }
        }
    }

    return item_set::NONE;
}

const char* item_set_name(item_set set)
{
    return sets[int(set)].name;
}

const item_set_bonus& get_item_set_bonus(item_set set, int index)
{
    return sets[int(set)].bonuses[index];
}

int item_set_worn(item_set set)
{
    int result = 0;

    for(item_id equipped : character().equipment)
    {
        if(equipped != item_id::NONE && get_item_set(equipped) == set)
        {
            ++result;
        }
    }

    return result;
}

void add_item_set_bonuses(stat_bonus& bonus)
{
    int valor = item_set_worn(item_set::VALOR);
    int magisters = item_set_worn(item_set::MAGISTERS);
    int beaststalker = item_set_worn(item_set::BEASTSTALKER);

    if(valor >= 2)
    {
        bonus.armor += 150;
    }

    if(valor >= 3)
    {
        bonus.attack_power += 30;
    }

    if(valor >= 5)
    {
        bonus.crit += 2;
        bonus.health_percent += 5;
    }

    if(magisters >= 2)
    {
        bonus.intellect += 20;
    }

    if(magisters >= 3)
    {
        bonus.spell_crit += 2;
    }

    if(magisters >= 5)
    {
        bonus.damage_percent += 6;
    }

    if(beaststalker >= 2)
    {
        bonus.ranged_attack_power += 30;
    }

    if(beaststalker >= 3)
    {
        bonus.crit += 2;
    }

    if(beaststalker >= 5)
    {
        bonus.ranged_haste_percent += 8;
    }
}

item_id item_set_drop(enemy_id boss)
{
    item_set set = class_set(character().player_class);

    if(set == item_set::NONE)
    {
        return item_id::NONE;
    }

    for(int index = 0; index < item_set_pieces; ++index)
    {
        if(set_bosses[index] == boss)
        {
            return sets[int(set)].pieces[index];
        }
    }

    return item_id::NONE;
}

}
