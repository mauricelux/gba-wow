#ifndef GW_ITEM_IDS_H
#define GW_ITEM_IDS_H

#include <cstdint>

// Every item, as X(ID, definition). gw_items.cpp expands the definitions with its helpers (weapon,
// armor, shield, consumable, junk); the ids become gw::item_id. Saves store item ids: only append.
//
// weapon(name, quality, type, level, speed in tenths of a second, str, agi, sta, int, spi)
// armor(name, quality, type, slot, level, str, agi, sta, int, spi)
// shield(name, quality, level, str, agi, sta, int, spi)
// consumable(name, type, level, amount restored, sell price, stack)
// junk(name, sell price, stack)
#define GW_ITEM_LIST(X) \
    X(NONE, junk("Nothing", 0)) \
    /* starting gear */ \
    X(WORN_SHORTSWORD, weapon("Worn Shortsword", C, SWORD, 1, 19)) \
    X(BENT_STAFF, weapon("Bent Staff", C, STAFF, 1, 29)) \
    X(WORN_HATCHET, weapon("Worn Hatchet", C, AXE, 1, 20)) \
    X(CRACKED_SHORTBOW, weapon("Cracked Shortbow", C, BOW, 1, 23)) \
    X(RECRUITS_VEST, armor("Recruit's Vest", C, MAIL, CHEST, 1)) \
    X(RECRUITS_PANTS, armor("Recruit's Pants", C, MAIL, LEGS, 1)) \
    X(RECRUITS_BOOTS, armor("Recruit's Boots", C, MAIL, FEET, 1)) \
    X(APPRENTICES_ROBE, armor("Apprentice's Robe", C, CLOTH, CHEST, 1, 0, 0, 0, 1, 0)) \
    X(APPRENTICES_PANTS, armor("Apprentice's Pants", C, CLOTH, LEGS, 1)) \
    X(APPRENTICES_BOOTS, armor("Apprentice's Boots", C, CLOTH, FEET, 1)) \
    X(TRAPPERS_VEST, armor("Trapper's Vest", C, LEATHER, CHEST, 1)) \
    X(TRAPPERS_PANTS, armor("Trapper's Pants", C, LEATHER, LEGS, 1)) \
    X(TRAPPERS_BOOTS, armor("Trapper's Boots", C, LEATHER, FEET, 1)) \
    /* food, drink and potions */ \
    X(TOUGH_JERKY, consumable("Tough Jerky", FOOD, 1, 60, 6, 20)) \
    X(HAUNCH_OF_MEAT, consumable("Haunch of Meat", FOOD, 5, 200, 25, 20)) \
    X(MUTTON_CHOP, consumable("Mutton Chop", FOOD, 15, 420, 100, 20)) \
    X(SPRING_WATER, consumable("Spring Water", DRINK, 1, 150, 6, 20)) \
    X(ICE_COLD_MILK, consumable("Ice Cold Milk", DRINK, 5, 400, 25, 20)) \
    X(MELON_JUICE, consumable("Melon Juice", DRINK, 15, 800, 100, 20)) \
    X(MINOR_HEALING_POTION, consumable("Minor Healing Potion", POTION, 1, 80, 10, 5)) \
    X(LESSER_HEALING_POTION, consumable("Lesser Healing Potion", POTION, 6, 160, 40, 5)) \
    X(HEALING_POTION, consumable("Healing Potion", POTION, 12, 320, 100, 5)) \
    /* junk */ \
    X(BROKEN_FANG, junk("Broken Fang", 3)) \
    X(CHIPPED_TUSK, junk("Chipped Boar Tusk", 6)) \
    X(SPIDER_SILK, junk("Spider Silk Strand", 8)) \
    X(DIM_CANDLE, junk("Dim Kobold Candle", 4)) \
    X(MURLOC_SCALE, junk("Slimy Murloc Scale", 9)) \
    X(GNOLL_PELT, junk("Mangy Gnoll Pelt", 12)) \
    X(LINEN_CLOTH, junk("Linen Cloth", 5, 20)) \
    X(WOOL_CLOTH, junk("Wool Cloth", 15, 20)) \
    X(RUSTED_GEAR, junk("Rusted Gear", 20)) \
    X(COPPER_BOLTS, junk("Bag of Copper Bolts", 30)) \
    /* Corina Steele, Goldshire */ \
    X(GLADIUS, weapon("Gladius", C, SWORD, 5, 24)) \
    X(HATCHET, weapon("Hatchet", C, AXE, 5, 22)) \
    X(CUDGEL, weapon("Cudgel", C, MACE, 5, 23)) \
    X(DIRK, weapon("Dirk", C, DAGGER, 5, 16)) \
    X(WALKING_STAFF, weapon("Walking Staff", C, STAFF, 5, 30)) \
    X(CLAYMORE, weapon("Claymore", C, TWO_HANDED, 6, 33)) \
    X(HUNTING_BOW, weapon("Hunting Bow", C, BOW, 5, 26)) \
    X(WOODEN_BUCKLER, shield("Wooden Buckler", C, 5)) \
    /* Quartermaster Lewis, Sentinel Hill */ \
    X(BROADSWORD, weapon("Broadsword", C, SWORD, 12, 26)) \
    X(MORNING_STAR, weapon("Morning Star", C, MACE, 12, 27)) \
    X(STILETTO, weapon("Stiletto", C, DAGGER, 12, 17)) \
    X(GNARLED_STAFF, weapon("Gnarled Staff", C, STAFF, 12, 31)) \
    X(BATTLE_AXE, weapon("Battle Axe", C, TWO_HANDED, 12, 34)) \
    X(RECURVE_BOW, weapon("Heavy Recurve Bow", C, BOW, 12, 27)) \
    X(LIGHT_MUSKET, weapon("Light Musket", C, GUN, 12, 28)) \
    X(ROUND_SHIELD, shield("Large Round Shield", C, 12)) \
    /* Andrew Krighton, Goldshire */ \
    X(LINEN_HOOD, armor("Linen Hood", C, CLOTH, HEAD, 5)) \
    X(LINEN_ROBE, armor("Linen Robe", C, CLOTH, CHEST, 5)) \
    X(LINEN_GLOVES, armor("Linen Gloves", C, CLOTH, HANDS, 5)) \
    X(LINEN_PANTS, armor("Linen Pants", C, CLOTH, LEGS, 5)) \
    X(LINEN_BOOTS, armor("Linen Boots", C, CLOTH, FEET, 5)) \
    X(LEATHER_CAP, armor("Leather Cap", C, LEATHER, HEAD, 5)) \
    X(LEATHER_VEST, armor("Leather Vest", C, LEATHER, CHEST, 5)) \
    X(LEATHER_GLOVES, armor("Leather Gloves", C, LEATHER, HANDS, 5)) \
    X(LEATHER_PANTS, armor("Leather Pants", C, LEATHER, LEGS, 5)) \
    X(LEATHER_BOOTS, armor("Leather Boots", C, LEATHER, FEET, 5)) \
    X(CHAIN_COIF, armor("Chain Coif", C, MAIL, HEAD, 5)) \
    X(CHAIN_VEST, armor("Chain Vest", C, MAIL, CHEST, 5)) \
    X(CHAIN_GLOVES, armor("Chain Gloves", C, MAIL, HANDS, 5)) \
    X(CHAIN_LEGGINGS, armor("Chain Leggings", C, MAIL, LEGS, 5)) \
    X(CHAIN_BOOTS, armor("Chain Boots", C, MAIL, FEET, 5)) \
    /* Quartermaster Lewis, Sentinel Hill */ \
    X(WOOLEN_HOOD, armor("Woolen Hood", C, CLOTH, HEAD, 12)) \
    X(WOOLEN_ROBE, armor("Woolen Robe", C, CLOTH, CHEST, 12)) \
    X(WOOLEN_GLOVES, armor("Woolen Gloves", C, CLOTH, HANDS, 12)) \
    X(WOOLEN_PANTS, armor("Woolen Pants", C, CLOTH, LEGS, 12)) \
    X(WOOLEN_BOOTS, armor("Woolen Boots", C, CLOTH, FEET, 12)) \
    X(HARDENED_CAP, armor("Hardened Cap", C, LEATHER, HEAD, 12)) \
    X(HARDENED_VEST, armor("Hardened Vest", C, LEATHER, CHEST, 12)) \
    X(HARDENED_GLOVES, armor("Hardened Gloves", C, LEATHER, HANDS, 12)) \
    X(HARDENED_PANTS, armor("Hardened Pants", C, LEATHER, LEGS, 12)) \
    X(HARDENED_BOOTS, armor("Hardened Boots", C, LEATHER, FEET, 12)) \
    X(SCALE_COIF, armor("Scale Coif", C, MAIL, HEAD, 12)) \
    X(SCALE_VEST, armor("Scale Vest", C, MAIL, CHEST, 12)) \
    X(SCALE_GLOVES, armor("Scale Gloves", C, MAIL, HANDS, 12)) \
    X(SCALE_LEGGINGS, armor("Scale Leggings", C, MAIL, LEGS, 12)) \
    X(SCALE_BOOTS, armor("Scale Boots", C, MAIL, FEET, 12)) \
    /* quest rewards */ \
    X(NORTHSHIRE_MAIL_VEST, armor("Northshire Mail Vest", U, MAIL, CHEST, 4, 1, 0, 1, 0, 0)) \
    X(NORTHSHIRE_ROBE, armor("Northshire Robe", U, CLOTH, CHEST, 4, 0, 0, 0, 2, 0)) \
    X(NORTHSHIRE_TUNIC, armor("Northshire Tunic", U, LEATHER, CHEST, 4, 0, 2, 0, 0, 0)) \
    X(GUARD_LEGGINGS, armor("Guard's Leggings", U, MAIL, LEGS, 7, 1, 0, 2, 0, 0)) \
    X(WOVEN_LEGGINGS, armor("Woven Leggings", U, CLOTH, LEGS, 7, 0, 0, 1, 2, 0)) \
    X(FORESTER_PANTS, armor("Forester's Pants", U, LEATHER, LEGS, 7, 0, 2, 1, 0, 0)) \
    X(PIG_IRON_GAUNTLETS, armor("Pig Iron Gauntlets", U, MAIL, HANDS, 10, 2, 0, 1, 0, 0)) \
    X(FARMHAND_GLOVES, armor("Farmhand's Gloves", U, CLOTH, HANDS, 10, 0, 0, 1, 2, 1)) \
    X(BOARHIDE_GLOVES, armor("Boarhide Gloves", U, LEATHER, HANDS, 10, 0, 2, 1, 0, 0)) \
    X(RIVERPAW_CLEAVER, weapon("Riverpaw Cleaver", U, AXE, 9, 25, 1, 0, 1, 0, 0)) \
    X(TRIBAL_STAFF, weapon("Tribal Staff", U, STAFF, 9, 30, 0, 0, 1, 3, 1)) \
    X(GNOLL_HUNTING_BOW, weapon("Gnoll Hunting Bow", U, BOW, 9, 27, 0, 2, 0, 0, 0)) \
    X(MARSHALS_GREATSWORD, weapon("Marshal's Greatsword", U, TWO_HANDED, 11, 33, 3, 0, 2, 0, 0)) \
    X(STAFF_OF_ELWYNN, weapon("Staff of Elwynn", U, STAFF, 11, 30, 0, 0, 1, 4, 2)) \
    X(ELWYNN_LONGBOW, weapon("Elwynn Longbow", U, BOW, 11, 28, 0, 3, 1, 0, 0)) \
    X(HARVESTER_BOOTS, armor("Harvester's Boots", U, MAIL, FEET, 12, 2, 0, 2, 0, 0)) \
    X(HARVESTER_SLIPPERS, armor("Harvester's Slippers", U, CLOTH, FEET, 12, 0, 0, 1, 3, 1)) \
    X(HARVESTER_MOCCASINS, armor("Harvest Moccasins", U, LEATHER, FEET, 12, 0, 3, 1, 0, 0)) \
    X(MILITIA_CHAINMAIL, armor("Militia Chainmail", U, MAIL, CHEST, 13, 3, 0, 3, 0, 0)) \
    X(MILITIA_ROBE, armor("Militia Robe", U, CLOTH, CHEST, 13, 0, 0, 2, 4, 2)) \
    X(MILITIA_JERKIN, armor("Militia Jerkin", U, LEATHER, CHEST, 13, 0, 4, 2, 0, 0)) \
    X(MILITIA_HELM, armor("Militia Helm", U, MAIL, HEAD, 13, 2, 0, 3, 0, 0)) \
    X(SEER_HOOD, armor("Seer's Hood", U, CLOTH, HEAD, 13, 0, 0, 1, 3, 2)) \
    X(SCOUT_HOOD, armor("Scout's Hood", U, LEATHER, HEAD, 13, 0, 3, 2, 0, 0)) \
    X(WESTFALL_WARHAMMER, weapon("Westfall Warhammer", U, TWO_HANDED, 15, 35, 4, 0, 3, 0, 0)) \
    X(FURLBROW_STAFF, weapon("Furlbrow's Staff", U, STAFF, 15, 31, 0, 0, 2, 5, 3)) \
    X(WESTFALL_SHORTBOW, weapon("Westfall Shortbow", U, BOW, 15, 27, 0, 4, 1, 0, 0)) \
    X(MILITIA_LEGPLATES, armor("Militia Legplates", U, MAIL, LEGS, 17, 4, 0, 4, 0, 0)) \
    X(SILK_TROUSERS, armor("Silk Trousers", U, CLOTH, LEGS, 17, 0, 0, 2, 5, 3)) \
    X(DEFIAS_LEGGINGS, armor("Defias Leggings", U, LEATHER, LEGS, 17, 0, 5, 2, 0, 0)) \
    X(SHREDDER_GAUNTLETS, armor("Shredder Gauntlets", U, MAIL, HANDS, 18, 4, 0, 3, 0, 0)) \
    X(TINKER_GLOVES, armor("Tinker's Gloves", U, CLOTH, HANDS, 18, 0, 0, 2, 4, 3)) \
    X(MECHANIC_GLOVES, armor("Mechanic's Gloves", U, LEATHER, HANDS, 18, 0, 4, 3, 0, 0)) \
    X(CHAUSSES_OF_WESTFALL, armor("Chausses of Westfall", R, MAIL, LEGS, 20, 6, 0, 6, 0, 0)) \
    X(TUNIC_OF_WESTFALL, armor("Tunic of Westfall", R, LEATHER, CHEST, 20, 2, 7, 4, 0, 0)) \
    X(STAFF_OF_WESTFALL, weapon("Staff of Westfall", R, STAFF, 20, 32, 0, 0, 3, 8, 5)) \
    /* boss drops */ \
    X(BUZZER_BLADE, weapon("Buzzer Blade", R, DAGGER, 19, 17, 0, 3, 2, 0, 0)) \
    X(TASKMASTER_AXE, weapon("Taskmaster Axe", R, TWO_HANDED, 19, 35, 6, 0, 4, 0, 0)) \
    X(GOLD_PLATED_BUCKLER, shield("Gold-plated Buckler", R, 19, 2, 0, 3, 0, 0)) \
    X(CRUEL_BARB, weapon("Cruel Barb", R, SWORD, 20, 28, 5, 0, 3, 0, 0)) \
    X(SMITES_HAMMER, weapon("Smite's Mighty Hammer", R, TWO_HANDED, 20, 36, 8, 0, 5, 0, 0)) \
    X(CORSAIR_OVERSHIRT, armor("Corsair's Overshirt", R, CLOTH, CHEST, 20, 0, 0, 4, 8, 4)) \
    /* world drops */ \
    X(FOOTPAD_VEST, armor("Footpad's Vest", U, LEATHER, CHEST, 4, 0, 1, 1, 0, 0)) \
    X(GLIMMERING_GLOVES, armor("Glimmering Gloves", U, CLOTH, HANDS, 4, 0, 0, 0, 1, 1)) \
    X(BRONZE_LEGGINGS, armor("Rough Bronze Leggings", U, MAIL, LEGS, 5, 1, 0, 1, 0, 0)) \
    X(BANDIT_SHORTSWORD, weapon("Bandit's Shortsword", U, SWORD, 5, 23, 1, 0, 0, 0, 0)) \
    X(FOREST_PANTS, armor("Forest Leather Pants", U, LEATHER, LEGS, 9, 0, 2, 1, 0, 0)) \
    X(SPELLBINDER_BOOTS, armor("Spellbinder Boots", U, CLOTH, FEET, 9, 0, 0, 0, 2, 1)) \
    X(SCALEMAIL_GLOVES, armor("Scalemail Gloves", U, MAIL, HANDS, 9, 2, 0, 1, 0, 0)) \
    X(IRONWOOD_MACE, weapon("Ironwood Mace", U, MACE, 9, 26, 2, 0, 0, 0, 0)) \
    X(ASH_LONGBOW, weapon("Ash Longbow", U, BOW, 9, 27, 0, 2, 0, 0, 0)) \
    X(SCOUTING_BOOTS, armor("Scouting Boots", U, LEATHER, FEET, 13, 0, 2, 2, 0, 0)) \
    X(SILKEN_COWL, armor("Silken Cowl", U, CLOTH, HEAD, 13, 0, 0, 1, 3, 1)) \
    X(IRONCLAD_LEGGINGS, armor("Ironclad Leggings", U, MAIL, LEGS, 14, 3, 0, 2, 0, 0)) \
    X(FINE_LONGSWORD, weapon("Fine Longsword", U, SWORD, 13, 26, 2, 0, 1, 0, 0)) \
    X(HORNWOOD_BOW, weapon("Hornwood Recurve Bow", U, BOW, 14, 28, 0, 3, 0, 0, 0)) \
    X(MILITIA_SHIELD, shield("Militia Kite Shield", U, 14, 1, 0, 2, 0, 0)) \
    X(BLACKENED_LEGGINGS, armor("Blackened Leggings", U, LEATHER, LEGS, 17, 0, 3, 2, 0, 0)) \
    X(CINDERCLOTH_ROBE, armor("Cindercloth Robe", U, CLOTH, CHEST, 18, 0, 0, 2, 4, 2)) \
    X(POLISHED_BOOTS, armor("Polished Chain Boots", U, MAIL, FEET, 18, 3, 0, 3, 0, 0)) \
    X(MINERS_REVENGE, weapon("Miner's Revenge", U, TWO_HANDED, 18, 35, 4, 0, 2, 0, 0))

namespace gw
{

#define GW_ITEM_ID(id, definition) id,

enum class item_id : uint8_t
{
    GW_ITEM_LIST(GW_ITEM_ID)
    COUNT
};

#undef GW_ITEM_ID

}

#endif
