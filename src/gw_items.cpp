#include "gw_items.h"

#include "gw_ui.h"

namespace gw
{

namespace
{
    constexpr item_quality P = item_quality::POOR;
    constexpr item_quality C = item_quality::COMMON;
    constexpr item_quality U = item_quality::UNCOMMON;
    constexpr item_quality R = item_quality::RARE;

    constexpr item_type FOOD = item_type::FOOD;
    constexpr item_type DRINK = item_type::DRINK;
    constexpr item_type POTION = item_type::POTION;
    constexpr item_type CLOTH = item_type::CLOTH;
    constexpr item_type LEATHER = item_type::LEATHER;
    constexpr item_type MAIL = item_type::MAIL;
    constexpr item_type SWORD = item_type::SWORD;
    constexpr item_type AXE = item_type::AXE;
    constexpr item_type MACE = item_type::MACE;
    constexpr item_type DAGGER = item_type::DAGGER;
    constexpr item_type STAFF = item_type::STAFF;
    constexpr item_type TWO_HANDED = item_type::TWO_HANDED;
    constexpr item_type BOW = item_type::BOW;
    constexpr item_type GUN = item_type::GUN;
    constexpr item_type HEARTHSTONE = item_type::HEARTHSTONE;

    constexpr equip_slot HEAD = equip_slot::HEAD;
    constexpr equip_slot CHEST = equip_slot::CHEST;
    constexpr equip_slot HANDS = equip_slot::HANDS;
    constexpr equip_slot LEGS = equip_slot::LEGS;
    constexpr equip_slot FEET = equip_slot::FEET;

    [[nodiscard]] constexpr int quality_percent(item_quality quality)
    {
        return quality == P ? 80 : quality == C ? 100 : quality == U ? 115 : quality == R ? 132 : 150;
    }

    // What a vendor pays: grows with the level, the quality and the kind of item.
    [[nodiscard]] constexpr uint16_t price(int level, item_quality quality, int percent)
    {
        int base = 5 + level * 5 + level * level * 2;
        int quality_factor = quality == P ? 50 : quality == C ? 100 : quality == U ? 250 : quality == R ? 500 : 1000;
        return uint16_t(base * quality_factor / 100 * percent / 100);
    }

    [[nodiscard]] constexpr item_def weapon(const char* name, item_quality quality, item_type type, int level,
                                            int speed, int str = 0, int agi = 0, int sta = 0, int intel = 0,
                                            int spi = 0)
    {
        bool two_handed = type == STAFF || type == TWO_HANDED;
        bool ranged = type == BOW || type == GUN;
        int dps = (150 + 45 * level) * quality_percent(quality) / 100 * (two_handed ? 135 : 100) / 100;
        int min = (dps * speed * 3 + 2000) / 4000;
        int max = (dps * speed * 5 + 2000) / 4000;
        return { name, quality, type, ranged ? equip_slot::RANGED : equip_slot::MAIN_HAND, uint8_t(level), 0,
                 int8_t(str), int8_t(agi), int8_t(sta), int8_t(intel), int8_t(spi),
                 int16_t(min < 1 ? 1 : min), int16_t(max < 2 ? 2 : max), uint8_t(speed),
                 price(level, quality, two_handed ? 180 : 150), 1 };
    }

    [[nodiscard]] constexpr item_def armor(const char* name, item_quality quality, item_type type, equip_slot slot,
                                           int level, int str = 0, int agi = 0, int sta = 0, int intel = 0,
                                           int spi = 0)
    {
        int type_factor = type == CLOTH ? 2 : type == LEATHER ? 4 : 7;
        int slot_percent = slot == CHEST ? 100 : slot == LEGS ? 85 : slot == HEAD ? 75 : slot == FEET ? 65 : 55;
        int value = type_factor * (level + 5) * slot_percent / 100 * (quality == U ? 110 : quality == R ? 125 : 100) /
                100;
        int price_percent = slot == CHEST ? 100 : slot == LEGS ? 90 : slot == HEAD ? 80 : slot == FEET ? 70 : 60;
        return { name, quality, type, slot, uint8_t(level), int16_t(value), int8_t(str), int8_t(agi), int8_t(sta),
                 int8_t(intel), int8_t(spi), 0, 0, 0, price(level, quality, price_percent), 1 };
    }

    [[nodiscard]] constexpr item_def shield(const char* name, item_quality quality, int level, int str = 0,
                                            int agi = 0, int sta = 0, int intel = 0, int spi = 0)
    {
        int value = 10 * (level + 5) * (quality == U ? 110 : quality == R ? 125 : 100) / 100;
        return { name, quality, item_type::SHIELD, equip_slot::OFF_HAND, uint8_t(level), int16_t(value),
                 int8_t(str), int8_t(agi), int8_t(sta), int8_t(intel), int8_t(spi), 0, 0, 0,
                 price(level, quality, 100), 1 };
    }

    [[nodiscard]] constexpr item_def consumable(const char* name, item_type type, int level, int amount,
                                                int sell_price, int stack)
    {
        // Potions roll their amount in a range; food and drink restore exactly the amount.
        int min = type == POTION ? amount * 85 / 100 : amount;
        int max = type == POTION ? amount * 115 / 100 : amount;
        return { name, C, type, equip_slot::NONE, uint8_t(level), 0, 0, 0, 0, 0, 0, int16_t(min), int16_t(max), 0,
                 uint16_t(sell_price), uint8_t(stack) };
    }

    [[nodiscard]] constexpr item_def junk(const char* name, int sell_price, int stack = 10)
    {
        return { name, P, item_type::JUNK, equip_slot::NONE, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, uint16_t(sell_price),
                 uint8_t(stack) };
    }

    [[nodiscard]] constexpr item_def special(const char* name, item_type type)
    {
        return { name, C, type, equip_slot::NONE, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };
    }

    #define GW_ITEM_DEF(id, definition) definition,

    constexpr item_def items[] = {
        GW_ITEM_LIST(GW_ITEM_DEF)
    };

    #undef GW_ITEM_DEF

    static_assert(sizeof(items) / sizeof(items[0]) == int(item_id::COUNT));
}

const item_def& get_item(item_id item)
{
    int index = int(item);
    return items[index < int(item_id::COUNT) ? index : 0];
}

bool is_two_handed(const item_def& item)
{
    return item.type == item_type::STAFF || item.type == item_type::TWO_HANDED;
}

bool can_equip(class_id player_class, const item_def& item)
{
    if(item.slot == equip_slot::NONE)
    {
        return false;
    }

    switch(player_class)
    {

    case class_id::WARRIOR:
        return item.type != item_type::WAND && item.type != item_type::STAFF;

    case class_id::MAGE:
        return item.type == item_type::CLOTH || item.type == item_type::STAFF || item.type == item_type::DAGGER ||
                item.type == item_type::SWORD || item.type == item_type::WAND;

    case class_id::HUNTER:
        return item.type == item_type::CLOTH || item.type == item_type::LEATHER || item.type == item_type::AXE ||
                item.type == item_type::SWORD || item.type == item_type::DAGGER ||
                item.type == item_type::TWO_HANDED || item.type == item_type::STAFF ||
                item.type == item_type::BOW || item.type == item_type::GUN;

    default:
        return false;
    }
}

uint8_t quality_color(item_quality quality)
{
    switch(quality)
    {

    case item_quality::POOR:
        return uint8_t(ui::color::GRAY);

    case item_quality::UNCOMMON:
        return uint8_t(ui::color::GREEN);

    case item_quality::RARE:
        return uint8_t(ui::color::BLUE);

    case item_quality::EPIC:
        return uint8_t(ui::color::PURPLE);

    default:
        return uint8_t(ui::color::WHITE);
    }
}

}
