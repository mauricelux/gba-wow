#include "gw_vendors.h"

namespace gw
{

namespace
{
    using i = item_id;

    constexpr item_id general_goods[] = {
        i::TOUGH_JERKY, i::SPRING_WATER, i::MINOR_HEALING_POTION
    };

    constexpr item_id weapons[] = {
        i::GLADIUS, i::HATCHET, i::CUDGEL, i::DIRK, i::WALKING_STAFF, i::CLAYMORE, i::HUNTING_BOW,
        i::WOODEN_BUCKLER
    };

    constexpr item_id armor[] = {
        i::CHAIN_COIF, i::CHAIN_VEST, i::CHAIN_GLOVES, i::CHAIN_LEGGINGS, i::CHAIN_BOOTS,
        i::LEATHER_CAP, i::LEATHER_VEST, i::LEATHER_GLOVES, i::LEATHER_PANTS, i::LEATHER_BOOTS,
        i::LINEN_HOOD, i::LINEN_ROBE, i::LINEN_GLOVES, i::LINEN_PANTS, i::LINEN_BOOTS
    };

    constexpr item_id inn[] = {
        i::TOUGH_JERKY, i::HAUNCH_OF_MEAT, i::SPRING_WATER, i::ICE_COLD_MILK, i::MINOR_HEALING_POTION,
        i::LESSER_HEALING_POTION
    };

    constexpr item_id militia[] = {
        i::HAUNCH_OF_MEAT, i::MUTTON_CHOP, i::ICE_COLD_MILK, i::MELON_JUICE, i::LESSER_HEALING_POTION,
        i::HEALING_POTION,
        i::BROADSWORD, i::MORNING_STAR, i::STILETTO, i::GNARLED_STAFF, i::BATTLE_AXE, i::RECURVE_BOW,
        i::LIGHT_MUSKET, i::ROUND_SHIELD,
        i::SCALE_COIF, i::SCALE_VEST, i::SCALE_GLOVES, i::SCALE_LEGGINGS, i::SCALE_BOOTS,
        i::HARDENED_CAP, i::HARDENED_VEST, i::HARDENED_GLOVES, i::HARDENED_PANTS, i::HARDENED_BOOTS,
        i::WOOLEN_HOOD, i::WOOLEN_ROBE, i::WOOLEN_GLOVES, i::WOOLEN_PANTS, i::WOOLEN_BOOTS
    };

    constexpr item_id stormwind_goods[] = {
        i::TOUGH_JERKY, i::HAUNCH_OF_MEAT, i::SPRING_WATER, i::ICE_COLD_MILK, i::MINOR_HEALING_POTION,
        i::LESSER_HEALING_POTION
    };

    constexpr item_id stormwind_weapons[] = {
        i::LONGSWORD, i::WAR_AXE, i::FLANGED_MACE, i::KRIS, i::QUARTERSTAFF, i::GREATSWORD, i::COMPOSITE_BOW,
        i::FLINTLOCK, i::KITE_SHIELD
    };

    constexpr item_id stormwind_armor[] = {
        i::RINGMAIL_COIF, i::RINGMAIL_VEST, i::RINGMAIL_GLOVES, i::RINGMAIL_LEGGINGS, i::RINGMAIL_BOOTS,
        i::STUDDED_CAP, i::STUDDED_VEST, i::STUDDED_GLOVES, i::STUDDED_PANTS, i::STUDDED_BOOTS,
        i::PADDED_HOOD, i::PADDED_ROBE, i::PADDED_GLOVES, i::PADDED_PANTS, i::PADDED_BOOTS
    };
}

bn::span<const item_id> vendor_stock(int vendor)
{
    switch(vendor)
    {

    case 1:
        return weapons;

    case 2:
        return armor;

    case 3:
        return inn;

    case 4:
        return militia;

    case 5:
        return stormwind_goods;

    case 6:
        return stormwind_weapons;

    case 7:
        return stormwind_armor;

    default:
        return general_goods;
    }
}

}
