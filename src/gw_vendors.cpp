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
        i::LESSER_HEALING_POTION, i::TELEPORTATION_RUNE
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
        i::LESSER_HEALING_POTION, i::TELEPORTATION_RUNE
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

    constexpr item_id lakeshire[] = {
        i::HAUNCH_OF_MEAT, i::MUTTON_CHOP, i::ICE_COLD_MILK, i::MELON_JUICE, i::LESSER_HEALING_POTION,
        i::HEALING_POTION, i::TELEPORTATION_RUNE
    };

    constexpr item_id darkshire[] = {
        i::MUTTON_CHOP, i::WILD_HOG_SHANK, i::MELON_JUICE, i::SWEET_NECTAR, i::HEALING_POTION,
        i::GREATER_HEALING_POTION, i::TELEPORTATION_RUNE
    };

    constexpr item_id ironforge_goods[] = {
        i::WILD_HOG_SHANK, i::DWARVEN_MILD, i::SWEET_NECTAR, i::MOONBERRY_JUICE, i::HEALING_POTION,
        i::GREATER_HEALING_POTION, i::TELEPORTATION_RUNE
    };

    constexpr item_id ironforge_smith[] = {
        i::DWARVEN_BROADSWORD, i::DWARVEN_WAR_AXE, i::IRONFORGE_WARHAMMER, i::OAKEN_WAR_STAFF, i::DWARVEN_GREATAXE,
        i::HEAVY_RECURVE_BOW, i::DWARVEN_HAND_CANNON, i::IRONFORGE_TOWER_SHIELD,
        i::BANDED_HELM, i::BANDED_HAUBERK, i::BANDED_GAUNTLETS, i::BANDED_LEGGINGS, i::BANDED_BOOTS,
        i::THICK_LEATHER_CAP, i::THICK_LEATHER_VEST, i::THICK_LEATHER_GLOVES, i::THICK_LEATHER_PANTS,
        i::THICK_LEATHER_BOOTS,
        i::MAGEWEAVE_HOOD, i::MAGEWEAVE_ROBE, i::MAGEWEAVE_GLOVES, i::MAGEWEAVE_PANTS, i::MAGEWEAVE_BOOTS
    };

    constexpr item_id menethil[] = {
        i::WILD_HOG_SHANK, i::DWARVEN_MILD, i::SWEET_NECTAR, i::MOONBERRY_JUICE, i::GREATER_HEALING_POTION,
        i::TELEPORTATION_RUNE
    };

    constexpr item_id southshore[] = {
        i::WILD_HOG_SHANK, i::GOLDENBARK_APPLE, i::MOONBERRY_JUICE, i::MORNING_GLORY_DEW,
        i::GREATER_HEALING_POTION, i::SUPERIOR_HEALING_POTION, i::TELEPORTATION_RUNE
    };

    constexpr item_id southshore_smith[] = {
        i::HARDENED_BROADSWORD, i::SOUTHSHORE_WAR_AXE, i::HEAVY_FLANGED_MACE, i::IRONBOUND_STAFF, i::HEAVY_CLAYMORE,
        i::HILLSBRAD_LONGBOW, i::HEAVY_BLUNDERBUSS, i::HEATER_SHIELD,
        i::REINFORCED_CHAIN_HELM, i::REINFORCED_CHAIN_HAUBERK, i::REINFORCED_CHAIN_GAUNTLETS,
        i::REINFORCED_CHAIN_LEGGINGS, i::REINFORCED_CHAIN_BOOTS,
        i::HARDENED_LEATHER_CAP, i::HARDENED_LEATHER_VEST, i::HARDENED_LEATHER_GLOVES, i::HARDENED_LEATHER_PANTS,
        i::HARDENED_LEATHER_BOOTS,
        i::SILK_HOOD, i::SILK_ROBE, i::SILK_GLOVES, i::SILK_PANTS, i::SILK_BOOTS
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

    case 8:
        return lakeshire;

    case 9:
        return darkshire;

    case 10:
        return ironforge_goods;

    case 11:
        return ironforge_smith;

    case 12:
        return menethil;

    case 13:
        return southshore;

    case 14:
        return southshore_smith;

    default:
        return general_goods;
    }
}

}
