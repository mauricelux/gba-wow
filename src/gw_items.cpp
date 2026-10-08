#include "gw_items.h"

#include "gw_ui.h"

namespace gw
{

namespace
{
    using q = item_quality;
    using t = item_type;
    using s = equip_slot;

    //   name, quality, type, slot, level, armor, str, agi, sta, int, spi, min, max, speed, price, stack
    constexpr item_def items[] = {
        { "Nothing", q::POOR, t::JUNK, s::NONE, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        // starting gear
        { "Worn Shortsword", q::COMMON, t::SWORD, s::MAIN_HAND, 1, 0, 0, 0, 0, 0, 0, 2, 5, 19, 7, 1 },
        { "Bent Staff", q::COMMON, t::STAFF, s::MAIN_HAND, 1, 0, 0, 0, 0, 0, 0, 3, 6, 29, 9, 1 },
        { "Worn Hatchet", q::COMMON, t::AXE, s::MAIN_HAND, 1, 0, 0, 0, 0, 0, 0, 2, 5, 20, 7, 1 },
        { "Cracked Shortbow", q::COMMON, t::BOW, s::RANGED, 1, 0, 0, 0, 0, 0, 0, 2, 5, 23, 7, 1 },
        { "Recruit's Vest", q::COMMON, t::MAIL, s::CHEST, 1, 12, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1 },
        { "Recruit's Pants", q::COMMON, t::MAIL, s::LEGS, 1, 9, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1 },
        { "Recruit's Boots", q::COMMON, t::MAIL, s::FEET, 1, 6, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1 },
        { "Apprentice's Robe", q::COMMON, t::CLOTH, s::CHEST, 1, 4, 0, 0, 0, 1, 0, 0, 0, 0, 2, 1 },
        { "Apprentice's Pants", q::COMMON, t::CLOTH, s::LEGS, 1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1 },
        { "Apprentice's Boots", q::COMMON, t::CLOTH, s::FEET, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1 },
        { "Trapper's Vest", q::COMMON, t::LEATHER, s::CHEST, 1, 8, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1 },
        { "Trapper's Pants", q::COMMON, t::LEATHER, s::LEGS, 1, 6, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1 },
        { "Trapper's Boots", q::COMMON, t::LEATHER, s::FEET, 1, 4, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1 },
    };

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
