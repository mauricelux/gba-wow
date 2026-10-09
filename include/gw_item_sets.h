#ifndef GW_ITEM_SETS_H
#define GW_ITEM_SETS_H

#include "gw_ids.h"
#include "gw_item_ids.h"

namespace gw
{

struct stat_bonus;

// The dungeon sets, one per class, after WoW's Dungeon Set 1. Each dungeon's last boss from
// Blackrock Depths on drops the piece of the hero's own class, and wearing pieces together gives
// bonuses.
enum class item_set : uint8_t
{
    NONE,
    VALOR,          // Battlegear of Valor, mail for warriors
    MAGISTERS,      // Magister's Regalia, cloth for mages
    BEASTSTALKER,   // Beaststalker Armor, leather for hunters
    COUNT
};

constexpr int item_set_pieces = 5;

// One bonus line of a set: from how many pieces it counts and what it says.
struct item_set_bonus
{
    int pieces;
    const char* text;
};

constexpr int item_set_bonuses = 3;

[[nodiscard]] item_set get_item_set(item_id item);

// "Battlegear of Valor".
[[nodiscard]] const char* item_set_name(item_set set);

[[nodiscard]] const item_set_bonus& get_item_set_bonus(item_set set, int index);

// How many pieces of the set the hero wears.
[[nodiscard]] int item_set_worn(item_set set);

// Adds the bonuses of the sets the hero wears.
void add_item_set_bonuses(stat_bonus& bonus);

// The set piece the boss drops for the hero's class, or NONE.
[[nodiscard]] item_id item_set_drop(enemy_id boss);

}

#endif
