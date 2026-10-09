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
// reagent(name, sell price, stack): used up by abilities
// special(name, type): no price, no stack
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
    X(MINERS_REVENGE, weapon("Miner's Revenge", U, TWO_HANDED, 18, 35, 4, 0, 2, 0, 0)) \
    X(HEARTHSTONE, special("Hearthstone", HEARTHSTONE)) \
    /* Gunther Weller, Stormwind */ \
    X(LONGSWORD, weapon("Longsword", C, SWORD, 9, 25)) \
    X(WAR_AXE, weapon("War Axe", C, AXE, 9, 24)) \
    X(FLANGED_MACE, weapon("Flanged Mace", C, MACE, 9, 26)) \
    X(KRIS, weapon("Kris", C, DAGGER, 9, 17)) \
    X(QUARTERSTAFF, weapon("Quarterstaff", C, STAFF, 9, 30)) \
    X(GREATSWORD, weapon("Greatsword", C, TWO_HANDED, 9, 34)) \
    X(COMPOSITE_BOW, weapon("Composite Bow", C, BOW, 9, 27)) \
    X(FLINTLOCK, weapon("Flintlock Rifle", C, GUN, 9, 28)) \
    X(KITE_SHIELD, shield("Kite Shield", C, 9)) \
    /* Lina Stover, Stormwind */ \
    X(PADDED_HOOD, armor("Padded Hood", C, CLOTH, HEAD, 9)) \
    X(PADDED_ROBE, armor("Padded Robe", C, CLOTH, CHEST, 9)) \
    X(PADDED_GLOVES, armor("Padded Gloves", C, CLOTH, HANDS, 9)) \
    X(PADDED_PANTS, armor("Padded Pants", C, CLOTH, LEGS, 9)) \
    X(PADDED_BOOTS, armor("Padded Boots", C, CLOTH, FEET, 9)) \
    X(STUDDED_CAP, armor("Studded Cap", C, LEATHER, HEAD, 9)) \
    X(STUDDED_VEST, armor("Studded Vest", C, LEATHER, CHEST, 9)) \
    X(STUDDED_GLOVES, armor("Studded Gloves", C, LEATHER, HANDS, 9)) \
    X(STUDDED_PANTS, armor("Studded Pants", C, LEATHER, LEGS, 9)) \
    X(STUDDED_BOOTS, armor("Studded Boots", C, LEATHER, FEET, 9)) \
    X(RINGMAIL_COIF, armor("Ringmail Coif", C, MAIL, HEAD, 9)) \
    X(RINGMAIL_VEST, armor("Ringmail Vest", C, MAIL, CHEST, 9)) \
    X(RINGMAIL_GLOVES, armor("Ringmail Gloves", C, MAIL, HANDS, 9)) \
    X(RINGMAIL_LEGGINGS, armor("Ringmail Leggings", C, MAIL, LEGS, 9)) \
    X(RINGMAIL_BOOTS, armor("Ringmail Boots", C, MAIL, FEET, 9)) \
    /* Brann Bronzebeard's treasure hunt */ \
    X(TRAILBLAZER_BOOTS, armor("Trailblazer Boots", U, LEATHER, FEET, 10, 0, 3, 1, 0, 0)) \
    X(PATHFINDER_GREAVES, armor("Pathfinder Greaves", U, MAIL, FEET, 10, 2, 0, 2, 0, 0)) \
    X(WANDERER_SANDALS, armor("Wanderer's Sandals", U, CLOTH, FEET, 10, 0, 0, 1, 3, 1)) \
    /* The Stockade: quest rewards */ \
    X(RIOTGUARD_HELM, armor("Riotguard Helm", R, MAIL, HEAD, 20, 5, 0, 5, 0, 0)) \
    X(JAILERS_COWL, armor("Jailer's Cowl", R, CLOTH, HEAD, 20, 0, 0, 3, 7, 4)) \
    X(TURNKEY_CAP, armor("Turnkey's Cap", R, LEATHER, HEAD, 20, 0, 6, 4, 0, 0)) \
    X(LIONHEART_BLADE, weapon("Lionheart Blade", E, TWO_HANDED, 20, 36, 9, 0, 7, 0, 0)) \
    X(STAFF_OF_THE_LION, weapon("Staff of the Lion", E, STAFF, 20, 32, 0, 0, 5, 11, 6)) \
    X(LIONHEART_LONGBOW, weapon("Lionheart Longbow", E, BOW, 20, 28, 0, 8, 4, 0, 0)) \
    /* The Stockade: boss drops */ \
    X(LUCINE_LONGSWORD, weapon("Lucine Longsword", R, SWORD, 20, 27, 4, 0, 3, 0, 0)) \
    X(KNUCKLE_WRAPS, armor("Dirty Knuckle Wraps", R, LEATHER, HANDS, 20, 0, 5, 3, 0, 0)) \
    X(SHACKLED_MITTS, armor("Shackled Mitts", R, CLOTH, HANDS, 20, 0, 0, 3, 5, 3)) \
    X(DEEPFURY_SHIELD, shield("Deepfury Shield", R, 20, 3, 0, 5, 0, 0)) \
    X(EMBERWEAVE_ROBE, armor("Emberweave Robe", R, CLOTH, CHEST, 20, 0, 0, 4, 8, 4)) \
    X(DARK_IRON_RIFLE, weapon("Dark Iron Rifle", R, GUN, 20, 28, 0, 5, 2, 0, 0)) \
    X(THREDDS_DUSKBLADE, weapon("Thredd's Duskblade", R, SWORD, 20, 26, 5, 2, 4, 0, 0)) \
    X(SMOKEWEAVE_PANTS, armor("Smokeweave Pants", R, CLOTH, LEGS, 20, 0, 0, 4, 8, 5)) \
    X(SHADOWHIDE_BOOTS, armor("Shadowhide Boots", R, LEATHER, FEET, 20, 0, 7, 4, 0, 0)) \
    /* conjured by mages, a kind per rank */ \
    X(CONJURED_WATER, consumable("Conjured Water", DRINK, 1, 150, 0, 20)) \
    X(CONJURED_FRESH_WATER, consumable("Conjured Fresh Water", DRINK, 5, 400, 0, 20)) \
    X(CONJURED_PURE_WATER, consumable("Conjured Pure Water", DRINK, 15, 800, 0, 20)) \
    X(CONJURED_BROOK_WATER, consumable("Conjured Brook Water", DRINK, 25, 1300, 0, 20)) \
    X(CONJURED_ICE_WATER, consumable("Conjured Ice Water", DRINK, 35, 1900, 0, 20)) \
    X(CONJURED_CLEAR_WATER, consumable("Conjured Clear Water", DRINK, 45, 2600, 0, 20)) \
    X(CONJURED_SNOW_WATER, consumable("Conjured Snow Water", DRINK, 55, 3400, 0, 20)) \
    X(CONJURED_MUFFIN, consumable("Conjured Muffin", FOOD, 1, 60, 0, 20)) \
    X(CONJURED_BREAD, consumable("Conjured Bread", FOOD, 5, 200, 0, 20)) \
    X(CONJURED_RYE, consumable("Conjured Rye", FOOD, 15, 420, 0, 20)) \
    X(CONJURED_SOURDOUGH, consumable("Conjured Sourdough", FOOD, 25, 700, 0, 20)) \
    X(CONJURED_SWEET_ROLL, consumable("Conjured Sweet Roll", FOOD, 35, 1000, 0, 20)) \
    X(CONJURED_CROISSANT, consumable("Conjured Croissant", FOOD, 45, 1400, 0, 20)) \
    X(TELEPORTATION_RUNE, reagent("Teleportation Rune", 250, 20)) \
    /* Redridge: quest rewards */ \
    X(LAKESHIRE_GAUNTLETS, armor("Lakeshire Gauntlets", U, MAIL, HANDS, 15, 3, 0, 2, 0, 0)) \
    X(CANYON_WRAPS, armor("Canyon Wraps", U, CLOTH, HANDS, 15, 0, 0, 1, 3, 2)) \
    X(MONGREL_HIDE_GLOVES, armor("Mongrel Hide Gloves", U, LEATHER, HANDS, 15, 0, 3, 2, 0, 0)) \
    X(DOCKHAND_BOOTS, armor("Dockhand's Boots", U, MAIL, FEET, 15, 2, 0, 3, 0, 0)) \
    X(SHORELINE_SANDALS, armor("Shoreline Sandals", U, CLOTH, FEET, 15, 0, 0, 1, 3, 2)) \
    X(POACHERS_BOOTS, armor("Poacher's Boots", U, LEATHER, FEET, 15, 0, 3, 2, 0, 0)) \
    X(LUCKY_FISHING_HAT, armor("Lucky Fishing Hat", U, CLOTH, HEAD, 15, 0, 0, 2, 1, 3)) \
    X(LAKESHIRE_LEGGUARDS, armor("Lakeshire Legguards", U, MAIL, LEGS, 16, 3, 0, 3, 0, 0)) \
    X(MAGISTRATE_TROUSERS, armor("Magistrate's Trousers", U, CLOTH, LEGS, 16, 0, 0, 1, 4, 2)) \
    X(SHADOWHIDE_LEGGINGS, armor("Shadowhide Leggings", U, LEATHER, LEGS, 16, 0, 4, 2, 0, 0)) \
    X(LAKESHIRE_LONGSWORD, weapon("Lakeshire Longsword", U, SWORD, 16, 27, 3, 0, 1, 0, 0)) \
    X(EVERSTILL_STAFF, weapon("Everstill Staff", U, STAFF, 16, 31, 0, 0, 1, 4, 2)) \
    X(REDRIDGE_RECURVE, weapon("Redridge Recurve", U, BOW, 16, 28, 0, 3, 1, 0, 0)) \
    X(LAKESHIRE_CHAINMAIL, armor("Lakeshire Chainmail", U, MAIL, CHEST, 17, 4, 0, 3, 0, 0)) \
    X(LAKESHIRE_ROBE, armor("Lakeshire Robe", U, CLOTH, CHEST, 17, 0, 0, 1, 5, 3)) \
    X(GORETUSK_HIDE_VEST, armor("Goretusk Hide Vest", U, LEATHER, CHEST, 17, 0, 4, 3, 0, 0)) \
    X(IRONWORKER_HELM, armor("Ironworker's Helm", U, MAIL, HEAD, 17, 3, 0, 3, 0, 0)) \
    X(SURVEYOR_HOOD, armor("Surveyor's Hood", U, CLOTH, HEAD, 17, 0, 0, 1, 4, 2)) \
    X(RIGGER_CAP, armor("Rigger's Cap", U, LEATHER, HEAD, 17, 0, 4, 2, 0, 0)) \
    X(TUSK_CLEAVER, weapon("Tusk Cleaver", U, AXE, 18, 26, 4, 0, 1, 0, 0)) \
    X(OSGOOD_WALKING_STAFF, weapon("Osgood's Walking Staff", U, STAFF, 18, 31, 0, 0, 2, 5, 3)) \
    X(PIGSTICKER_BOW, weapon("Pigsticker Bow", U, BOW, 18, 28, 0, 4, 1, 0, 0)) \
    X(MARRIS_SABATONS, armor("Marris's Sabatons", U, MAIL, FEET, 18, 3, 0, 3, 0, 0)) \
    X(LAKESHIRE_SLIPPERS, armor("Lakeshire Slippers", U, CLOTH, FEET, 18, 0, 0, 1, 4, 3)) \
    X(OUTRUNNER_BOOTS, armor("Outrunner Boots", U, LEATHER, FEET, 18, 0, 4, 2, 0, 0)) \
    X(STONEWATCH_GAUNTLETS, armor("Stonewatch Gauntlets", U, MAIL, HANDS, 19, 4, 0, 3, 0, 0)) \
    X(SUMMONER_GLOVES, armor("Summoner's Gloves", U, CLOTH, HANDS, 19, 0, 0, 2, 5, 3)) \
    X(RENEGADE_GRIPS, armor("Renegade Grips", U, LEATHER, HANDS, 19, 0, 5, 2, 0, 0)) \
    X(REDRIDGE_WARBLADE, weapon("Redridge Warblade", R, TWO_HANDED, 20, 35, 7, 0, 5, 0, 0)) \
    X(STAFF_OF_LAKESHIRE, weapon("Staff of Lakeshire", R, STAFF, 20, 32, 0, 0, 3, 8, 5)) \
    X(EVERSTILL_LONGBOW, weapon("Everstill Longbow", R, BOW, 20, 28, 0, 7, 3, 0, 0)) \
    /* Redridge: drops of Ribchaser and Gath'Ilzogg */ \
    X(RIBCHASERS_CLEAVER, weapon("Ribchaser's Cleaver", R, AXE, 18, 25, 4, 1, 3, 0, 0)) \
    X(GNOLLBONE_STAFF, weapon("Gnollbone Staff", R, STAFF, 18, 31, 0, 0, 3, 7, 4)) \
    X(RIBCHASERS_LONGBOW, weapon("Ribchaser's Longbow", R, BOW, 18, 28, 0, 6, 2, 0, 0)) \
    X(GATHS_WARMAUL, weapon("Gath'Ilzogg's Warmaul", R, TWO_HANDED, 20, 36, 7, 0, 6, 0, 0)) \
    X(SHADOWCASTER_ROBE, armor("Shadowcaster Robe", R, CLOTH, CHEST, 20, 0, 0, 4, 8, 4)) \
    X(BLACKROCK_HUNTING_BOW, weapon("Blackrock Hunting Bow", R, BOW, 20, 28, 0, 7, 3, 0, 0)) \
    /* Duskwood: supplies and what its creatures carry */ \
    X(WILD_HOG_SHANK, consumable("Wild Hog Shank", FOOD, 25, 700, 180, 20)) \
    X(SWEET_NECTAR, consumable("Sweet Nectar", DRINK, 25, 1300, 180, 20)) \
    X(GREATER_HEALING_POTION, consumable("Greater Healing Potion", POTION, 21, 560, 250, 5)) \
    X(SILK_CLOTH, junk("Silk Cloth", 25, 20)) \
    X(BONE_FRAGMENTS, junk("Bone Fragments", 30, 20)) \
    X(PUTRID_CLAW, junk("Putrid Claw", 40, 20)) \
    X(WORGEN_FANG, junk("Worgen Fang", 45, 20)) \
    X(OGRE_TOOTH, junk("Ogre Tooth", 50, 20)) \
    X(DIRE_WOLF_PELT, junk("Dire Wolf Pelt", 35, 20)) \
    /* Duskwood: quest rewards */ \
    X(NIGHT_WATCH_GAUNTLETS, armor("Night Watch Gauntlets", U, MAIL, HANDS, 21, 4, 0, 3, 0, 0)) \
    X(WATCHERS_HANDWRAPS, armor("Watcher's Handwraps", U, CLOTH, HANDS, 21, 0, 0, 2, 4, 3)) \
    X(DUSKWOOD_GRIPS, armor("Duskwood Grips", U, LEATHER, HANDS, 21, 0, 4, 3, 0, 0)) \
    X(WOLFHEAD_HELM, armor("Wolfhead Helm", U, MAIL, HEAD, 21, 4, 0, 4, 0, 0)) \
    X(DUSKWOOD_COWL, armor("Duskwood Cowl", U, CLOTH, HEAD, 21, 0, 0, 2, 5, 3)) \
    X(DIRE_PELT_CAP, armor("Dire Pelt Cap", U, LEATHER, HEAD, 21, 0, 5, 3, 0, 0)) \
    X(RAVEN_HILL_GREAVES, armor("Raven Hill Greaves", U, MAIL, FEET, 22, 4, 0, 4, 0, 0)) \
    X(GRAVEDIGGER_SLIPPERS, armor("Gravedigger's Slippers", U, CLOTH, FEET, 22, 0, 0, 2, 5, 3)) \
    X(CEMETERY_BOOTS, armor("Cemetery Boots", U, LEATHER, FEET, 22, 0, 5, 3, 0, 0)) \
    X(BRIGHTWOOD_LEGPLATES, armor("Brightwood Legplates", U, MAIL, LEGS, 22, 5, 0, 4, 0, 0)) \
    X(WEAVERS_LEGGINGS, armor("Weaver's Leggings", U, CLOTH, LEGS, 22, 0, 0, 3, 6, 3)) \
    X(NIGHTBANE_TROUSERS, armor("Nightbane Trousers", U, LEATHER, LEGS, 22, 0, 5, 4, 0, 0)) \
    X(OGRE_CLEAVER, weapon("Splinter Fist Cleaver", U, AXE, 23, 27, 5, 0, 3, 0, 0)) \
    X(MOUND_STAFF, weapon("Vul'Gol Staff", U, STAFF, 23, 32, 0, 0, 3, 6, 4)) \
    X(SPLINTER_BOW, weapon("Splinter Bow", U, BOW, 23, 28, 0, 5, 3, 0, 0)) \
    X(MISTMANTLE_BLADE, weapon("Mistmantle Blade", U, SWORD, 24, 27, 5, 0, 4, 0, 0)) \
    X(STAFF_OF_THE_MISTS, weapon("Staff of the Mists", U, STAFF, 24, 32, 0, 0, 3, 7, 4)) \
    X(MISTMANTLE_LONGBOW, weapon("Mistmantle Longbow", U, BOW, 24, 28, 0, 6, 3, 0, 0)) \
    X(LADIMORE_HAUBERK, armor("Ladimore Hauberk", U, MAIL, CHEST, 24, 6, 0, 5, 0, 0)) \
    X(SEXTONS_ROBE, armor("Sexton's Robe", U, CLOTH, CHEST, 24, 0, 0, 4, 7, 4)) \
    X(RAVEN_HILL_JERKIN, armor("Raven Hill Jerkin", U, LEATHER, CHEST, 24, 0, 6, 5, 0, 0)) \
    X(CRYPTBREAKER, weapon("Cryptbreaker", R, TWO_HANDED, 25, 36, 9, 0, 6, 0, 0)) \
    X(STAFF_OF_VON_INDI, weapon("Staff of Von'Indi", R, STAFF, 25, 32, 0, 0, 4, 10, 6)) \
    X(GRAVEWATCH_LONGBOW, weapon("Gravewatch Longbow", R, BOW, 25, 28, 0, 8, 4, 0, 0)) \
    X(NIGHT_WATCH_SHORTSWORD, weapon("Night Watch Shortsword", R, SWORD, 25, 26, 6, 1, 4, 0, 0)) \
    X(DARKSHIRE_ROBE, armor("Darkshire Robe", R, CLOTH, CHEST, 25, 0, 0, 5, 9, 5)) \
    X(GLOOMWOOD_LONGBOW, weapon("Gloomwood Longbow", R, BOW, 25, 28, 0, 8, 4, 0, 0)) \
    /* Shadowfang Keep: quest rewards */ \
    X(SHADOWFANG_GAUNTLETS, armor("Shadowfang Gauntlets", R, MAIL, HANDS, 23, 5, 0, 4, 0, 0)) \
    X(DALARAN_WRAPS, armor("Dalaran Wraps", R, CLOTH, HANDS, 23, 0, 0, 3, 6, 3)) \
    X(SCOUTS_GLOVES, armor("Scout's Gloves", R, LEATHER, HANDS, 23, 0, 6, 3, 0, 0)) \
    X(SPRINGVALES_SABATONS, armor("Springvale's Sabatons", R, MAIL, FEET, 24, 6, 0, 4, 0, 0)) \
    X(CHAPEL_SANDALS, armor("Chapel Sandals", R, CLOTH, FEET, 24, 0, 0, 3, 7, 4)) \
    X(BLINDWATCHER_BOOTS, armor("Blindwatcher Boots", R, LEATHER, FEET, 24, 0, 7, 3, 0, 0)) \
    X(MOONSTEEL_GREATSWORD, weapon("Moonsteel Greatsword", E, TWO_HANDED, 25, 36, 11, 0, 8, 0, 0)) \
    X(STAFF_OF_DALARAN, weapon("Staff of Dalaran", E, STAFF, 25, 32, 0, 0, 6, 13, 7)) \
    X(VALDANS_LONGBOW, weapon("Valdan's Longbow", E, BOW, 25, 28, 0, 10, 5, 0, 0)) \
    /* Shadowfang Keep: boss drops */ \
    X(WOLFGUARD_GAUNTLETS, armor("Wolfguard Gauntlets", R, MAIL, HANDS, 22, 5, 0, 3, 0, 0)) \
    X(SOUL_DRAIN_WRAPS, armor("Soul-Drain Wraps", R, CLOTH, HANDS, 22, 0, 0, 3, 5, 3)) \
    X(RETHILGORES_GRIPS, armor("Rethilgore's Grips", R, LEATHER, HANDS, 22, 0, 5, 3, 0, 0)) \
    X(BUTCHERS_SLICER, weapon("Butcher's Slicer", R, SWORD, 22, 26, 5, 1, 3, 0, 0)) \
    X(BUTCHERS_APRON, armor("Butcher's Apron", R, CLOTH, CHEST, 22, 0, 0, 4, 8, 4)) \
    X(RAZORCLAW_LEGGINGS, armor("Razorclaw Leggings", R, LEATHER, LEGS, 22, 0, 7, 4, 0, 0)) \
    X(SILVERLAINES_HELM, armor("Silverlaine's Helm", R, MAIL, HEAD, 23, 6, 0, 4, 0, 0)) \
    X(BARONS_CIRCLET, armor("Baron's Circlet", R, CLOTH, HEAD, 23, 0, 0, 4, 8, 4)) \
    X(MOONRAGE_HOOD, armor("Moonrage Hood", R, LEATHER, HEAD, 23, 0, 7, 4, 0, 0)) \
    X(COMMANDERS_CREST, shield("Commander's Crest", R, 23, 4, 0, 6, 0, 0)) \
    X(CHAPLAINS_VESTMENTS, armor("Chaplain's Vestments", R, CLOTH, CHEST, 23, 0, 0, 5, 9, 5)) \
    X(WOLFSKIN_JERKIN, armor("Wolfskin Jerkin", R, LEATHER, CHEST, 23, 0, 8, 5, 0, 0)) \
    X(BLINDWATCHER_GREATAXE, weapon("Blindwatcher Greataxe", R, TWO_HANDED, 24, 36, 9, 0, 6, 0, 0)) \
    X(ODOS_LEY_STAFF, weapon("Odo's Ley Staff", R, STAFF, 24, 32, 0, 0, 4, 10, 6)) \
    X(BLINDSIGHT_BOW, weapon("Blindsight Bow", R, BOW, 24, 28, 0, 8, 4, 0, 0)) \
    X(SHADOWFANG, weapon("Shadowfang", R, SWORD, 25, 26, 7, 2, 5, 0, 0)) \
    X(ROBE_OF_ARUGAL, armor("Robe of Arugal", R, CLOTH, CHEST, 25, 0, 0, 6, 11, 6)) \
    X(WORGEN_HIDE_LEGGINGS, armor("Worgen Hide Leggings", R, LEATHER, LEGS, 25, 0, 9, 6, 0, 0)) \
    /* World drops, levels 21 to 30 */ \
    X(SCALED_LEATHER_HEADBAND, armor("Scaled Leather Headband", U, LEATHER, HEAD, 22, 0, 5, 3, 0, 0)) \
    X(GREENWEAVE_ROBE, armor("Greenweave Robe", U, CLOTH, CHEST, 23, 0, 0, 3, 6, 4)) \
    X(DEFENDER_GAUNTLETS, armor("Defender Gauntlets", U, MAIL, HANDS, 22, 4, 0, 3, 0, 0)) \
    X(KNIGHTS_LONGSWORD, weapon("Knight's Longsword", U, SWORD, 24, 26, 5, 0, 3, 0, 0)) \
    X(EMBERSTONE_STAFF, weapon("Emberstone Staff", U, STAFF, 23, 32, 0, 0, 3, 6, 4)) \
    X(OUTRIDERS_BOW, weapon("Outrider's Bow", U, BOW, 24, 28, 0, 5, 3, 0, 0)) \
    X(BOGWALKER_BOOTS, armor("Bogwalker Boots", U, LEATHER, FEET, 27, 0, 6, 3, 0, 0)) \
    X(FENPLATE_LEGGINGS, armor("Fenplate Leggings", U, MAIL, LEGS, 27, 6, 0, 4, 0, 0)) \
    X(MOONGLOW_HOOD, armor("Moonglow Hood", U, CLOTH, HEAD, 27, 0, 0, 3, 7, 4)) \
    X(KNIGHTLY_GREATSWORD, weapon("Knightly Greatsword", U, TWO_HANDED, 28, 36, 8, 0, 5, 0, 0)) \
    X(SAGES_STAFF, weapon("Sage's Staff", U, STAFF, 28, 32, 0, 0, 4, 8, 5)) \
    X(HAWKEYE_BOW, weapon("Hawkeye Bow", U, BOW, 28, 28, 0, 7, 3, 0, 0)) \
    X(BULWARK_SHIELD, shield("Bulwark Shield", U, 27, 3, 0, 5, 0, 0)) \
    /* Ironforge and the Wetlands: supplies and what their creatures carry */ \
    X(DWARVEN_MILD, consumable("Dwarven Mild", FOOD, 25, 700, 180, 20)) \
    X(MOONBERRY_JUICE, consumable("Moonberry Juice", DRINK, 25, 1300, 180, 20)) \
    X(MAGEWEAVE_CLOTH, junk("Mageweave Cloth", 60, 20)) \
    X(CROCOLISK_SCALE, junk("Crocolisk Scale", 55, 20)) \
    X(RAPTOR_TALON, junk("Raptor Talon", 55, 20)) \
    X(NAGA_SCALE, junk("Naga Scale", 65, 20)) \
    X(GREASY_COG, junk("Greasy Cog", 60, 20)) \
    X(GLOWING_SLUDGE, junk("Glowing Sludge", 50, 20)) \
    X(TROGG_STONE_TOOTH, junk("Trogg Stone Tooth", 55, 20)) \
    /* Bruuk Barleybeard, Ironforge */ \
    X(DWARVEN_BROADSWORD, weapon("Dwarven Broadsword", C, SWORD, 24, 26)) \
    X(DWARVEN_WAR_AXE, weapon("Dwarven War Axe", C, AXE, 24, 25)) \
    X(IRONFORGE_WARHAMMER, weapon("Ironforge Warhammer", C, MACE, 24, 27)) \
    X(OAKEN_WAR_STAFF, weapon("Oaken War Staff", C, STAFF, 24, 31)) \
    X(DWARVEN_GREATAXE, weapon("Dwarven Greataxe", C, TWO_HANDED, 24, 35)) \
    X(HEAVY_RECURVE_BOW, weapon("Heavy Recurve Bow", C, BOW, 24, 28)) \
    X(DWARVEN_HAND_CANNON, weapon("Dwarven Hand Cannon", C, GUN, 24, 29)) \
    X(IRONFORGE_TOWER_SHIELD, shield("Ironforge Tower Shield", C, 24)) \
    X(BANDED_HELM, armor("Banded Helm", C, MAIL, HEAD, 24)) \
    X(BANDED_HAUBERK, armor("Banded Hauberk", C, MAIL, CHEST, 24)) \
    X(BANDED_GAUNTLETS, armor("Banded Gauntlets", C, MAIL, HANDS, 24)) \
    X(BANDED_LEGGINGS, armor("Banded Leggings", C, MAIL, LEGS, 24)) \
    X(BANDED_BOOTS, armor("Banded Boots", C, MAIL, FEET, 24)) \
    X(THICK_LEATHER_CAP, armor("Thick Leather Cap", C, LEATHER, HEAD, 24)) \
    X(THICK_LEATHER_VEST, armor("Thick Leather Vest", C, LEATHER, CHEST, 24)) \
    X(THICK_LEATHER_GLOVES, armor("Thick Leather Gloves", C, LEATHER, HANDS, 24)) \
    X(THICK_LEATHER_PANTS, armor("Thick Leather Pants", C, LEATHER, LEGS, 24)) \
    X(THICK_LEATHER_BOOTS, armor("Thick Leather Boots", C, LEATHER, FEET, 24)) \
    X(MAGEWEAVE_HOOD, armor("Mageweave Hood", C, CLOTH, HEAD, 24)) \
    X(MAGEWEAVE_ROBE, armor("Mageweave Robe", C, CLOTH, CHEST, 24)) \
    X(MAGEWEAVE_GLOVES, armor("Mageweave Gloves", C, CLOTH, HANDS, 24)) \
    X(MAGEWEAVE_PANTS, armor("Mageweave Pants", C, CLOTH, LEGS, 24)) \
    X(MAGEWEAVE_BOOTS, armor("Mageweave Boots", C, CLOTH, FEET, 24)) \
    /* The Wetlands: quest rewards */ \
    X(CROCSCALE_BOOTS, armor("Crocscale Boots", U, MAIL, FEET, 25, 4, 0, 4, 0, 0)) \
    X(MARSHWALKER_SANDALS, armor("Marshwalker Sandals", U, CLOTH, FEET, 25, 0, 0, 2, 5, 3)) \
    X(CROCOLISK_HIDE_BOOTS, armor("Crocolisk Hide Boots", U, LEATHER, FEET, 25, 0, 5, 3, 0, 0)) \
    X(RAPTORSCALE_GAUNTLETS, armor("Raptorscale Gauntlets", U, MAIL, HANDS, 25, 4, 0, 4, 0, 0)) \
    X(SCREECHER_WRAPS, armor("Screecher Wraps", U, CLOTH, HANDS, 25, 0, 0, 2, 5, 3)) \
    X(RAPTOR_HIDE_GLOVES, armor("Raptor Hide Gloves", U, LEATHER, HANDS, 25, 0, 5, 3, 0, 0)) \
    X(EXCAVATORS_HELM, armor("Excavator's Helm", U, MAIL, HEAD, 26, 5, 0, 4, 0, 0)) \
    X(ARCHAEOLOGISTS_HOOD, armor("Archaeologist's Hood", U, CLOTH, HEAD, 26, 0, 0, 3, 6, 3)) \
    X(PROSPECTORS_CAP, armor("Prospector's Cap", U, LEATHER, HEAD, 26, 0, 6, 3, 0, 0)) \
    X(THANDOL_LEGPLATES, armor("Thandol Legplates", U, MAIL, LEGS, 26, 6, 0, 4, 0, 0)) \
    X(DUN_MODR_LEGGINGS, armor("Dun Modr Leggings", U, CLOTH, LEGS, 26, 0, 0, 3, 6, 4)) \
    X(MOUNTAINEERS_BREECHES, armor("Mountaineer's Breeches", U, LEATHER, LEGS, 26, 0, 6, 4, 0, 0)) \
    X(DRAGONMAW_WAR_AXE, weapon("Dragonmaw War Axe", U, AXE, 27, 26, 6, 0, 4, 0, 0)) \
    X(SHADOWWARDER_STAFF, weapon("Shadowwarder Staff", U, STAFF, 27, 32, 0, 0, 4, 8, 4)) \
    X(ANGERFANG_BOW, weapon("Angerfang Bow", U, BOW, 27, 28, 0, 6, 4, 0, 0)) \
    X(STOUTFISTS_CHAINMAIL, armor("Stoutfist's Chainmail", R, MAIL, CHEST, 28, 8, 0, 6, 0, 0)) \
    X(HARBOR_MAGES_ROBE, armor("Harbor Mage's Robe", R, CLOTH, CHEST, 28, 0, 0, 5, 10, 6)) \
    X(WETLANDS_JERKIN, armor("Wetlands Jerkin", R, LEATHER, CHEST, 28, 0, 9, 5, 0, 0)) \
    X(SARLTOOTHS_CLEAVER, weapon("Sarltooth's Cleaver", R, AXE, 28, 26, 7, 1, 5, 0, 0)) \
    X(IRONBRAID_STAFF, weapon("Ironbraid Staff", R, STAFF, 28, 32, 0, 0, 4, 10, 6)) \
    X(ORMERS_LONGBOW, weapon("Ormer's Longbow", R, BOW, 28, 28, 0, 9, 4, 0, 0)) \
    X(NEKROSHS_MAUL, weapon("Nek'rosh's Maul", R, TWO_HANDED, 29, 36, 11, 0, 7, 0, 0)) \
    X(STAFF_OF_GRIM_BATOL, weapon("Staff of Grim Batol", R, STAFF, 29, 32, 0, 0, 5, 11, 7)) \
    X(DRAGONMAW_LONGBOW, weapon("Dragonmaw Longbow", R, BOW, 29, 28, 0, 10, 5, 0, 0)) \
    /* Blackfathom Deeps: quest rewards */ \
    X(GERRIGS_GAUNTLETS, armor("Gerrig's Gauntlets", R, MAIL, HANDS, 26, 6, 0, 4, 0, 0)) \
    X(LORGALIS_WRAPS, armor("Lorgalis Wraps", R, CLOTH, HANDS, 26, 0, 0, 3, 7, 4)) \
    X(BONEGRIP_GLOVES, armor("Bonegrip Gloves", R, LEATHER, HANDS, 26, 0, 7, 3, 0, 0)) \
    X(ARGENT_SABATONS, armor("Argent Sabatons", R, MAIL, FEET, 26, 6, 0, 4, 0, 0)) \
    X(DAWNWATCHER_SANDALS, armor("Dawnwatcher Sandals", R, CLOTH, FEET, 26, 0, 0, 3, 7, 4)) \
    X(TWILIGHT_TREADS, armor("Twilight Treads", R, LEATHER, FEET, 26, 0, 7, 3, 0, 0)) \
    X(TIDEWALKER_LEGPLATES, armor("Tidewalker Legplates", R, MAIL, LEGS, 26, 7, 0, 5, 0, 0)) \
    X(TIDECALLER_LEGGINGS, armor("Tidecaller Leggings", R, CLOTH, LEGS, 26, 0, 0, 4, 8, 5)) \
    X(SNAPJAW_HIDE_PANTS, armor("Snapjaw Hide Pants", R, LEATHER, LEGS, 26, 0, 8, 4, 0, 0)) \
    X(ARGENT_GUARDS_BLADE, weapon("Argent Guard's Blade", R, SWORD, 27, 26, 7, 1, 4, 0, 0)) \
    X(DAWNWATCHER_STAFF, weapon("Dawnwatcher Staff", R, STAFF, 27, 32, 0, 0, 4, 10, 6)) \
    X(STRANDWALKER_BOW, weapon("Strandwalker Bow", R, BOW, 27, 28, 0, 9, 4, 0, 0)) \
    X(FATHOM_HAUBERK, armor("Fathom Hauberk", R, MAIL, CHEST, 28, 9, 0, 6, 0, 0)) \
    X(ROBE_OF_THE_MOONSHRINE, armor("Robe of the Moonshrine", R, CLOTH, CHEST, 28, 0, 0, 6, 11, 6)) \
    X(DEEPWATER_JERKIN, armor("Deepwater Jerkin", R, LEATHER, CHEST, 28, 0, 10, 6, 0, 0)) \
    /* Blackfathom Deeps: boss drops */ \
    X(TURTLE_SHELL_SHIELD, shield("Turtle Shell Shield", R, 24, 4, 0, 6, 0, 0)) \
    X(GHAMOO_RA_WRAPS, armor("Ghamoo-ra's Wraps", R, CLOTH, HANDS, 24, 0, 0, 3, 6, 4)) \
    X(SNAPJAW_GLOVES, armor("Snapjaw Gloves", R, LEATHER, HANDS, 24, 0, 6, 3, 0, 0)) \
    X(STRIKE_OF_THE_HYDRA, weapon("Strike of the Hydra", R, SWORD, 25, 26, 6, 1, 4, 0, 0)) \
    X(ROBE_OF_THE_DEEPS, armor("Robe of the Deeps", R, CLOTH, CHEST, 25, 0, 0, 5, 9, 5)) \
    X(NAGA_SCALE_JERKIN, armor("Naga Scale Jerkin", R, LEATHER, CHEST, 25, 0, 8, 5, 0, 0)) \
    X(MURKBLOOD_HELM, armor("Murkblood Helm", R, MAIL, HEAD, 25, 6, 0, 5, 0, 0)) \
    X(GELIHASTS_HOOD, armor("Gelihast's Hood", R, CLOTH, HEAD, 25, 0, 0, 4, 8, 5)) \
    X(FISHSCALE_CAP, armor("Fishscale Cap", R, LEATHER, HEAD, 25, 0, 8, 4, 0, 0)) \
    X(TWILIGHT_CLEAVER, weapon("Twilight Cleaver", R, AXE, 26, 26, 7, 1, 4, 0, 0)) \
    X(ROD_OF_THE_SLEEPWALKER, weapon("Rod of the Sleepwalker", R, STAFF, 26, 32, 0, 0, 4, 10, 6)) \
    X(TWILIGHT_LONGBOW, weapon("Twilight Longbow", R, BOW, 26, 28, 0, 9, 4, 0, 0)) \
    X(AKU_MAIS_FANG, weapon("Aku'mai's Fang", R, TWO_HANDED, 27, 36, 10, 0, 7, 0, 0)) \
    X(STAFF_OF_THE_DEEP_MOTHER, weapon("Staff of the Deep Mother", R, STAFF, 27, 32, 0, 0, 5, 11, 6)) \
    X(ABYSSAL_LONGBOW, weapon("Abyssal Longbow", R, BOW, 27, 28, 0, 10, 4, 0, 0)) \
    /* Gnomeregan: quest rewards */ \
    X(LEAD_LINED_HELM, armor("Lead-Lined Helm", U, MAIL, HEAD, 27, 6, 0, 4, 0, 0)) \
    X(RAD_PROOF_HOOD, armor("Rad-Proof Hood", U, CLOTH, HEAD, 27, 0, 0, 3, 7, 4)) \
    X(GNOMISH_GOGGLES, armor("Gnomish Goggles", U, LEATHER, HEAD, 27, 0, 6, 4, 0, 0)) \
    X(GRUBBIS_GAUNTLETS, armor("Grubbis's Gauntlets", R, MAIL, HANDS, 28, 7, 0, 5, 0, 0)) \
    X(CAVERNDEEP_WRAPS, armor("Caverndeep Wraps", R, CLOTH, HANDS, 28, 0, 0, 4, 8, 5)) \
    X(BURROWER_GRIPS, armor("Burrower Grips", R, LEATHER, HANDS, 28, 0, 8, 4, 0, 0)) \
    X(GYRO_PLATED_LEGGUARDS, armor("Gyro-Plated Legguards", R, MAIL, LEGS, 28, 8, 0, 6, 0, 0)) \
    X(SPARK_WOVEN_PANTS, armor("Spark-Woven Pants", R, CLOTH, LEGS, 28, 0, 0, 5, 10, 5)) \
    X(MECHANISTS_BREECHES, armor("Mechanist's Breeches", R, LEATHER, LEGS, 28, 0, 9, 5, 0, 0)) \
    X(THERMAPLUGGS_UNDOING, weapon("Thermaplugg's Undoing", E, TWO_HANDED, 30, 36, 13, 0, 9, 0, 0)) \
    X(MEKKATORQUES_ARCANO_STAFF, weapon("Mekkatorque's Arcano-Staff", E, STAFF, 30, 32, 0, 0, 7, 15, 8)) \
    X(TINKER_TOWN_HAND_CANNON, weapon("Tinker Town Hand Cannon", E, GUN, 30, 29, 0, 12, 6, 0, 0)) \
    /* Gnomeregan: boss drops */ \
    X(TROGGSTONE_HELM, armor("Troggstone Helm", R, MAIL, HEAD, 26, 7, 0, 4, 0, 0)) \
    X(CAVERNDEEP_COWL, armor("Caverndeep Cowl", R, CLOTH, HEAD, 26, 0, 0, 4, 8, 4)) \
    X(BURROWER_HOOD, armor("Burrower Hood", R, LEATHER, HEAD, 26, 0, 7, 4, 0, 0)) \
    X(FALLOUT_LEGPLATES, armor("Fallout Legplates", R, MAIL, LEGS, 27, 8, 0, 5, 0, 0)) \
    X(RADIANT_LEGGINGS, armor("Radiant Leggings", R, CLOTH, LEGS, 27, 0, 0, 4, 9, 5)) \
    X(SLUDGE_SOAKED_PANTS, armor("Sludge-Soaked Pants", R, LEATHER, LEGS, 27, 0, 9, 4, 0, 0)) \
    X(ELECTROCUTIONER_LEG, weapon("Electrocutioner Leg", R, SWORD, 28, 26, 8, 1, 5, 0, 0)) \
    X(ARC_SPARK_STAFF, weapon("Arc-Spark Staff", R, STAFF, 28, 32, 0, 0, 5, 11, 6)) \
    X(STATIC_LONGBOW, weapon("Static Longbow", R, BOW, 28, 28, 0, 10, 4, 0, 0)) \
    X(MANUAL_CROWD_PUMMELER, weapon("Manual Crowd Pummeler", R, TWO_HANDED, 28, 36, 11, 0, 7, 0, 0)) \
    X(OSCILLATING_POWER_ROBE, armor("Oscillating Power Robe", R, CLOTH, CHEST, 28, 0, 0, 6, 11, 6)) \
    X(GEAR_STUDDED_JERKIN, armor("Gear-Studded Jerkin", R, LEATHER, CHEST, 28, 0, 10, 6, 0, 0)) \
    X(THERMAPLUGGS_LEFT_ARM, weapon("Thermaplugg's Left Arm", R, TWO_HANDED, 29, 36, 12, 0, 8, 0, 0)) \
    X(MEKGINEERS_SPARK_STAFF, weapon("Mekgineer's Spark Staff", R, STAFF, 29, 32, 0, 0, 6, 12, 7)) \
    X(THERMAPLUGGS_BLUNDERBUSS, weapon("Thermaplugg's Blunderbuss", R, GUN, 29, 29, 0, 11, 5, 0, 0)) \
    /* World drops, levels 31 to 35 */ \
    X(ALTERAC_CHAIN_HELM, armor("Alterac Chain Helm", U, MAIL, HEAD, 32, 6, 0, 5, 0, 0)) \
    X(SILKWEAVE_ROBE, armor("Silkweave Robe", U, CLOTH, CHEST, 33, 0, 0, 4, 8, 5)) \
    X(STALKERS_LEGGINGS, armor("Stalker's Leggings", U, LEATHER, LEGS, 33, 0, 8, 4, 0, 0)) \
    X(BATTLEFORGE_GREATSWORD, weapon("Battleforge Greatsword", U, TWO_HANDED, 33, 36, 9, 0, 6, 0, 0)) \
    X(IVORY_STAFF, weapon("Ivory Staff", U, STAFF, 33, 32, 0, 0, 4, 9, 5)) \
    X(IRONBARK_LONGBOW, weapon("Ironbark Longbow", U, BOW, 33, 28, 0, 8, 4, 0, 0)) \
    X(BASTION_SHIELD, shield("Bastion Shield", U, 32, 3, 0, 6, 0, 0)) \
    /* Hillsbrad and the Monastery: supplies and what their creatures carry */ \
    X(GOLDENBARK_APPLE, consumable("Goldenbark Apple", FOOD, 35, 1000, 250, 20)) \
    X(MORNING_GLORY_DEW, consumable("Morning Glory Dew", DRINK, 35, 1900, 250, 20)) \
    X(SUPERIOR_HEALING_POTION, consumable("Superior Healing Potion", POTION, 31, 900, 400, 5)) \
    X(THICK_FUR, junk("Thick Fur", 65, 20)) \
    X(YETI_HORN, junk("Yeti Horn", 75, 20)) \
    X(GHOSTLY_ECTOPLASM, junk("Ghostly Ectoplasm", 75, 20)) \
    X(SCARLET_INSIGNIA, junk("Scarlet Insignia", 70, 20)) \
    /* Southshore's smith */ \
    X(HARDENED_BROADSWORD, weapon("Hardened Broadsword", C, SWORD, 30, 26)) \
    X(SOUTHSHORE_WAR_AXE, weapon("Southshore War Axe", C, AXE, 30, 25)) \
    X(HEAVY_FLANGED_MACE, weapon("Heavy Flanged Mace", C, MACE, 30, 27)) \
    X(IRONBOUND_STAFF, weapon("Ironbound Staff", C, STAFF, 30, 31)) \
    X(HEAVY_CLAYMORE, weapon("Heavy Claymore", C, TWO_HANDED, 30, 35)) \
    X(HILLSBRAD_LONGBOW, weapon("Hillsbrad Longbow", C, BOW, 30, 28)) \
    X(HEAVY_BLUNDERBUSS, weapon("Heavy Blunderbuss", C, GUN, 30, 29)) \
    X(HEATER_SHIELD, shield("Heater Shield", C, 30)) \
    X(REINFORCED_CHAIN_HELM, armor("Reinforced Chain Helm", C, MAIL, HEAD, 30)) \
    X(REINFORCED_CHAIN_HAUBERK, armor("Reinforced Chain Hauberk", C, MAIL, CHEST, 30)) \
    X(REINFORCED_CHAIN_GAUNTLETS, armor("Reinforced Chain Gauntlets", C, MAIL, HANDS, 30)) \
    X(REINFORCED_CHAIN_LEGGINGS, armor("Reinforced Chain Leggings", C, MAIL, LEGS, 30)) \
    X(REINFORCED_CHAIN_BOOTS, armor("Reinforced Chain Boots", C, MAIL, FEET, 30)) \
    X(HARDENED_LEATHER_CAP, armor("Hardened Leather Cap", C, LEATHER, HEAD, 30)) \
    X(HARDENED_LEATHER_VEST, armor("Hardened Leather Vest", C, LEATHER, CHEST, 30)) \
    X(HARDENED_LEATHER_GLOVES, armor("Hardened Leather Gloves", C, LEATHER, HANDS, 30)) \
    X(HARDENED_LEATHER_PANTS, armor("Hardened Leather Pants", C, LEATHER, LEGS, 30)) \
    X(HARDENED_LEATHER_BOOTS, armor("Hardened Leather Boots", C, LEATHER, FEET, 30)) \
    X(SILK_HOOD, armor("Silk Hood", C, CLOTH, HEAD, 30)) \
    X(SILK_ROBE, armor("Silk Robe", C, CLOTH, CHEST, 30)) \
    X(SILK_GLOVES, armor("Silk Gloves", C, CLOTH, HANDS, 30)) \
    X(SILK_PANTS, armor("Silk Pants", C, CLOTH, LEGS, 30)) \
    X(SILK_BOOTS, armor("Silk Boots", C, CLOTH, FEET, 30)) \
    /* Hillsbrad Foothills: quest rewards */ \
    X(SOUTHSHORE_SABATONS, armor("Southshore Sabatons", U, MAIL, FEET, 30, 5, 0, 4, 0, 0)) \
    X(SEASPRAY_SANDALS, armor("Seaspray Sandals", U, CLOTH, FEET, 30, 0, 0, 3, 6, 4)) \
    X(SALTWATER_BOOTS, armor("Saltwater Boots", U, LEATHER, FEET, 30, 0, 6, 4, 0, 0)) \
    X(HILLSBRAD_HELM, armor("Hillsbrad Helm", U, MAIL, HEAD, 30, 6, 0, 4, 0, 0)) \
    X(DARROW_HILL_HOOD, armor("Darrow Hill Hood", U, CLOTH, HEAD, 30, 0, 0, 3, 7, 4)) \
    X(LIONHIDE_CAP, armor("Lionhide Cap", U, LEATHER, HEAD, 30, 0, 7, 4, 0, 0)) \
    X(MAGISTRATES_GAUNTLETS, armor("Magistrate's Gauntlets", U, MAIL, HANDS, 31, 6, 0, 4, 0, 0)) \
    X(GARROTE_WRAPS, armor("Garrote Wraps", U, CLOTH, HANDS, 31, 0, 0, 3, 7, 4)) \
    X(FOOTPADS_GLOVES, armor("Footpad's Gloves", U, LEATHER, HANDS, 31, 0, 7, 4, 0, 0)) \
    X(CRUSHRIDGE_CLEAVER, weapon("Crushridge Cleaver", U, AXE, 32, 26, 7, 0, 4, 0, 0)) \
    X(CRUSHRIDGE_ROD, weapon("Crushridge Rod", U, STAFF, 32, 32, 0, 0, 4, 9, 5)) \
    X(ALTERAC_LONGBOW, weapon("Alterac Longbow", U, BOW, 32, 28, 0, 8, 4, 0, 0)) \
    X(PLAGUEWARDEN_LEGPLATES, armor("Plaguewarden Legplates", R, MAIL, LEGS, 32, 8, 0, 6, 0, 0)) \
    X(APOTHECARYS_LEGGINGS, armor("Apothecary's Leggings", R, CLOTH, LEGS, 32, 0, 0, 5, 9, 5)) \
    X(PLAGUE_HUNTERS_BREECHES, armor("Plague Hunter's Breeches", R, LEATHER, LEGS, 32, 0, 9, 5, 0, 0)) \
    X(DURNHOLDE_HAUBERK, armor("Durnholde Hauberk", R, MAIL, CHEST, 33, 10, 0, 7, 0, 0)) \
    X(SHADOW_MAGES_ROBE, armor("Shadow Mage's Robe", R, CLOTH, CHEST, 33, 0, 0, 6, 12, 7)) \
    X(DURNHOLDE_JERKIN, armor("Durnholde Jerkin", R, LEATHER, CHEST, 33, 0, 11, 7, 0, 0)) \
    X(YETI_FUR_GAUNTLETS, armor("Yeti Fur Gauntlets", R, MAIL, HANDS, 33, 7, 0, 5, 0, 0)) \
    X(FROSTWEAVE_GLOVES, armor("Frostweave Gloves", R, CLOTH, HANDS, 33, 0, 0, 4, 9, 5)) \
    X(YETIHIDE_GLOVES, armor("Yetihide Gloves", R, LEATHER, HANDS, 33, 0, 9, 4, 0, 0)) \
    X(SYNDICATE_SABRE, weapon("Syndicate Sabre", R, SWORD, 34, 26, 8, 2, 5, 0, 0)) \
    X(STAFF_OF_STRAHNBRAD, weapon("Staff of Strahnbrad", R, STAFF, 34, 32, 0, 0, 5, 12, 7)) \
    X(STRAHNBRAD_LONGBOW, weapon("Strahnbrad Longbow", R, BOW, 34, 28, 0, 11, 5, 0, 0)) \
    X(YETI_SLAYERS_SABATONS, armor("Yeti-Slayer's Sabatons", R, MAIL, FEET, 35, 8, 0, 6, 0, 0)) \
    X(FROSTWALKER_SANDALS, armor("Frostwalker Sandals", R, CLOTH, FEET, 35, 0, 0, 5, 9, 5)) \
    X(SNOWSTALKER_BOOTS, armor("Snowstalker Boots", R, LEATHER, FEET, 35, 0, 9, 5, 0, 0)) \
    /* The Scarlet Monastery: quest rewards */ \
    X(ARGENT_CHAIN_VEST, armor("Argent Chain Vest", U, MAIL, CHEST, 31, 7, 0, 5, 0, 0)) \
    X(GHOSTWEAVE_ROBE, armor("Ghostweave Robe", U, CLOTH, CHEST, 31, 0, 0, 4, 8, 5)) \
    X(SPIRITBOUND_TUNIC, armor("Spiritbound Tunic", U, LEATHER, CHEST, 31, 0, 8, 5, 0, 0)) \
    X(CRYPT_WARDENS_HELM, armor("Crypt Warden's Helm", R, MAIL, HEAD, 32, 8, 0, 5, 0, 0)) \
    X(ARGENT_COWL, armor("Argent Cowl", R, CLOTH, HEAD, 32, 0, 0, 5, 9, 5)) \
    X(GRAVE_STALKERS_CAP, armor("Grave Stalker's Cap", R, LEATHER, HEAD, 32, 0, 9, 5, 0, 0)) \
    X(CRYPTWARDEN_LEGPLATES, armor("Cryptwarden Legplates", R, MAIL, LEGS, 33, 9, 0, 6, 0, 0)) \
    X(LEGGINGS_OF_ETERNAL_REST, armor("Leggings of Eternal Rest", R, CLOTH, LEGS, 33, 0, 0, 5, 10, 6)) \
    X(BONECARVER_BREECHES, armor("Bonecarver Breeches", R, LEATHER, LEGS, 33, 0, 10, 5, 0, 0)) \
    X(ZEALOTS_GAUNTLETS, armor("Zealot's Gauntlets", R, MAIL, HANDS, 34, 8, 0, 5, 0, 0)) \
    X(ZEALOTS_WRAPS, armor("Zealot's Wraps", R, CLOTH, HANDS, 34, 0, 0, 4, 10, 5)) \
    X(ZEALOTS_GRIPS, armor("Zealot's Grips", R, LEATHER, HANDS, 34, 0, 10, 5, 0, 0)) \
    X(TITAN_FORGED_BLADE, weapon("Titan-Forged Blade", R, SWORD, 34, 26, 9, 2, 5, 0, 0)) \
    X(STAFF_OF_THE_TITANS, weapon("Staff of the Titans", R, STAFF, 34, 32, 0, 0, 6, 13, 7)) \
    X(EXPLORERS_LONGRIFLE, weapon("Explorer's Longrifle", R, GUN, 34, 29, 0, 12, 5, 0, 0)) \
    X(BLADE_OF_THE_DEVOUT, weapon("Blade of the Devout", E, TWO_HANDED, 35, 36, 15, 0, 10, 0, 0)) \
    X(STAFF_OF_THE_SILVER_HAND, weapon("Staff of the Silver Hand", E, STAFF, 35, 32, 0, 0, 8, 17, 9)) \
    X(BOW_OF_THE_ARGENT_WATCH, weapon("Bow of the Argent Watch", E, BOW, 35, 28, 0, 14, 7, 0, 0)) \
    /* The Scarlet Monastery: boss drops */ \
    X(INTERROGATORS_HELM, armor("Interrogator's Helm", R, MAIL, HEAD, 31, 7, 0, 5, 0, 0)) \
    X(HOOD_OF_CONFESSION, armor("Hood of Confession", R, CLOTH, HEAD, 31, 0, 0, 4, 9, 5)) \
    X(TORTURERS_MASK, armor("Torturer's Mask", R, LEATHER, HEAD, 31, 0, 9, 4, 0, 0)) \
    X(SLEEPLESS_GAUNTLETS, armor("Sleepless Gauntlets", R, MAIL, HANDS, 32, 7, 0, 5, 0, 0)) \
    X(GHOSTSHROUD_WRAPS, armor("Ghostshroud Wraps", R, CLOTH, HANDS, 32, 0, 0, 4, 9, 5)) \
    X(GHOSTWALKER_GRIPS, armor("Ghostwalker Grips", R, LEATHER, HANDS, 32, 0, 9, 4, 0, 0)) \
    X(THALNOS_CLEAVER, weapon("Thalnos's Cleaver", R, AXE, 33, 26, 8, 1, 5, 0, 0)) \
    X(STAFF_OF_THE_BLOODMAGE, weapon("Staff of the Bloodmage", R, STAFF, 33, 32, 0, 0, 5, 12, 7)) \
    X(FLAMESPIKE_BOW, weapon("Flamespike Bow", R, BOW, 33, 28, 0, 11, 5, 0, 0)) \
    X(IRONSPINES_RIBCAGE, armor("Ironspine's Ribcage", R, MAIL, CHEST, 33, 10, 0, 7, 0, 0)) \
    X(SHROUD_OF_THE_OSSUARY, armor("Shroud of the Ossuary", R, CLOTH, CHEST, 33, 0, 0, 6, 12, 7)) \
    X(BONE_STUDDED_JERKIN, armor("Bone-Studded Jerkin", R, LEATHER, CHEST, 33, 0, 11, 7, 0, 0)) \
    X(HOUNDMASTERS_SABATONS, armor("Houndmaster's Sabatons", R, MAIL, FEET, 34, 8, 0, 6, 0, 0)) \
    X(KENNELKEEPERS_SLIPPERS, armor("Kennelkeeper's Slippers", R, CLOTH, FEET, 34, 0, 0, 5, 10, 5)) \
    X(HOUNDMASTERS_BOOTS, armor("Houndmaster's Boots", R, LEATHER, FEET, 34, 0, 10, 5, 0, 0)) \
    X(HYPNOTIC_BLADE, weapon("Hypnotic Blade", R, SWORD, 35, 26, 9, 2, 6, 0, 0)) \
    X(ILLUSIONARY_ROD, weapon("Illusionary Rod", R, STAFF, 35, 32, 0, 0, 6, 14, 8)) \
    X(SCARLET_LONGBOW, weapon("Scarlet Longbow", R, BOW, 35, 28, 0, 12, 6, 0, 0)) \
    /* World drops, levels 36 to 40 */ \
    X(EMBERFORGED_HELM, armor("Emberforged Helm", U, MAIL, HEAD, 37, 7, 0, 6, 0, 0)) \
    X(STARSILK_ROBE, armor("Starsilk Robe", U, CLOTH, CHEST, 38, 0, 0, 5, 10, 6)) \
    X(JUNGLESTALKER_LEGGINGS, armor("Junglestalker Leggings", U, LEATHER, LEGS, 38, 0, 10, 5, 0, 0)) \
    X(CRESCENT_GREATSWORD, weapon("Crescent Greatsword", U, TWO_HANDED, 38, 36, 11, 0, 7, 0, 0)) \
    X(SERPENTWOOD_STAFF, weapon("Serpentwood Staff", U, STAFF, 38, 32, 0, 0, 5, 11, 6)) \
    X(THORNROOT_LONGBOW, weapon("Thornroot Longbow", U, BOW, 38, 28, 0, 10, 5, 0, 0)) \
    X(BULWARK_OF_THE_VALE, shield("Bulwark of the Vale", U, 37, 4, 0, 7, 0, 0)) \
    /* Stranglethorn: what its people and creatures carry */ \
    X(TROLL_TUSK, junk("Troll Tusk", 80, 20)) \
    X(RAPTOR_CLAW, junk("Raptor Claw", 70, 20)) \
    /* Booty Bay's smith */ \
    X(JUNGLE_CUTLASS, weapon("Jungle Cutlass", C, SWORD, 35, 26)) \
    X(GOBLIN_CHOPPER, weapon("Goblin Chopper", C, AXE, 35, 25)) \
    X(SPIKED_CUDGEL, weapon("Spiked Cudgel", C, MACE, 35, 27)) \
    X(JUNGLE_STAFF, weapon("Jungle Staff", C, STAFF, 35, 31)) \
    X(GOBLIN_GREATSWORD, weapon("Goblin Greatsword", C, TWO_HANDED, 35, 35)) \
    X(JUNGLE_LONGBOW, weapon("Jungle Longbow", C, BOW, 35, 28)) \
    X(GOBLIN_RIFLE, weapon("Goblin Rifle", C, GUN, 35, 29)) \
    X(BOOTY_BAY_BUCKLER, shield("Booty Bay Buckler", C, 35)) \
    X(MITHRIL_CHAIN_HELM, armor("Mithril Chain Helm", C, MAIL, HEAD, 35)) \
    X(MITHRIL_CHAIN_HAUBERK, armor("Mithril Chain Hauberk", C, MAIL, CHEST, 35)) \
    X(MITHRIL_CHAIN_GAUNTLETS, armor("Mithril Chain Gauntlets", C, MAIL, HANDS, 35)) \
    X(MITHRIL_CHAIN_LEGGINGS, armor("Mithril Chain Leggings", C, MAIL, LEGS, 35)) \
    X(MITHRIL_CHAIN_BOOTS, armor("Mithril Chain Boots", C, MAIL, FEET, 35)) \
    X(BARBARIC_LEATHER_CAP, armor("Barbaric Leather Cap", C, LEATHER, HEAD, 35)) \
    X(BARBARIC_LEATHER_VEST, armor("Barbaric Leather Vest", C, LEATHER, CHEST, 35)) \
    X(BARBARIC_LEATHER_GLOVES, armor("Barbaric Leather Gloves", C, LEATHER, HANDS, 35)) \
    X(BARBARIC_LEATHER_PANTS, armor("Barbaric Leather Pants", C, LEATHER, LEGS, 35)) \
    X(BARBARIC_LEATHER_BOOTS, armor("Barbaric Leather Boots", C, LEATHER, FEET, 35)) \
    X(SHADOWEAVE_HOOD, armor("Shadoweave Hood", C, CLOTH, HEAD, 35)) \
    X(SHADOWEAVE_ROBE, armor("Shadoweave Robe", C, CLOTH, CHEST, 35)) \
    X(SHADOWEAVE_GLOVES, armor("Shadoweave Gloves", C, CLOTH, HANDS, 35)) \
    X(SHADOWEAVE_PANTS, armor("Shadoweave Pants", C, CLOTH, LEGS, 35)) \
    X(SHADOWEAVE_BOOTS, armor("Shadoweave Boots", C, CLOTH, FEET, 35)) \
    /* Stranglethorn Vale: quest rewards */ \
    X(REBEL_BROADSWORD, weapon("Rebel Broadsword", U, SWORD, 35, 26, 7, 1, 5, 0, 0)) \
    X(ZUL_KUNDA_STAFF, weapon("Zul'Kunda Staff", U, STAFF, 35, 32, 0, 0, 5, 10, 5)) \
    X(REBEL_LONGBOW, weapon("Rebel Longbow", U, BOW, 35, 28, 0, 9, 5, 0, 0)) \
    X(TIGERSTRIPE_SABATONS, armor("Tigerstripe Sabatons", U, MAIL, FEET, 35, 6, 0, 5, 0, 0)) \
    X(JUNGLE_SANDALS, armor("Jungle Sandals", U, CLOTH, FEET, 35, 0, 0, 4, 8, 4)) \
    X(TIGERHIDE_BOOTS, armor("Tigerhide Boots", U, LEATHER, FEET, 35, 0, 7, 5, 0, 0)) \
    X(HEADHUNTERS_GAUNTLETS, armor("Headhunter's Gauntlets", U, MAIL, HANDS, 36, 7, 0, 5, 0, 0)) \
    X(VOODOO_WRAPS, armor("Voodoo Wraps", U, CLOTH, HANDS, 36, 0, 0, 4, 8, 5)) \
    X(HEADHUNTERS_GRIPS, armor("Headhunter's Grips", U, LEATHER, HANDS, 36, 0, 8, 5, 0, 0)) \
    X(PANTHER_HELM, armor("Panther Helm", U, MAIL, HEAD, 36, 7, 0, 5, 0, 0)) \
    X(SHADOWMAW_COWL, armor("Shadowmaw Cowl", U, CLOTH, HEAD, 36, 0, 0, 4, 8, 5)) \
    X(PANTHERHIDE_CAP, armor("Pantherhide Cap", U, LEATHER, HEAD, 36, 0, 8, 5, 0, 0)) \
    X(RAPTORSCALE_LEGGUARDS, armor("Raptorscale Legguards", U, MAIL, LEGS, 37, 8, 0, 6, 0, 0)) \
    X(JUNGLE_LEGGINGS, armor("Jungle Leggings", U, CLOTH, LEGS, 37, 0, 0, 5, 9, 5)) \
    X(RAPTORHIDE_PANTS, armor("Raptorhide Pants", U, LEATHER, LEGS, 37, 0, 9, 6, 0, 0)) \
    X(BOARDING_HAUBERK, armor("Boarding Hauberk", R, MAIL, CHEST, 37, 11, 0, 8, 0, 0)) \
    X(BUCCANEERS_ROBE, armor("Buccaneer's Robe", R, CLOTH, CHEST, 37, 0, 0, 7, 13, 7)) \
    X(CORSAIRS_VEST, armor("Corsair's Vest", R, LEATHER, CHEST, 37, 0, 12, 8, 0, 0)) \
    X(BARNILS_HUNTING_BLADE, weapon("Barnil's Hunting Blade", R, SWORD, 37, 26, 9, 2, 6, 0, 0)) \
    X(STONEPOTS_WALKING_STICK, weapon("Stonepot's Walking Stick", R, STAFF, 37, 32, 0, 0, 6, 14, 8)) \
    X(NESINGWARY_4000, weapon("Nesingwary 4000", R, GUN, 37, 29, 0, 13, 6, 0, 0)) \
    X(LASHTAIL_HELM, armor("Lashtail Helm", U, MAIL, HEAD, 38, 8, 0, 5, 0, 0)) \
    X(RAPTOR_HUNTERS_HOOD, armor("Raptor Hunter's Hood", U, CLOTH, HEAD, 38, 0, 0, 5, 9, 5)) \
    X(LASHTAIL_CAP, armor("Lashtail Cap", U, LEATHER, HEAD, 38, 0, 9, 5, 0, 0)) \
    X(SILVERBACK_HAUBERK, armor("Silverback Hauberk", U, MAIL, CHEST, 38, 9, 0, 6, 0, 0)) \
    X(MISTVALE_ROBE, armor("Mistvale Robe", U, CLOTH, CHEST, 38, 0, 0, 5, 10, 6)) \
    X(GORILLAHIDE_VEST, armor("Gorillahide Vest", U, LEATHER, CHEST, 38, 0, 10, 6, 0, 0)) \
    X(DECKHANDS_LEGGUARDS, armor("Deckhand's Legguards", U, MAIL, LEGS, 38, 9, 0, 6, 0, 0)) \
    X(SAILCLOTH_TROUSERS, armor("Sailcloth Trousers", U, CLOTH, LEGS, 38, 0, 0, 5, 10, 5)) \
    X(SEADOG_BREECHES, armor("Seadog Breeches", U, LEATHER, LEGS, 38, 0, 10, 6, 0, 0)) \
    X(FIRALLONS_CUTLASS, weapon("Firallon's Cutlass", R, SWORD, 39, 26, 10, 2, 6, 0, 0)) \
    X(FLEET_MASTERS_STAFF, weapon("Fleet Master's Staff", R, STAFF, 39, 32, 0, 0, 6, 15, 8)) \
    X(BLOODSAIL_BLUNDERBUSS, weapon("Bloodsail Blunderbuss", R, GUN, 39, 29, 0, 14, 6, 0, 0)) \
    X(BIG_GAME_HUNTERS_HELM, armor("Big Game Hunter's Helm", R, MAIL, HEAD, 39, 10, 0, 7, 0, 0)) \
    X(SAFARI_HAT, armor("Safari Hat", R, CLOTH, HEAD, 39, 0, 0, 6, 12, 7)) \
    X(TIGERSKULL_CAP, armor("Tigerskull Cap", R, LEATHER, HEAD, 39, 0, 12, 7, 0, 0)) \
    /* The Scarlet Monastery's Armory and Cathedral: quest rewards */ \
    X(ARMORY_SABATONS, armor("Armory Sabatons", U, MAIL, FEET, 37, 7, 0, 5, 0, 0)) \
    X(ARMORERS_SLIPPERS, armor("Armorer's Slippers", U, CLOTH, FEET, 37, 0, 0, 4, 9, 5)) \
    X(SQUIRES_BOOTS, armor("Squire's Boots", U, LEATHER, FEET, 37, 0, 9, 5, 0, 0)) \
    X(BLADE_OF_THE_ARMORY, weapon("Blade of the Armory", R, TWO_HANDED, 38, 36, 14, 0, 9, 0, 0)) \
    X(STAFF_OF_THE_CHAMPION, weapon("Staff of the Champion", R, STAFF, 38, 32, 0, 0, 6, 14, 8)) \
    X(ARMORY_LONGBOW, weapon("Armory Longbow", R, BOW, 38, 28, 0, 13, 6, 0, 0)) \
    X(CATHEDRAL_SABATONS, armor("Cathedral Sabatons", R, MAIL, FEET, 39, 9, 0, 7, 0, 0)) \
    X(ABBOTS_SLIPPERS, armor("Abbot's Slippers", R, CLOTH, FEET, 39, 0, 0, 5, 11, 6)) \
    X(CRIMSON_BOOTS, armor("Crimson Boots", R, LEATHER, FEET, 39, 0, 11, 6, 0, 0)) \
    X(SWORD_OF_OMEN, weapon("Sword of Omen", E, TWO_HANDED, 40, 36, 17, 0, 12, 0, 0)) \
    X(STAFF_OF_LORICA, weapon("Staff of Lorica", E, STAFF, 40, 32, 0, 0, 9, 19, 10)) \
    X(BOW_OF_ABSOLUTION, weapon("Bow of Absolution", E, BOW, 40, 28, 0, 16, 8, 0, 0)) \
    /* Stranglethorn's elites and rare */ \
    X(SABERTOOTH_GAUNTLETS, armor("Sabertooth Gauntlets", R, MAIL, HANDS, 39, 9, 0, 6, 0, 0)) \
    X(SILVERSTRIPE_GLOVES, armor("Silverstripe Gloves", R, CLOTH, HANDS, 39, 0, 0, 5, 11, 6)) \
    X(BANGALASHS_GRIPS, armor("Bangalash's Grips", R, LEATHER, HANDS, 39, 0, 11, 6, 0, 0)) \
    X(CAPTAINS_SABATONS, armor("Captain's Sabatons", R, MAIL, FEET, 39, 9, 0, 6, 0, 0)) \
    X(SEAFARERS_SLIPPERS, armor("Seafarer's Slippers", R, CLOTH, FEET, 39, 0, 0, 5, 11, 6)) \
    X(FIRALLONS_BOOTS, armor("Firallon's Boots", R, LEATHER, FEET, 39, 0, 11, 6, 0, 0)) \
    X(MOGHS_LEGPLATES, armor("Mogh's Legplates", R, MAIL, LEGS, 38, 10, 0, 7, 0, 0)) \
    X(LEGGINGS_OF_THE_UNDYING, armor("Leggings of the Undying", R, CLOTH, LEGS, 38, 0, 0, 6, 12, 7)) \
    X(VOODOO_BREECHES, armor("Voodoo Breeches", R, LEATHER, LEGS, 38, 0, 12, 7, 0, 0)) \
    /* The Scarlet Monastery's Armory and Cathedral: boss drops */ \
    X(HERODS_BREASTPLATE, armor("Herod's Breastplate", R, MAIL, CHEST, 38, 11, 0, 8, 0, 0)) \
    X(CHAMPIONS_ROBE, armor("Champion's Robe", R, CLOTH, CHEST, 38, 0, 0, 7, 13, 7)) \
    X(BLOODWHIRL_TUNIC, armor("Bloodwhirl Tunic", R, LEATHER, CHEST, 38, 0, 12, 8, 0, 0)) \
    X(INQUISITORS_HELM, armor("Inquisitor's Helm", R, MAIL, HEAD, 39, 10, 0, 7, 0, 0)) \
    X(HOOD_OF_PENANCE, armor("Hood of Penance", R, CLOTH, HEAD, 39, 0, 0, 6, 12, 7)) \
    X(MASK_OF_ATONEMENT, armor("Mask of Atonement", R, LEATHER, HEAD, 39, 0, 12, 7, 0, 0)) \
    X(MOGRAINES_MIGHT, weapon("Mograine's Might", R, TWO_HANDED, 40, 36, 15, 0, 10, 0, 0)) \
    X(STAFF_OF_THE_COMMANDER, weapon("Staff of the Commander", R, STAFF, 40, 32, 0, 0, 7, 16, 9)) \
    X(CRUSADERS_LONGBOW, weapon("Crusader's Longbow", R, BOW, 40, 28, 0, 14, 7, 0, 0)) \
    X(GAUNTLETS_OF_DIVINITY, armor("Gauntlets of Divinity", R, MAIL, HANDS, 40, 10, 0, 7, 0, 0)) \
    X(WHITEMANES_GLOVES, armor("Whitemane's Gloves", R, CLOTH, HANDS, 40, 0, 0, 6, 12, 7)) \
    X(GRIPS_OF_RESURRECTION, armor("Grips of Resurrection", R, LEATHER, HANDS, 40, 0, 12, 7, 0, 0)) \
    /* Tanaris and Thousand Needles: what its people and creatures carry */ \
    X(SCORPID_STINGER, junk("Scorpid Stinger", 90, 20)) \
    X(QUILBOAR_TUSK, junk("Quilboar Tusk", 85, 20)) \
    X(RUNECLOTH, junk("Runecloth", 120, 20)) \
    X(ROASTED_QUAIL, consumable("Roasted Quail", FOOD, 45, 1400, 350, 20)) \
    X(SPARKLING_DESERT_WATER, consumable("Sparkling Desert Water", DRINK, 45, 2600, 350, 20)) \
    X(MAJOR_HEALING_POTION, consumable("Major Healing Potion", POTION, 41, 1300, 600, 5)) \
    /* Gadgetzan's smith */ \
    X(GADGETZAN_SABER, weapon("Gadgetzan Saber", C, SWORD, 40, 26)) \
    X(STEAMWHEEDLE_CLEAVER, weapon("Steamwheedle Cleaver", C, AXE, 40, 25)) \
    X(DESERT_MAUL, weapon("Desert Maul", C, MACE, 40, 27)) \
    X(SANDWALKER_STAFF, weapon("Sandwalker Staff", C, STAFF, 40, 31)) \
    X(GADGETZAN_GREATAXE, weapon("Gadgetzan Greataxe", C, TWO_HANDED, 40, 35)) \
    X(DESERT_LONGBOW, weapon("Desert Longbow", C, BOW, 40, 28)) \
    X(STEAMWHEEDLE_RIFLE, weapon("Steamwheedle Rifle", C, GUN, 40, 29)) \
    X(GADGETZAN_KITE_SHIELD, shield("Gadgetzan Kite Shield", C, 40)) \
    X(DESERT_CHAIN_HELM, armor("Desert Chain Helm", C, MAIL, HEAD, 40)) \
    X(DESERT_CHAIN_HAUBERK, armor("Desert Chain Hauberk", C, MAIL, CHEST, 40)) \
    X(DESERT_CHAIN_GAUNTLETS, armor("Desert Chain Gauntlets", C, MAIL, HANDS, 40)) \
    X(DESERT_CHAIN_LEGGINGS, armor("Desert Chain Leggings", C, MAIL, LEGS, 40)) \
    X(DESERT_CHAIN_BOOTS, armor("Desert Chain Boots", C, MAIL, FEET, 40)) \
    X(DUNEWALKER_CAP, armor("Dunewalker Cap", C, LEATHER, HEAD, 40)) \
    X(DUNEWALKER_VEST, armor("Dunewalker Vest", C, LEATHER, CHEST, 40)) \
    X(DUNEWALKER_GLOVES, armor("Dunewalker Gloves", C, LEATHER, HANDS, 40)) \
    X(DUNEWALKER_PANTS, armor("Dunewalker Pants", C, LEATHER, LEGS, 40)) \
    X(DUNEWALKER_BOOTS, armor("Dunewalker Boots", C, LEATHER, FEET, 40)) \
    X(SANDSILK_HOOD, armor("Sandsilk Hood", C, CLOTH, HEAD, 40)) \
    X(SANDSILK_ROBE, armor("Sandsilk Robe", C, CLOTH, CHEST, 40)) \
    X(SANDSILK_GLOVES, armor("Sandsilk Gloves", C, CLOTH, HANDS, 40)) \
    X(SANDSILK_PANTS, armor("Sandsilk Pants", C, CLOTH, LEGS, 40)) \
    X(SANDSILK_BOOTS, armor("Sandsilk Boots", C, CLOTH, FEET, 40)) \
    /* Tanaris and Thousand Needles: quest rewards */ \
    X(BANDIT_HUNTER_GAUNTLETS, armor("Bandit Hunter Gauntlets", U, MAIL, HANDS, 41, 8, 0, 5, 0, 0)) \
    X(OASIS_WRAPS, armor("Oasis Wraps", U, CLOTH, HANDS, 41, 0, 0, 4, 9, 5)) \
    X(WASTEWANDER_GRIPS, armor("Wastewander Grips", U, LEATHER, HANDS, 41, 0, 9, 5, 0, 0)) \
    X(WATERSPRING_SABATONS, armor("Waterspring Sabatons", U, MAIL, FEET, 41, 8, 0, 5, 0, 0)) \
    X(OASIS_SANDALS, armor("Oasis Sandals", U, CLOTH, FEET, 41, 0, 0, 4, 9, 5)) \
    X(DUNE_TREADS, armor("Dune Treads", U, LEATHER, FEET, 41, 0, 9, 5, 0, 0)) \
    X(GALAK_HELM, armor("Galak Helm", U, MAIL, HEAD, 41, 9, 0, 5, 0, 0)) \
    X(WINDCHASER_HOOD, armor("Windchaser Hood", U, CLOTH, HEAD, 41, 0, 0, 4, 10, 6)) \
    X(GALAK_HIDE_CAP, armor("Galak Hide Cap", U, LEATHER, HEAD, 41, 0, 10, 5, 0, 0)) \
    X(QUILGUARD_LEGGUARDS, armor("Quilguard Legguards", U, MAIL, LEGS, 41, 9, 0, 6, 0, 0)) \
    X(BRAMBLECLOTH_PANTS, armor("Bramblecloth Pants", U, CLOTH, LEGS, 41, 0, 0, 5, 10, 6)) \
    X(THORNHIDE_PANTS, armor("Thornhide Pants", U, LEATHER, LEGS, 41, 0, 10, 6, 0, 0)) \
    X(BONELINK_GAUNTLETS, armor("Bonelink Gauntlets", U, MAIL, HANDS, 41, 8, 0, 5, 0, 0)) \
    X(BARROWCLOTH_GLOVES, armor("Barrowcloth Gloves", U, CLOTH, HANDS, 41, 0, 0, 4, 9, 5)) \
    X(DEATHSHEAD_GRIPS, armor("Deathshead Grips", U, LEATHER, HANDS, 41, 0, 9, 5, 0, 0)) \
    X(SHAKEDOWN_LEGGUARDS, armor("Shakedown Legguards", U, MAIL, LEGS, 42, 9, 0, 7, 0, 0)) \
    X(STEAMWHEEDLE_TROUSERS, armor("Steamwheedle Trousers", U, CLOTH, LEGS, 42, 0, 0, 6, 10, 6)) \
    X(SMUGGLER_BREECHES, armor("Smuggler Breeches", U, LEATHER, LEGS, 42, 0, 10, 7, 0, 0)) \
    X(SANDSORROW_GAUNTLETS, armor("Sandsorrow Gauntlets", U, MAIL, HANDS, 42, 8, 0, 6, 0, 0)) \
    X(SANDSORROW_WRAPS, armor("Sandsorrow Wraps", U, CLOTH, HANDS, 42, 0, 0, 5, 9, 5)) \
    X(SANDSORROW_GRIPS, armor("Sandsorrow Grips", U, LEATHER, HANDS, 42, 0, 9, 6, 0, 0)) \
    X(RACEWAY_WRENCH, weapon("Raceway Wrench", U, MACE, 42, 27, 8, 1, 6, 0, 0)) \
    X(SALT_FLATS_STAFF, weapon("Salt Flats Staff", U, STAFF, 42, 32, 0, 0, 6, 13, 7)) \
    X(RACERS_RIFLE, weapon("Racer's Rifle", U, GUN, 42, 29, 0, 13, 6, 0, 0)) \
    X(RAZORFLANK_BLADE, weapon("Razorflank Blade", R, SWORD, 42, 26, 10, 2, 7, 0, 0)) \
    X(CRONES_STAFF, weapon("Crone's Staff", R, STAFF, 42, 32, 0, 0, 7, 16, 9)) \
    X(THORNWEAVE_BOW, weapon("Thornweave Bow", R, BOW, 42, 28, 0, 15, 7, 0, 0)) \
    X(CARAPACE_HAUBERK, armor("Carapace Hauberk", R, MAIL, CHEST, 42, 12, 0, 9, 0, 0)) \
    X(WEBSPINNER_ROBE, armor("Webspinner Robe", R, CLOTH, CHEST, 42, 0, 0, 8, 14, 9)) \
    X(TOMB_FIEND_JERKIN, armor("Tomb Fiend Jerkin", R, LEATHER, CHEST, 42, 0, 13, 9, 0, 0)) \
    X(OGRESLAYER_HAUBERK, armor("Ogreslayer Hauberk", U, MAIL, CHEST, 43, 10, 0, 7, 0, 0)) \
    X(DUNEMAUL_ROBE, armor("Dunemaul Robe", U, CLOTH, CHEST, 43, 0, 0, 6, 11, 7)) \
    X(OGREHIDE_JERKIN, armor("Ogrehide Jerkin", U, LEATHER, CHEST, 43, 0, 11, 7, 0, 0)) \
    X(CALIPHS_SCIMITAR, weapon("Caliph's Scimitar", R, SWORD, 43, 26, 11, 2, 7, 0, 0)) \
    X(SCORPIDSTING_STAFF, weapon("Scorpidsting Staff", R, STAFF, 43, 32, 0, 0, 7, 17, 9)) \
    X(STINGER_LONGBOW, weapon("Stinger Longbow", R, BOW, 43, 28, 0, 15, 7, 0, 0)) \
    X(LIGHTFORGED_BLADE, weapon("Lightforged Blade", R, SWORD, 43, 26, 11, 2, 7, 0, 0)) \
    X(STAFF_OF_THE_DAWN, weapon("Staff of the Dawn", R, STAFF, 43, 32, 0, 0, 7, 17, 9)) \
    X(ARGENT_LONGBOW, weapon("Argent Longbow", R, BOW, 43, 28, 0, 15, 7, 0, 0)) \
    X(FIREBEARDS_HELM, armor("Firebeard's Helm", R, MAIL, HEAD, 44, 12, 0, 8, 0, 0)) \
    X(CORSAIRS_TRICORN, armor("Corsair's Tricorn", R, CLOTH, HEAD, 44, 0, 0, 7, 14, 9)) \
    X(PIRATE_BANDANA, armor("Pirate Bandana", R, LEATHER, HEAD, 44, 0, 13, 8, 0, 0)) \
    X(TEMPERED_TROLL_BLADE, weapon("Tempered Troll Blade", U, SWORD, 44, 26, 9, 1, 6, 0, 0)) \
    X(JUJU_STAFF, weapon("Juju Staff", U, STAFF, 44, 32, 0, 0, 6, 14, 7)) \
    X(TROLL_HUNTERS_BOW, weapon("Troll Hunter's Bow", U, BOW, 44, 28, 0, 14, 6, 0, 0)) \
    X(SCARAB_PLATED_BOOTS, armor("Scarab Plated Boots", U, MAIL, FEET, 44, 9, 0, 6, 0, 0)) \
    X(SCARAB_SLIPPERS, armor("Scarab Slippers", U, CLOTH, FEET, 44, 0, 0, 5, 10, 6)) \
    X(CARAPACE_BOOTS, armor("Carapace Boots", U, LEATHER, FEET, 44, 0, 10, 6, 0, 0)) \
    X(HYDRASCALE_LEGGUARDS, armor("Hydrascale Legguards", R, MAIL, LEGS, 45, 12, 0, 8, 0, 0)) \
    X(TIDEWEAVE_LEGGINGS, armor("Tideweave Leggings", R, CLOTH, LEGS, 45, 0, 0, 7, 14, 9)) \
    X(SCALED_LEGGINGS, armor("Scaled Leggings", R, LEATHER, LEGS, 45, 0, 13, 8, 0, 0)) \
    X(UKORZS_GREATAXE, weapon("Ukorz's Greataxe", E, TWO_HANDED, 45, 36, 19, 0, 13, 0, 0)) \
    X(STAFF_OF_THE_SANDFURY, weapon("Staff of the Sandfury", E, STAFF, 45, 32, 0, 0, 9, 20, 10)) \
    X(ZUL_FARRAK_LONGBOW, weapon("Zul'Farrak Longbow", E, BOW, 45, 28, 0, 18, 9, 0, 0)) \
    /* Tanaris's elites and rare */ \
    X(SCORPIDSTING_GAUNTLETS, armor("Scorpidsting Gauntlets", R, MAIL, HANDS, 43, 10, 0, 7, 0, 0)) \
    X(CALIPHS_GLOVES, armor("Caliph's Gloves", R, CLOTH, HANDS, 43, 0, 0, 6, 12, 7)) \
    X(SANDSTALKER_GRIPS, armor("Sandstalker Grips", R, LEATHER, HANDS, 43, 0, 11, 7, 0, 0)) \
    X(ANDRES_SABATONS, armor("Andre's Sabatons", R, MAIL, FEET, 44, 11, 0, 7, 0, 0)) \
    X(CAPTAINS_SLIPPERS, armor("Captain's Slippers", R, CLOTH, FEET, 44, 0, 0, 6, 13, 8)) \
    X(FIREBEARD_BOOTS, armor("Firebeard Boots", R, LEATHER, FEET, 44, 0, 12, 7, 0, 0)) \
    X(OMGORNS_LEGPLATES, armor("Omgorn's Legplates", R, MAIL, LEGS, 44, 12, 0, 8, 0, 0)) \
    X(LEGGINGS_OF_THE_LOST, armor("Leggings of the Lost", R, CLOTH, LEGS, 44, 0, 0, 7, 14, 9)) \
    X(OGRE_HIDE_BREECHES, armor("Ogre-Hide Breeches", R, LEATHER, LEGS, 44, 0, 13, 8, 0, 0)) \
    /* Razorfen Kraul, Razorfen Downs and Zul'Farrak: boss drops */ \
    X(THORNCURSE_GAUNTLETS, armor("Thorncurse Gauntlets", R, MAIL, HANDS, 40, 10, 0, 6, 0, 0)) \
    X(THORNWEAVE_GLOVES, armor("Thornweave Gloves", R, CLOTH, HANDS, 40, 0, 0, 5, 12, 7)) \
    X(BRAMBLEHIDE_GRIPS, armor("Bramblehide Grips", R, LEATHER, HANDS, 40, 0, 11, 6, 0, 0)) \
    X(DEATH_SPEAKER_HELM, armor("Death Speaker Helm", R, MAIL, HEAD, 40, 11, 0, 7, 0, 0)) \
    X(JARGBAS_COWL, armor("Jargba's Cowl", R, CLOTH, HEAD, 40, 0, 0, 6, 13, 8)) \
    X(SPEAKERS_MASK, armor("Speaker's Mask", R, LEATHER, HEAD, 40, 0, 12, 7, 0, 0)) \
    X(RAMTUSKS_CLEAVER, weapon("Ramtusk's Cleaver", R, AXE, 41, 25, 10, 2, 6, 0, 0)) \
    X(RAMSTAFF, weapon("Ramstaff", R, STAFF, 41, 32, 0, 0, 6, 16, 9)) \
    X(TUSKER_LONGBOW, weapon("Tusker Longbow", R, BOW, 41, 28, 0, 15, 6, 0, 0)) \
    X(STAMPEDE_SABATONS, armor("Stampede Sabatons", R, MAIL, FEET, 41, 10, 0, 6, 0, 0)) \
    X(AGAM_AR_SLIPPERS, armor("Agam'ar Slippers", R, CLOTH, FEET, 41, 0, 0, 5, 12, 7)) \
    X(BOARHIDE_BOOTS, armor("Boarhide Boots", R, LEATHER, FEET, 41, 0, 11, 6, 0, 0)) \
    X(RAZORFLANK_HAUBERK, armor("Razorflank Hauberk", R, MAIL, CHEST, 42, 12, 0, 9, 0, 0)) \
    X(CHARLGAS_ROBE, armor("Charlga's Robe", R, CLOTH, CHEST, 42, 0, 0, 8, 14, 9)) \
    X(CRONES_VEST, armor("Crone's Vest", R, LEATHER, CHEST, 42, 0, 13, 9, 0, 0)) \
    X(SILK_WRAPPED_LEGGUARDS, armor("Silk-Wrapped Legguards", R, MAIL, LEGS, 42, 11, 0, 8, 0, 0)) \
    X(SPIDERSILK_LEGGINGS, armor("Spidersilk Leggings", R, CLOTH, LEGS, 42, 0, 0, 7, 13, 8)) \
    X(FIENDHIDE_PANTS, armor("Fiendhide Pants", R, LEATHER, LEGS, 42, 0, 12, 8, 0, 0)) \
    X(MORDRESHS_BLADE, weapon("Mordresh's Blade", R, SWORD, 42, 26, 10, 2, 7, 0, 0)) \
    X(STAFF_OF_THE_FIRE_EYE, weapon("Staff of the Fire Eye", R, STAFF, 42, 32, 0, 0, 7, 16, 9)) \
    X(BONE_LONGBOW, weapon("Bone Longbow", R, BOW, 42, 28, 0, 15, 7, 0, 0)) \
    X(GLUTTONOUS_SABATONS, armor("Gluttonous Sabatons", R, MAIL, FEET, 42, 10, 0, 7, 0, 0)) \
    X(LARDER_SLIPPERS, armor("Larder Slippers", R, CLOTH, FEET, 42, 0, 0, 6, 12, 7)) \
    X(BUTCHERS_BOOTS, armor("Butcher's Boots", R, LEATHER, FEET, 42, 0, 11, 7, 0, 0)) \
    X(COLDBRINGER_HELM, armor("Coldbringer Helm", R, MAIL, HEAD, 43, 11, 0, 8, 0, 0)) \
    X(COLDBRINGER_COWL, armor("Coldbringer Cowl", R, CLOTH, HEAD, 43, 0, 0, 7, 13, 8)) \
    X(FROSTBITTEN_MASK, armor("Frostbitten Mask", R, LEATHER, HEAD, 43, 0, 12, 8, 0, 0)) \
    X(ANTUSULS_GAUNTLETS, armor("Antu'sul's Gauntlets", R, MAIL, HANDS, 44, 11, 0, 7, 0, 0)) \
    X(SANDFURY_WRAPS, armor("Sandfury Wraps", R, CLOTH, HANDS, 44, 0, 0, 6, 13, 8)) \
    X(SCARABSKIN_GRIPS, armor("Scarabskin Grips", R, LEATHER, HANDS, 44, 0, 12, 7, 0, 0)) \
    X(MARTYRS_BREASTPLATE, armor("Martyr's Breastplate", R, MAIL, CHEST, 44, 13, 0, 9, 0, 0)) \
    X(THEKAS_ROBE, armor("Theka's Robe", R, CLOTH, CHEST, 44, 0, 0, 8, 15, 10)) \
    X(MARTYRS_VEST, armor("Martyr's Vest", R, LEATHER, CHEST, 44, 0, 14, 9, 0, 0)) \
    X(WITCH_DOCTOR_MACHETE, weapon("Witch Doctor Machete", R, SWORD, 44, 26, 11, 2, 7, 0, 0)) \
    X(WITCH_DOCTOR_STAFF, weapon("Witch Doctor Staff", R, STAFF, 44, 32, 0, 0, 7, 17, 9)) \
    X(VOODOO_LONGBOW, weapon("Voodoo Longbow", R, BOW, 44, 28, 0, 16, 7, 0, 0)) \
    X(HYDRA_LEGPLATES, armor("Hydra Legplates", R, MAIL, LEGS, 45, 12, 0, 8, 0, 0)) \
    X(GAHZ_RILLA_LEGGINGS, armor("Gahz'rilla Leggings", R, CLOTH, LEGS, 45, 0, 0, 7, 14, 9)) \
    X(HYDRAHIDE_PANTS, armor("Hydrahide Pants", R, LEATHER, LEGS, 45, 0, 13, 8, 0, 0)) \
    X(GUTCHEWER_SABATONS, armor("Gutchewer Sabatons", R, MAIL, FEET, 45, 11, 0, 7, 0, 0)) \
    X(SHADOWPRIEST_SLIPPERS, armor("Shadowpriest Slippers", R, CLOTH, FEET, 45, 0, 0, 6, 13, 8)) \
    X(SEZZ_ZIZS_BOOTS, armor("Sezz'ziz's Boots", R, LEATHER, FEET, 45, 0, 12, 7, 0, 0)) \
    X(RUUZLUS_AXE, weapon("Ruuzlu's Axe", R, AXE, 45, 25, 11, 2, 7, 0, 0)) \
    X(SANDFURY_SPIRE, weapon("Sandfury Spire", R, STAFF, 45, 32, 0, 0, 7, 17, 9)) \
    X(RUUZLUS_RECURVE, weapon("Ruuzlu's Recurve", R, BOW, 45, 28, 0, 16, 7, 0, 0)) \
    X(SANDSCALP_HELM, armor("Sandscalp Helm", R, MAIL, HEAD, 45, 12, 0, 8, 0, 0)) \
    X(CHIEFTAINS_HEADDRESS, armor("Chieftain's Headdress", R, CLOTH, HEAD, 45, 0, 0, 7, 14, 9)) \
    X(SANDSCALP_MASK, armor("Sandscalp Mask", R, LEATHER, HEAD, 45, 0, 13, 8, 0, 0)) \
    /* World drops, levels 41 to 45 */ \
    X(SANDSTORM_HELM, armor("Sandstorm Helm", U, MAIL, HEAD, 42, 9, 0, 6, 0, 0)) \
    X(MIRAGE_ROBE, armor("Mirage Robe", U, CLOTH, CHEST, 43, 0, 0, 6, 11, 7)) \
    X(DUNESHADOW_LEGGINGS, armor("Duneshadow Leggings", U, LEATHER, LEGS, 43, 0, 10, 7, 0, 0)) \
    X(SCORCHING_GREATSWORD, weapon("Scorching Greatsword", U, TWO_HANDED, 43, 36, 14, 0, 9, 0, 0)) \
    X(STAFF_OF_THE_DUNES, weapon("Staff of the Dunes", U, STAFF, 43, 32, 0, 0, 6, 14, 7)) \
    X(SIROCCO_LONGBOW, weapon("Sirocco Longbow", U, BOW, 43, 28, 0, 13, 6, 0, 0)) \
    X(SANDSTONE_BULWARK, shield("Sandstone Bulwark", U, 42, 4, 0, 7, 0, 0)) \
    /* Feralas, Desolace, Maraudon and Dire Maul: what their people and creatures carry */ \
    X(SATYR_HORN, junk("Satyr Horn", 95, 20)) \
    X(ELEMENTAL_EARTH, junk("Elemental Earth", 110, 20)) \
    X(WILDKIN_FEATHER, junk("Wildkin Feather", 90, 20)) \
    X(SPLINTERED_BARK, junk("Splintered Bark", 90, 20)) \
    X(FELCLOTH, junk("Felcloth", 140, 20)) \
    /* Feathermoon's smith */ \
    X(FEATHERMOON_GLAIVE, weapon("Feathermoon Glaive", C, SWORD, 45, 26)) \
    X(SENTINEL_HATCHET, weapon("Sentinel Hatchet", C, AXE, 45, 25)) \
    X(MOONSTEEL_MACE, weapon("Moonsteel Mace", C, MACE, 45, 27)) \
    X(FEATHERMOON_STAFF, weapon("Feathermoon Staff", C, STAFF, 45, 31)) \
    X(FEATHERMOON_WARBLADE, weapon("Feathermoon Warblade", C, TWO_HANDED, 45, 35)) \
    X(SENTINEL_LONGBOW, weapon("Sentinel Longbow", C, BOW, 45, 28)) \
    X(MOONSTEEL_RIFLE, weapon("Moonsteel Rifle", C, GUN, 45, 29)) \
    X(FEATHERMOON_KITE_SHIELD, shield("Feathermoon Kite Shield", C, 45)) \
    X(MOONSTEEL_HELM, armor("Moonsteel Helm", C, MAIL, HEAD, 45)) \
    X(MOONSTEEL_HAUBERK, armor("Moonsteel Hauberk", C, MAIL, CHEST, 45)) \
    X(MOONSTEEL_GAUNTLETS, armor("Moonsteel Gauntlets", C, MAIL, HANDS, 45)) \
    X(MOONSTEEL_LEGGINGS, armor("Moonsteel Leggings", C, MAIL, LEGS, 45)) \
    X(MOONSTEEL_BOOTS, armor("Moonsteel Boots", C, MAIL, FEET, 45)) \
    X(WILDWOOD_CAP, armor("Wildwood Cap", C, LEATHER, HEAD, 45)) \
    X(WILDWOOD_VEST, armor("Wildwood Vest", C, LEATHER, CHEST, 45)) \
    X(WILDWOOD_GLOVES, armor("Wildwood Gloves", C, LEATHER, HANDS, 45)) \
    X(WILDWOOD_PANTS, armor("Wildwood Pants", C, LEATHER, LEGS, 45)) \
    X(WILDWOOD_BOOTS, armor("Wildwood Boots", C, LEATHER, FEET, 45)) \
    X(MOONWEAVE_HOOD, armor("Moonweave Hood", C, CLOTH, HEAD, 45)) \
    X(MOONWEAVE_ROBE, armor("Moonweave Robe", C, CLOTH, CHEST, 45)) \
    X(MOONWEAVE_GLOVES, armor("Moonweave Gloves", C, CLOTH, HANDS, 45)) \
    X(MOONWEAVE_PANTS, armor("Moonweave Pants", C, CLOTH, LEGS, 45)) \
    X(MOONWEAVE_BOOTS, armor("Moonweave Boots", C, CLOTH, FEET, 45)) \
    /* Feralas and Desolace: quest rewards */ \
    X(HATECREST_GAUNTLETS, armor("Hatecrest Gauntlets", U, MAIL, HANDS, 45, 9, 0, 6, 0, 0)) \
    X(SIREN_GLOVES, armor("Siren Gloves", U, CLOTH, HANDS, 45, 0, 0, 5, 10, 6)) \
    X(TIDEHUNTER_GRIPS, armor("Tidehunter Grips", U, LEATHER, HANDS, 45, 0, 10, 6, 0, 0)) \
    X(CORAL_CUTLASS, weapon("Coral Cutlass", R, SWORD, 47, 26, 12, 2, 7, 0, 0)) \
    X(DREADMIST_STAFF, weapon("Dreadmist Staff", R, STAFF, 47, 32, 0, 0, 7, 18, 10)) \
    X(CORALWOOD_BOW, weapon("Coralwood Bow", R, BOW, 47, 28, 0, 17, 7, 0, 0)) \
    X(STRIDER_SABATONS, armor("Strider Sabatons", U, MAIL, FEET, 45, 9, 0, 6, 0, 0)) \
    X(SEASPRAY_SLIPPERS, armor("Seaspray Slippers", U, CLOTH, FEET, 45, 0, 0, 5, 10, 6)) \
    X(GIANTHIDE_BOOTS, armor("Gianthide Boots", U, LEATHER, FEET, 45, 0, 10, 6, 0, 0)) \
    X(ISILDIEN_LEGGUARDS, armor("Isildien Legguards", U, MAIL, LEGS, 46, 10, 0, 7, 0, 0)) \
    X(HIGHBORNE_TROUSERS, armor("Highborne Trousers", U, CLOTH, LEGS, 46, 0, 0, 6, 11, 7)) \
    X(RUINSTALKER_PANTS, armor("Ruinstalker Pants", U, LEATHER, LEGS, 46, 0, 11, 7, 0, 0)) \
    X(WARLORDS_HAUBERK, armor("Warlord's Hauberk", R, MAIL, CHEST, 47, 13, 0, 9, 0, 0)) \
    X(GORDUNNI_ROBE, armor("Gordunni Robe", R, CLOTH, CHEST, 47, 0, 0, 8, 15, 10)) \
    X(OGREHUNTER_VEST, armor("Ogrehunter Vest", R, LEATHER, CHEST, 47, 0, 14, 9, 0, 0)) \
    X(YETIHIDE_HAUBERK, armor("Yetihide Hauberk", U, MAIL, CHEST, 46, 11, 0, 7, 0, 0)) \
    X(FUR_LINED_ROBE, armor("Fur-Lined Robe", U, CLOTH, CHEST, 46, 0, 0, 6, 12, 8)) \
    X(RAGE_SCAR_VEST, armor("Rage Scar Vest", U, LEATHER, CHEST, 46, 0, 12, 7, 0, 0)) \
    X(GRIMTOTEM_HELM, armor("Grimtotem Helm", U, MAIL, HEAD, 47, 10, 0, 6, 0, 0)) \
    X(NATURALISTS_HOOD, armor("Naturalist's Hood", U, CLOTH, HEAD, 47, 0, 0, 5, 11, 7)) \
    X(TOTEMHIDE_CAP, armor("Totemhide Cap", U, LEATHER, HEAD, 47, 0, 11, 6, 0, 0)) \
    X(MOONKIN_MACE, weapon("Moonkin Mace", U, MACE, 46, 27, 9, 1, 6, 0, 0)) \
    X(FEATHERWOOD_STAFF, weapon("Featherwood Staff", U, STAFF, 46, 32, 0, 0, 6, 15, 8)) \
    X(TALONWOOD_BOW, weapon("Talonwood Bow", U, BOW, 46, 28, 0, 14, 6, 0, 0)) \
    X(DREAMBOUGH_SABATONS, armor("Dreambough Sabatons", R, MAIL, FEET, 48, 12, 0, 7, 0, 0)) \
    X(DREAMWEAVE_SLIPPERS, armor("Dreamweave Slippers", R, CLOTH, FEET, 48, 0, 0, 6, 14, 9)) \
    X(EMERALD_BOOTS, armor("Emerald Boots", R, LEATHER, FEET, 48, 0, 13, 7, 0, 0)) \
    X(WEBWARDEN_GAUNTLETS, armor("Webwarden Gauntlets", R, MAIL, HANDS, 49, 12, 0, 8, 0, 0)) \
    X(DARKWEAVE_GLOVES, armor("Darkweave Gloves", R, CLOTH, HANDS, 49, 0, 0, 7, 14, 9)) \
    X(SHADOWSILK_GRIPS, armor("Shadowsilk Grips", R, LEATHER, HANDS, 49, 0, 13, 8, 0, 0)) \
    X(FELVINE_LEGGUARDS, armor("Felvine Legguards", R, MAIL, LEGS, 50, 13, 0, 9, 0, 0)) \
    X(FELVINE_LEGGINGS, armor("Felvine Leggings", R, CLOTH, LEGS, 50, 0, 0, 8, 15, 10)) \
    X(FELVINE_PANTS, armor("Felvine Pants", R, LEATHER, LEGS, 50, 0, 14, 9, 0, 0)) \
    X(BREASTPLATE_OF_THE_SHENDRALAR, armor("Breastplate of the Shendralar", E, MAIL, CHEST, 50, 16, 0, 12, 0, 0)) \
    X(ROBE_OF_THE_SHENDRALAR, armor("Robe of the Shendralar", E, CLOTH, CHEST, 50, 0, 0, 11, 18, 13)) \
    X(VEST_OF_THE_SHENDRALAR, armor("Vest of the Shendralar", E, LEATHER, CHEST, 50, 0, 17, 12, 0, 0)) \
    X(KINGSLAYERS_HELM, armor("Kingslayer's Helm", R, MAIL, HEAD, 50, 13, 0, 9, 0, 0)) \
    X(KINGSLAYERS_COWL, armor("Kingslayer's Cowl", R, CLOTH, HEAD, 50, 0, 0, 8, 15, 10)) \
    X(KINGSLAYERS_MASK, armor("Kingslayer's Mask", R, LEATHER, HEAD, 50, 0, 14, 9, 0, 0)) \
    X(MAGRAM_SABATONS, armor("Magram Sabatons", U, MAIL, FEET, 46, 9, 0, 6, 0, 0)) \
    X(KODOHIDE_SLIPPERS, armor("Kodohide Slippers", U, CLOTH, FEET, 46, 0, 0, 5, 10, 6)) \
    X(CENTAUR_HIDE_BOOTS, armor("Centaur Hide Boots", U, LEATHER, FEET, 46, 0, 10, 6, 0, 0)) \
    X(CARVED_STONE_AXE, weapon("Carved Stone Axe", U, AXE, 47, 25, 10, 1, 6, 0, 0)) \
    X(WILLOW_BRANCH_STAFF, weapon("Willow Branch Staff", U, STAFF, 47, 32, 0, 0, 6, 15, 8)) \
    X(THERADRIC_HAND_CANNON, weapon("Theradric Hand Cannon", U, GUN, 47, 29, 0, 15, 6, 0, 0)) \
    X(SATYRBANE_LEGGUARDS, armor("Satyrbane Legguards", R, MAIL, LEGS, 47, 12, 0, 8, 0, 0)) \
    X(SATYRBANE_LEGGINGS, armor("Satyrbane Leggings", R, CLOTH, LEGS, 47, 0, 0, 7, 14, 9)) \
    X(SATYRBANE_PANTS, armor("Satyrbane Pants", R, LEATHER, LEGS, 47, 0, 13, 8, 0, 0)) \
    X(CELEBRIAN_GAUNTLETS, armor("Celebrian Gauntlets", R, MAIL, HANDS, 48, 12, 0, 7, 0, 0)) \
    X(CELEBRIAN_GLOVES, armor("Celebrian Gloves", R, CLOTH, HANDS, 48, 0, 0, 6, 14, 9)) \
    X(CELEBRIAN_GRIPS, armor("Celebrian Grips", R, LEATHER, HANDS, 48, 0, 13, 7, 0, 0)) \
    X(ZAETARS_LEGGUARDS, armor("Zaetar's Legguards", E, MAIL, LEGS, 49, 15, 0, 11, 0, 0)) \
    X(SEEDWEAVE_LEGGINGS, armor("Seedweave Leggings", E, CLOTH, LEGS, 49, 0, 0, 10, 17, 12)) \
    X(EARTHWARDEN_PANTS, armor("Earthwarden Pants", E, LEATHER, LEGS, 49, 0, 16, 11, 0, 0)) \
    /* Feralas's elite and rare */ \
    X(SHALZARUS_LEGPLATES, armor("Shalzaru's Legplates", R, MAIL, LEGS, 47, 12, 0, 8, 0, 0)) \
    X(NAGA_LORD_LEGGINGS, armor("Naga Lord Leggings", R, CLOTH, LEGS, 47, 0, 0, 7, 14, 9)) \
    X(DREADSCALE_PANTS, armor("Dreadscale Pants", R, LEATHER, LEGS, 47, 0, 13, 8, 0, 0)) \
    X(GRIZZLEGUT_GAUNTLETS, armor("Grizzlegut Gauntlets", R, MAIL, HANDS, 47, 11, 0, 7, 0, 0)) \
    X(GRIZZLEGUT_GLOVES, armor("Grizzlegut Gloves", R, CLOTH, HANDS, 47, 0, 0, 6, 13, 8)) \
    X(GRIZZLEGUT_GRIPS, armor("Grizzlegut Grips", R, LEATHER, HANDS, 47, 0, 12, 7, 0, 0)) \
    /* Maraudon and Dire Maul: boss drops */ \
    X(NOXIOUS_GAUNTLETS, armor("Noxious Gauntlets", R, MAIL, HANDS, 46, 11, 0, 7, 0, 0)) \
    X(TOXIC_WRAPS, armor("Toxic Wraps", R, CLOTH, HANDS, 46, 0, 0, 6, 13, 8)) \
    X(SLUDGE_COVERED_GRIPS, armor("Sludge-Covered Grips", R, LEATHER, HANDS, 46, 0, 12, 7, 0, 0)) \
    X(THORNSTRIDER_SABATONS, armor("Thornstrider Sabatons", R, MAIL, FEET, 46, 11, 0, 7, 0, 0)) \
    X(VINEWOVEN_SLIPPERS, armor("Vinewoven Slippers", R, CLOTH, FEET, 46, 0, 0, 6, 13, 8)) \
    X(RAZORLASH_BOOTS, armor("Razorlash Boots", R, LEATHER, FEET, 46, 0, 12, 7, 0, 0)) \
    X(VYLETONGUES_BLADE, weapon("Vyletongue's Blade", R, SWORD, 47, 26, 12, 2, 7, 0, 0)) \
    X(PUTRIDUS_STAFF, weapon("Putridus Staff", R, STAFF, 47, 32, 0, 0, 7, 18, 10)) \
    X(SATYRHORN_BOW, weapon("Satyrhorn Bow", R, BOW, 47, 28, 0, 17, 7, 0, 0)) \
    X(CELEBRAS_HELM, armor("Celebras' Helm", R, MAIL, HEAD, 47, 12, 0, 8, 0, 0)) \
    X(KEEPERS_COWL, armor("Keeper's Cowl", R, CLOTH, HEAD, 47, 0, 0, 7, 14, 9)) \
    X(GROVEWARDEN_MASK, armor("Grovewarden Mask", R, LEATHER, HEAD, 47, 0, 13, 8, 0, 0)) \
    X(GIZLOCKS_WRENCH, weapon("Gizlock's Wrench", R, MACE, 47, 27, 12, 2, 7, 0, 0)) \
    X(GIZLOCKS_STAFF, weapon("Gizlock's Staff", R, STAFF, 47, 32, 0, 0, 7, 18, 10)) \
    X(GIZLOCKS_HAND_CANNON, weapon("Gizlock's Hand Cannon", R, GUN, 47, 29, 0, 17, 7, 0, 0)) \
    X(ROCKSLIDE_LEGPLATES, armor("Rockslide Legplates", R, MAIL, LEGS, 48, 13, 0, 8, 0, 0)) \
    X(EARTHWEAVE_LEGGINGS, armor("Earthweave Leggings", R, CLOTH, LEGS, 48, 0, 0, 7, 15, 10)) \
    X(STONEHIDE_PANTS, armor("Stonehide Pants", R, LEATHER, LEGS, 48, 0, 14, 8, 0, 0)) \
    X(ROTGRIP_HAUBERK, armor("Rotgrip Hauberk", R, MAIL, CHEST, 48, 14, 0, 9, 0, 0)) \
    X(FENWEAVE_ROBE, armor("Fenweave Robe", R, CLOTH, CHEST, 48, 0, 0, 8, 16, 11)) \
    X(CROCSCALE_VEST, armor("Crocscale Vest", R, LEATHER, CHEST, 48, 0, 15, 9, 0, 0)) \
    X(PRINCESSS_GREATAXE, weapon("Princess's Greataxe", E, TWO_HANDED, 49, 36, 20, 0, 14, 0, 0)) \
    X(STAFF_OF_THERADRAS, weapon("Staff of Theradras", E, STAFF, 49, 32, 0, 0, 10, 22, 11)) \
    X(EARTHSONG_LONGBOW, weapon("Earthsong Longbow", E, BOW, 49, 28, 0, 20, 10, 0, 0)) \
    X(THORNHOOF_SABATONS, armor("Thornhoof Sabatons", R, MAIL, FEET, 47, 11, 0, 7, 0, 0)) \
    X(HELLFIRE_SLIPPERS, armor("Hellfire Slippers", R, CLOTH, FEET, 47, 0, 0, 6, 13, 8)) \
    X(FELHIDE_BOOTS, armor("Felhide Boots", R, LEATHER, FEET, 47, 0, 12, 7, 0, 0)) \
    X(HYDROSPAWN_GAUNTLETS, armor("Hydrospawn Gauntlets", R, MAIL, HANDS, 48, 12, 0, 7, 0, 0)) \
    X(TIDAL_GLOVES, armor("Tidal Gloves", R, CLOTH, HANDS, 48, 0, 0, 6, 14, 9)) \
    X(WATERLOGGED_GRIPS, armor("Waterlogged Grips", R, LEATHER, HANDS, 48, 0, 13, 7, 0, 0)) \
    X(LETHTENDRISS_HELM, armor("Lethtendris's Helm", R, MAIL, HEAD, 48, 13, 0, 8, 0, 0)) \
    X(SHADOWWEAVE_COWL, armor("Shadowweave Cowl", R, CLOTH, HEAD, 48, 0, 0, 7, 15, 10)) \
    X(WEBSPUN_MASK, armor("Webspun Mask", R, LEATHER, HEAD, 48, 0, 14, 8, 0, 0)) \
    X(WILDSHAPERS_LEGGUARDS, armor("Wildshaper's Legguards", R, MAIL, LEGS, 49, 13, 0, 9, 0, 0)) \
    X(WILDWEAVE_LEGGINGS, armor("Wildweave Leggings", R, CLOTH, LEGS, 49, 0, 0, 8, 15, 10)) \
    X(WILDHIDE_PANTS, armor("Wildhide Pants", R, LEATHER, LEGS, 49, 0, 14, 9, 0, 0)) \
    X(WARPWOOD_HAUBERK, armor("Warpwood Hauberk", R, MAIL, CHEST, 48, 14, 0, 9, 0, 0)) \
    X(BARKWEAVE_ROBE, armor("Barkweave Robe", R, CLOTH, CHEST, 48, 0, 0, 8, 16, 11)) \
    X(IRONBARK_VEST, armor("Ironbark Vest", R, LEATHER, CHEST, 48, 0, 15, 9, 0, 0)) \
    X(IMMOL_THARS_CLAW, weapon("Immol'thar's Claw", R, AXE, 49, 25, 12, 2, 8, 0, 0)) \
    X(DEMONIC_STAFF, weapon("Demonic Staff", R, STAFF, 49, 32, 0, 0, 8, 19, 10)) \
    X(FEL_LONGBOW, weapon("Fel Longbow", R, BOW, 49, 28, 0, 18, 8, 0, 0)) \
    X(TORTHELDRINS_BLADE, weapon("Tortheldrin's Blade", R, SWORD, 50, 26, 12, 2, 8, 0, 0)) \
    X(HIGHBORNE_STAFF, weapon("Highborne Staff", R, STAFF, 50, 32, 0, 0, 8, 19, 10)) \
    X(PRINCES_LONGBOW, weapon("Prince's Longbow", R, BOW, 50, 28, 0, 18, 8, 0, 0)) \
    X(OBSERVERS_GAUNTLETS, armor("Observer's Gauntlets", R, MAIL, HANDS, 49, 12, 0, 8, 0, 0)) \
    X(CHO_RUSHS_GLOVES, armor("Cho'Rush's Gloves", R, CLOTH, HANDS, 49, 0, 0, 7, 14, 9)) \
    X(OBSERVERS_GRIPS, armor("Observer's Grips", R, LEATHER, HANDS, 49, 0, 13, 8, 0, 0)) \
    X(GORDOKS_GREATAXE, weapon("Gordok's Greataxe", E, TWO_HANDED, 50, 36, 21, 0, 14, 0, 0)) \
    X(STAFF_OF_THE_OGRE_KING, weapon("Staff of the Ogre King", E, STAFF, 50, 32, 0, 0, 10, 22, 11)) \
    X(GORDOKS_LONGBOW, weapon("Gordok's Longbow", E, BOW, 50, 28, 0, 20, 10, 0, 0)) \
    /* World drops, levels 46 to 50 */ \
    X(THUNDERHEAD_HELM, armor("Thunderhead Helm", U, MAIL, HEAD, 47, 10, 0, 6, 0, 0)) \
    X(MOONSHADOW_ROBE, armor("Moonshadow Robe", U, CLOTH, CHEST, 48, 0, 0, 6, 13, 9)) \
    X(WILDHEART_LEGGINGS, armor("Wildheart Leggings", U, LEATHER, LEGS, 48, 0, 12, 7, 0, 0)) \
    X(TWILIGHT_GREATSWORD, weapon("Twilight Greatsword", U, TWO_HANDED, 48, 36, 16, 0, 10, 0, 0)) \
    X(STAFF_OF_THE_ANCIENTS, weapon("Staff of the Ancients", U, STAFF, 48, 32, 0, 0, 7, 15, 8)) \
    X(FERALAS_LONGBOW, weapon("Feralas Longbow", U, BOW, 48, 28, 0, 15, 7, 0, 0)) \
    X(HIGHBORNE_BULWARK, shield("Highborne Bulwark", U, 47, 4, 0, 7, 0, 0))

namespace gw
{

#define GW_ITEM_ID(id, definition) id,

enum class item_id : uint16_t
{
    GW_ITEM_LIST(GW_ITEM_ID)
    COUNT
};

#undef GW_ITEM_ID

}

#endif
