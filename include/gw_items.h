#ifndef GW_ITEMS_H
#define GW_ITEMS_H

#include <cstdint>

#include "gw_ids.h"

namespace gw
{

enum class item_id : uint16_t;

enum class item_quality : uint8_t
{
    POOR,       // gray junk, only worth selling
    COMMON,     // white
    UNCOMMON,   // green
    RARE,       // blue
    EPIC        // purple
};

enum class equip_slot : uint8_t
{
    HEAD,
    CHEST,
    HANDS,
    LEGS,
    FEET,
    MAIN_HAND,
    OFF_HAND,
    RANGED,
    COUNT,
    NONE = COUNT
};

enum class item_type : uint8_t
{
    JUNK,
    QUEST,
    FOOD,       // restores health while sitting
    DRINK,      // restores mana while sitting
    POTION,     // restores health instantly
    CLOTH,
    LEATHER,
    MAIL,
    SHIELD,
    SWORD,
    AXE,
    MACE,
    DAGGER,
    STAFF,      // two-handed
    TWO_HANDED, // two-handed swords, axes and maces
    BOW,
    GUN,
    WAND,
    HEARTHSTONE,
    REAGENT     // used up by abilities
};

struct item_def
{
    const char* name;
    item_quality quality;
    item_type type;
    equip_slot slot;
    uint8_t level;          // required level
    int16_t armor;
    int8_t strength;
    int8_t agility;
    int8_t stamina;
    int8_t intellect;
    int8_t spirit;
    int16_t min_damage;     // weapons; for consumables the amount restored
    int16_t max_damage;
    uint8_t speed;          // weapons: tenths of a second between swings
    uint16_t price;         // what a vendor pays, in copper; vendors sell for four times as much
    uint8_t stack;          // how many fit in one bag slot
};

[[nodiscard]] const item_def& get_item(item_id item);

[[nodiscard]] bool is_two_handed(const item_def& item);

// Weapons and armor the class can equip.
[[nodiscard]] bool can_equip(class_id player_class, const item_def& item);

// Display color for the item's quality (a gw::ui::color value).
[[nodiscard]] uint8_t quality_color(item_quality quality);

}

#include "gw_item_ids.h"

#endif
