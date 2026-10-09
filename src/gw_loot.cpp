#include "gw_loot.h"

#include "gw_character.h"
#include "gw_enemies.h"
#include "gw_item_sets.h"
#include "gw_types.h"

namespace gw
{

namespace
{
    using i = item_id;

    struct loot_entry
    {
        item_id item;
        uint8_t chance;     // percent
        uint8_t min_count;
        uint8_t max_count;
    };

    struct loot_table
    {
        loot_entry entries[3];
        bool money;             // beasts carry no coins
        item_id choice[3];      // bosses always drop one of these
    };

    constexpr loot_entry none = { i::NONE, 0, 0, 0 };

    // Indexed by enemy_def::loot_table.
    constexpr loot_table tables[] = {
        { { none, none, none }, false, { i::NONE, i::NONE, i::NONE } },
        // 1 wolves
        { { { i::BROKEN_FANG, 50, 1, 1 }, none, none }, false, { i::NONE, i::NONE, i::NONE } },
        // 2 kobold vermin
        { { { i::DIM_CANDLE, 40, 1, 1 }, { i::LINEN_CLOTH, 20, 1, 1 }, none }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 3 defias thug
        { { { i::LINEN_CLOTH, 40, 1, 2 }, { i::TOUGH_JERKY, 10, 1, 1 }, { i::MINOR_HEALING_POTION, 5, 1, 1 } },
          true, { i::NONE, i::NONE, i::NONE } },
        // 4 spiders
        { { { i::SPIDER_SILK, 50, 1, 1 }, none, none }, false, { i::NONE, i::NONE, i::NONE } },
        // 5 kobold tunneler
        { { { i::DIM_CANDLE, 40, 1, 1 }, { i::LINEN_CLOTH, 25, 1, 1 }, { i::MINOR_HEALING_POTION, 5, 1, 1 } },
          true, { i::NONE, i::NONE, i::NONE } },
        // 6 murlocs
        { { { i::MURLOC_SCALE, 50, 1, 1 }, none, none }, true, { i::NONE, i::NONE, i::NONE } },
        // 7 boars
        { { { i::CHIPPED_TUSK, 50, 1, 1 }, none, none }, false, { i::NONE, i::NONE, i::NONE } },
        // 8 Princess
        { { { i::CHIPPED_TUSK, 100, 2, 3 }, none, none }, false, { i::NONE, i::NONE, i::NONE } },
        // 9 gnolls
        { { { i::GNOLL_PELT, 45, 1, 1 }, { i::LINEN_CLOTH, 20, 1, 1 }, { i::LESSER_HEALING_POTION, 4, 1, 1 } },
          true, { i::NONE, i::NONE, i::NONE } },
        // 10 Hogger
        { { { i::GNOLL_PELT, 100, 1, 2 }, { i::LESSER_HEALING_POTION, 50, 1, 2 }, none }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 11 harvest watchers
        { { { i::RUSTED_GEAR, 50, 1, 1 }, none, none }, false, { i::NONE, i::NONE, i::NONE } },
        // 12 Westfall defias
        { { { i::WOOL_CLOTH, 35, 1, 2 }, { i::HAUNCH_OF_MEAT, 8, 1, 1 }, { i::LESSER_HEALING_POTION, 5, 1, 1 } },
          true, { i::NONE, i::NONE, i::NONE } },
        // 13 Deadmines defias
        { { { i::WOOL_CLOTH, 45, 1, 2 }, { i::MUTTON_CHOP, 8, 1, 1 }, { i::HEALING_POTION, 5, 1, 1 } }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 14 goblins
        { { { i::COPPER_BOLTS, 45, 1, 1 }, { i::WOOL_CLOTH, 20, 1, 1 }, { i::HEALING_POTION, 5, 1, 1 } }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 15 Sneed
        { { { i::COPPER_BOLTS, 100, 2, 3 }, { i::HEALING_POTION, 50, 1, 2 }, none }, true,
          { i::BUZZER_BLADE, i::TASKMASTER_AXE, i::GOLD_PLATED_BUCKLER } },
        // 16 Edwin VanCleef
        { { { i::HEALING_POTION, 100, 2, 3 }, none, none }, true,
          { i::CRUEL_BARB, i::SMITES_HAMMER, i::CORSAIR_OVERSHIRT } },
        // 17 Stockade prisoners
        { { { i::WOOL_CLOTH, 45, 1, 2 }, { i::MUTTON_CHOP, 8, 1, 1 }, { i::HEALING_POTION, 6, 1, 1 } }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 18 Targorr the Dread
        { { { i::HEALING_POTION, 60, 1, 2 }, none, none }, true,
          { i::LUCINE_LONGSWORD, i::KNUCKLE_WRAPS, i::SHACKLED_MITTS } },
        // 19 Kam Deepfury
        { { { i::HEALING_POTION, 60, 1, 2 }, none, none }, true,
          { i::DEEPFURY_SHIELD, i::EMBERWEAVE_ROBE, i::DARK_IRON_RIFLE } },
        // 20 Bazil Thredd
        { { { i::HEALING_POTION, 100, 2, 3 }, none, none }, true,
          { i::THREDDS_DUSKBLADE, i::SMOKEWEAVE_PANTS, i::SHADOWHIDE_BOOTS } },
        // 21 Blackrock orcs
        { { { i::WOOL_CLOTH, 40, 1, 2 }, { i::MUTTON_CHOP, 8, 1, 1 }, { i::HEALING_POTION, 5, 1, 1 } }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 22 Bellygrub
        { { { i::CHIPPED_TUSK, 100, 2, 3 }, none, none }, false, { i::NONE, i::NONE, i::NONE } },
        // 23 Ribchaser
        { { { i::GNOLL_PELT, 100, 1, 2 }, { i::HEALING_POTION, 50, 1, 1 }, none }, true,
          { i::RIBCHASERS_CLEAVER, i::GNOLLBONE_STAFF, i::RIBCHASERS_LONGBOW } },
        // 24 Gath'Ilzogg
        { { { i::HEALING_POTION, 100, 1, 2 }, none, none }, true,
          { i::GATHS_WARMAUL, i::SHADOWCASTER_ROBE, i::BLACKROCK_HUNTING_BOW } },
        // 25 Redridge gnolls
        { { { i::GNOLL_PELT, 45, 1, 1 }, { i::WOOL_CLOTH, 25, 1, 1 }, { i::HEALING_POTION, 4, 1, 1 } }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 26 dire wolves and worgs
        { { { i::DIRE_WOLF_PELT, 50, 1, 1 }, none, none }, false,
          { i::NONE, i::NONE, i::NONE } },
        // 27 venom web spiders
        { { { i::SPIDER_SILK, 50, 1, 2 }, none, none }, false, { i::NONE, i::NONE, i::NONE } },
        // 28 Nightbane worgen
        { { { i::WORGEN_FANG, 45, 1, 1 }, { i::SILK_CLOTH, 25, 1, 1 }, { i::HEALING_POTION, 4, 1, 1 } }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 29 skeletons
        { { { i::BONE_FRAGMENTS, 45, 1, 2 }, { i::SILK_CLOTH, 20, 1, 1 }, { i::HEALING_POTION, 4, 1, 1 } }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 30 ghouls
        { { { i::PUTRID_CLAW, 45, 1, 1 }, { i::BONE_FRAGMENTS, 20, 1, 1 }, { i::HEALING_POTION, 4, 1, 1 } }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 31 Splinter Fist ogres
        { { { i::OGRE_TOOTH, 45, 1, 1 }, { i::SILK_CLOTH, 25, 1, 2 }, { i::WILD_HOG_SHANK, 8, 1, 1 } }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 32 Mor'Ladim
        { { { i::BONE_FRAGMENTS, 100, 2, 3 }, { i::GREATER_HEALING_POTION, 60, 1, 2 }, none }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 33 Stalvan Mistmantle
        { { { i::SILK_CLOTH, 100, 2, 3 }, { i::GREATER_HEALING_POTION, 60, 1, 2 }, none }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 34 Morbent Fel
        { { { i::BONE_FRAGMENTS, 100, 2, 3 }, { i::GREATER_HEALING_POTION, 100, 1, 2 }, none }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 35 Stitches
        { { { i::PUTRID_CLAW, 100, 2, 3 }, { i::GREATER_HEALING_POTION, 100, 2, 2 }, none }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 36 Shadowfang worgen
        { { { i::WORGEN_FANG, 45, 1, 1 }, { i::SILK_CLOTH, 30, 1, 2 }, { i::GREATER_HEALING_POTION, 4, 1, 1 } },
          true, { i::NONE, i::NONE, i::NONE } },
        // 37 Shadowfang undead
        { { { i::BONE_FRAGMENTS, 40, 1, 2 }, { i::SILK_CLOTH, 30, 1, 2 }, { i::GREATER_HEALING_POTION, 4, 1, 1 } },
          true, { i::NONE, i::NONE, i::NONE } },
        // 38 Rethilgore
        { { { i::GREATER_HEALING_POTION, 60, 1, 2 }, none, none }, true,
          { i::WOLFGUARD_GAUNTLETS, i::SOUL_DRAIN_WRAPS, i::RETHILGORES_GRIPS } },
        // 39 Razorclaw the Butcher
        { { { i::GREATER_HEALING_POTION, 60, 1, 2 }, none, none }, true,
          { i::BUTCHERS_SLICER, i::BUTCHERS_APRON, i::RAZORCLAW_LEGGINGS } },
        // 40 Baron Silverlaine
        { { { i::GREATER_HEALING_POTION, 60, 1, 2 }, none, none }, true,
          { i::SILVERLAINES_HELM, i::BARONS_CIRCLET, i::MOONRAGE_HOOD } },
        // 41 Commander Springvale
        { { { i::GREATER_HEALING_POTION, 60, 1, 2 }, none, none }, true,
          { i::COMMANDERS_CREST, i::CHAPLAINS_VESTMENTS, i::WOLFSKIN_JERKIN } },
        // 42 Odo the Blindwatcher
        { { { i::GREATER_HEALING_POTION, 60, 1, 2 }, none, none }, true,
          { i::BLINDWATCHER_GREATAXE, i::ODOS_LEY_STAFF, i::BLINDSIGHT_BOW } },
        // 43 Archmage Arugal
        { { { i::GREATER_HEALING_POTION, 100, 2, 3 }, none, none }, true,
          { i::SHADOWFANG, i::ROBE_OF_ARUGAL, i::WORGEN_HIDE_LEGGINGS } },
        // 44 crocolisks
        { { { i::CROCOLISK_SCALE, 50, 1, 1 }, none, none }, false, { i::NONE, i::NONE, i::NONE } },
        // 45 raptors
        { { { i::RAPTOR_TALON, 50, 1, 1 }, none, none }, false, { i::NONE, i::NONE, i::NONE } },
        // 46 Mosshide gnolls
        { { { i::GNOLL_PELT, 45, 1, 1 }, { i::MAGEWEAVE_CLOTH, 20, 1, 1 }, { i::GREATER_HEALING_POTION, 4, 1, 1 } },
          true, { i::NONE, i::NONE, i::NONE } },
        // 47 Bluegill and Blindlight murlocs
        { { { i::MURLOC_SCALE, 50, 1, 1 }, { i::MAGEWEAVE_CLOTH, 10, 1, 1 }, none }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 48 Dark Iron dwarves
        { { { i::MAGEWEAVE_CLOTH, 35, 1, 2 }, { i::DWARVEN_MILD, 8, 1, 1 }, { i::GREATER_HEALING_POTION, 5, 1, 1 } },
          true, { i::NONE, i::NONE, i::NONE } },
        // 49 Dragonmaw orcs
        { { { i::MAGEWEAVE_CLOTH, 40, 1, 2 }, { i::WILD_HOG_SHANK, 8, 1, 1 }, { i::GREATER_HEALING_POTION, 5, 1, 1 } },
          true, { i::NONE, i::NONE, i::NONE } },
        // 50 Sarltooth
        { { { i::RAPTOR_TALON, 100, 2, 3 }, { i::GREATER_HEALING_POTION, 50, 1, 1 }, none }, false,
          { i::NONE, i::NONE, i::NONE } },
        // 51 Balgaras the Foul
        { { { i::MAGEWEAVE_CLOTH, 100, 2, 3 }, { i::GREATER_HEALING_POTION, 60, 1, 2 }, none }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 52 Nek'rosh
        { { { i::GREATER_HEALING_POTION, 100, 1, 2 }, { i::MAGEWEAVE_CLOTH, 100, 2, 3 }, none }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 53 naga
        { { { i::NAGA_SCALE, 45, 1, 1 }, { i::MAGEWEAVE_CLOTH, 25, 1, 1 }, { i::GREATER_HEALING_POTION, 4, 1, 1 } },
          true, { i::NONE, i::NONE, i::NONE } },
        // 54 Twilight cultists
        { { { i::MAGEWEAVE_CLOTH, 45, 1, 2 }, { i::MOONBERRY_JUICE, 8, 1, 1 }, { i::GREATER_HEALING_POTION, 5, 1, 1 } },
          true, { i::NONE, i::NONE, i::NONE } },
        // 55 Ghamoo-ra
        { { { i::GREATER_HEALING_POTION, 60, 1, 2 }, none, none }, false,
          { i::TURTLE_SHELL_SHIELD, i::GHAMOO_RA_WRAPS, i::SNAPJAW_GLOVES } },
        // 56 Lady Sarevess
        { { { i::GREATER_HEALING_POTION, 60, 1, 2 }, { i::NAGA_SCALE, 100, 1, 2 }, none }, true,
          { i::STRIKE_OF_THE_HYDRA, i::ROBE_OF_THE_DEEPS, i::NAGA_SCALE_JERKIN } },
        // 57 Gelihast
        { { { i::GREATER_HEALING_POTION, 60, 1, 2 }, { i::MURLOC_SCALE, 100, 1, 2 }, none }, true,
          { i::MURKBLOOD_HELM, i::GELIHASTS_HOOD, i::FISHSCALE_CAP } },
        // 58 Twilight Lord Kelris
        { { { i::GREATER_HEALING_POTION, 60, 1, 2 }, none, none }, true,
          { i::TWILIGHT_CLEAVER, i::ROD_OF_THE_SLEEPWALKER, i::TWILIGHT_LONGBOW } },
        // 59 Aku'mai
        { { { i::GREATER_HEALING_POTION, 100, 2, 3 }, none, none }, false,
          { i::AKU_MAIS_FANG, i::STAFF_OF_THE_DEEP_MOTHER, i::ABYSSAL_LONGBOW } },
        // 60 troggs
        { { { i::TROGG_STONE_TOOTH, 45, 1, 1 }, { i::MAGEWEAVE_CLOTH, 20, 1, 1 },
            { i::GREATER_HEALING_POTION, 4, 1, 1 } }, true, { i::NONE, i::NONE, i::NONE } },
        // 61 leper gnomes
        { { { i::GREASY_COG, 40, 1, 1 }, { i::MAGEWEAVE_CLOTH, 30, 1, 2 }, { i::GREATER_HEALING_POTION, 5, 1, 1 } },
          true, { i::NONE, i::NONE, i::NONE } },
        // 62 robots
        { { { i::GREASY_COG, 55, 1, 2 }, none, none }, false, { i::NONE, i::NONE, i::NONE } },
        // 63 slimes
        { { { i::GLOWING_SLUDGE, 50, 1, 1 }, none, none }, false, { i::NONE, i::NONE, i::NONE } },
        // 64 Grubbis
        { { { i::GREATER_HEALING_POTION, 60, 1, 2 }, { i::TROGG_STONE_TOOTH, 100, 1, 2 }, none }, true,
          { i::TROGGSTONE_HELM, i::CAVERNDEEP_COWL, i::BURROWER_HOOD } },
        // 65 Viscous Fallout
        { { { i::GLOWING_SLUDGE, 100, 2, 3 }, none, none }, false,
          { i::FALLOUT_LEGPLATES, i::RADIANT_LEGGINGS, i::SLUDGE_SOAKED_PANTS } },
        // 66 Electrocutioner 6000
        { { { i::GREASY_COG, 100, 2, 3 }, none, none }, false,
          { i::ELECTROCUTIONER_LEG, i::ARC_SPARK_STAFF, i::STATIC_LONGBOW } },
        // 67 Crowd Pummeler 9-60
        { { { i::GREASY_COG, 100, 2, 3 }, none, none }, false,
          { i::MANUAL_CROWD_PUMMELER, i::OSCILLATING_POWER_ROBE, i::GEAR_STUDDED_JERKIN } },
        // 68 Mekgineer Thermaplugg
        { { { i::GREATER_HEALING_POTION, 100, 2, 3 }, none, none }, true,
          { i::THERMAPLUGGS_LEFT_ARM, i::MEKGINEERS_SPARK_STAFF, i::THERMAPLUGGS_BLUNDERBUSS } },
        // 69 bears, mountain lions and hounds
        { { { i::THICK_FUR, 50, 1, 1 }, none, none }, false, { i::NONE, i::NONE, i::NONE } },
        // 70 Syndicate
        { { { i::SILK_CLOTH, 40, 1, 2 }, { i::GOLDENBARK_APPLE, 8, 1, 1 }, { i::SUPERIOR_HEALING_POTION, 4, 1, 1 } },
          true, { i::NONE, i::NONE, i::NONE } },
        // 71 Crushridge ogres
        { { { i::SILK_CLOTH, 35, 1, 2 }, { i::GOLDENBARK_APPLE, 10, 1, 1 }, { i::SUPERIOR_HEALING_POTION, 5, 1, 1 } },
          true, { i::NONE, i::NONE, i::NONE } },
        // 72 Forsaken
        { { { i::SILK_CLOTH, 40, 1, 2 }, { i::MORNING_GLORY_DEW, 8, 1, 1 }, { i::SUPERIOR_HEALING_POTION, 4, 1, 1 } },
          true, { i::NONE, i::NONE, i::NONE } },
        // 73 yetis
        { { { i::YETI_HORN, 45, 1, 1 }, { i::THICK_FUR, 30, 1, 1 }, none }, false, { i::NONE, i::NONE, i::NONE } },
        // 74 Torn Fin murlocs
        { { { i::MURLOC_SCALE, 50, 1, 1 }, { i::SILK_CLOTH, 15, 1, 1 }, none }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 75 Gravis Slipknot
        { { { i::SILK_CLOTH, 100, 2, 3 }, { i::SUPERIOR_HEALING_POTION, 60, 1, 2 }, none }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 76 Bloodfang
        { { { i::YETI_HORN, 100, 2, 3 }, { i::THICK_FUR, 100, 1, 2 }, none }, false,
          { i::NONE, i::NONE, i::NONE } },
        // 77 Scarlet Crusade
        { { { i::SCARLET_INSIGNIA, 40, 1, 1 }, { i::SILK_CLOTH, 35, 1, 2 }, { i::SUPERIOR_HEALING_POTION, 5, 1, 1 } },
          true, { i::NONE, i::NONE, i::NONE } },
        // 78 spirits
        { { { i::GHOSTLY_ECTOPLASM, 50, 1, 1 }, none, none }, false, { i::NONE, i::NONE, i::NONE } },
        // 79 Interrogator Vishas
        { { { i::SUPERIOR_HEALING_POTION, 60, 1, 2 }, { i::SCARLET_INSIGNIA, 100, 1, 2 }, none }, true,
          { i::INTERROGATORS_HELM, i::HOOD_OF_CONFESSION, i::TORTURERS_MASK } },
        // 80 Azshir the Sleepless
        { { { i::GHOSTLY_ECTOPLASM, 100, 2, 3 }, none, none }, false,
          { i::SLEEPLESS_GAUNTLETS, i::GHOSTSHROUD_WRAPS, i::GHOSTWALKER_GRIPS } },
        // 81 Bloodmage Thalnos
        { { { i::SUPERIOR_HEALING_POTION, 60, 1, 2 }, { i::SCARLET_INSIGNIA, 100, 1, 2 }, none }, true,
          { i::THALNOS_CLEAVER, i::STAFF_OF_THE_BLOODMAGE, i::FLAMESPIKE_BOW } },
        // 82 Ironspine
        { { { i::SUPERIOR_HEALING_POTION, 60, 1, 2 }, none, none }, false,
          { i::IRONSPINES_RIBCAGE, i::SHROUD_OF_THE_OSSUARY, i::BONE_STUDDED_JERKIN } },
        // 83 Houndmaster Loksey
        { { { i::SUPERIOR_HEALING_POTION, 60, 1, 2 }, { i::SCARLET_INSIGNIA, 100, 1, 2 }, none }, true,
          { i::HOUNDMASTERS_SABATONS, i::KENNELKEEPERS_SLIPPERS, i::HOUNDMASTERS_BOOTS } },
        // 84 Arcanist Doan
        { { { i::SUPERIOR_HEALING_POTION, 100, 2, 3 }, none, none }, true,
          { i::HYPNOTIC_BLADE, i::ILLUSIONARY_ROD, i::SCARLET_LONGBOW } },
        // 85 tigers, panthers and gorillas
        { { { i::THICK_FUR, 50, 1, 1 }, none, none }, false, { i::NONE, i::NONE, i::NONE } },
        // 86 raptors
        { { { i::RAPTOR_CLAW, 50, 1, 1 }, { i::THICK_FUR, 20, 1, 1 }, none }, false, { i::NONE, i::NONE, i::NONE } },
        // 87 Bloodscalp trolls
        { { { i::TROLL_TUSK, 40, 1, 1 }, { i::MAGEWEAVE_CLOTH, 35, 1, 2 }, { i::SUPERIOR_HEALING_POTION, 5, 1, 1 } },
          true, { i::NONE, i::NONE, i::NONE } },
        // 88 Bloodsail pirates
        { { { i::MAGEWEAVE_CLOTH, 40, 1, 2 }, { i::MORNING_GLORY_DEW, 8, 1, 1 }, { i::SUPERIOR_HEALING_POTION, 5, 1, 1 } },
          true, { i::NONE, i::NONE, i::NONE } },
        // 89 the Scarlet Crusade in the Armory and the Cathedral
        { { { i::SCARLET_INSIGNIA, 40, 1, 1 }, { i::MAGEWEAVE_CLOTH, 35, 1, 2 }, { i::SUPERIOR_HEALING_POTION, 5, 1, 1 } },
          true, { i::NONE, i::NONE, i::NONE } },
        // 90 King Bangalash
        { { { i::THICK_FUR, 100, 2, 3 }, none, none }, false,
          { i::SABERTOOTH_GAUNTLETS, i::SILVERSTRIPE_GLOVES, i::BANGALASHS_GRIPS } },
        // 91 Fleet Master Firallon
        { { { i::MAGEWEAVE_CLOTH, 100, 2, 3 }, { i::SUPERIOR_HEALING_POTION, 60, 1, 2 }, none }, true,
          { i::CAPTAINS_SABATONS, i::SEAFARERS_SLIPPERS, i::FIRALLONS_BOOTS } },
        // 92 Mogh the Undying
        { { { i::TROLL_TUSK, 100, 1, 2 }, { i::MAGEWEAVE_CLOTH, 100, 1, 2 }, none }, true,
          { i::MOGHS_LEGPLATES, i::LEGGINGS_OF_THE_UNDYING, i::VOODOO_BREECHES } },
        // 93 Herod
        { { { i::SUPERIOR_HEALING_POTION, 60, 1, 2 }, { i::SCARLET_INSIGNIA, 100, 1, 2 }, none }, true,
          { i::HERODS_BREASTPLATE, i::CHAMPIONS_ROBE, i::BLOODWHIRL_TUNIC } },
        // 94 High Inquisitor Fairbanks
        { { { i::SUPERIOR_HEALING_POTION, 60, 1, 2 }, { i::SCARLET_INSIGNIA, 100, 1, 2 }, none }, true,
          { i::INQUISITORS_HELM, i::HOOD_OF_PENANCE, i::MASK_OF_ATONEMENT } },
        // 95 Scarlet Commander Mograine
        { { { i::SUPERIOR_HEALING_POTION, 60, 1, 2 }, { i::SCARLET_INSIGNIA, 100, 1, 2 }, none }, true,
          { i::MOGRAINES_MIGHT, i::STAFF_OF_THE_COMMANDER, i::CRUSADERS_LONGBOW } },
        // 96 High Inquisitor Whitemane
        { { { i::SUPERIOR_HEALING_POTION, 100, 2, 3 }, none, none }, true,
          { i::GAUNTLETS_OF_DIVINITY, i::WHITEMANES_GLOVES, i::GRIPS_OF_RESURRECTION } },
        // 97 Wastewander bandits
        { { { i::RUNECLOTH, 35, 1, 2 }, { i::SPARKLING_DESERT_WATER, 8, 1, 1 }, { i::MAJOR_HEALING_POTION, 5, 1, 1 } },
          true, { i::NONE, i::NONE, i::NONE } },
        // 98 scorpids
        { { { i::SCORPID_STINGER, 50, 1, 1 }, none, none }, false, { i::NONE, i::NONE, i::NONE } },
        // 99 hyenas, basilisks and boars
        { { { i::THICK_FUR, 45, 1, 1 }, none, none }, false, { i::NONE, i::NONE, i::NONE } },
        // 100 Dunemaul ogres
        { { { i::OGRE_TOOTH, 40, 1, 1 }, { i::RUNECLOTH, 35, 1, 2 }, { i::MAJOR_HEALING_POTION, 5, 1, 1 } },
          true, { i::NONE, i::NONE, i::NONE } },
        // 101 Southsea pirates
        { { { i::RUNECLOTH, 40, 1, 2 }, { i::SPARKLING_DESERT_WATER, 8, 1, 1 }, { i::MAJOR_HEALING_POTION, 5, 1, 1 } },
          true, { i::NONE, i::NONE, i::NONE } },
        // 102 Sandfury trolls
        { { { i::TROLL_TUSK, 40, 1, 1 }, { i::RUNECLOTH, 35, 1, 2 }, { i::MAJOR_HEALING_POTION, 5, 1, 1 } },
          true, { i::NONE, i::NONE, i::NONE } },
        // 103 Galak centaurs
        { { { i::RUNECLOTH, 30, 1, 2 }, { i::THICK_FUR, 25, 1, 1 }, { i::MAJOR_HEALING_POTION, 5, 1, 1 } },
          true, { i::NONE, i::NONE, i::NONE } },
        // 104 Razorfen quilboar
        { { { i::QUILBOAR_TUSK, 40, 1, 1 }, { i::RUNECLOTH, 30, 1, 2 }, { i::MAJOR_HEALING_POTION, 5, 1, 1 } },
          true, { i::NONE, i::NONE, i::NONE } },
        // 105 the Scourge in the Downs
        { { { i::BONE_FRAGMENTS, 40, 1, 2 }, { i::RUNECLOTH, 30, 1, 2 }, { i::MAJOR_HEALING_POTION, 5, 1, 1 } },
          true, { i::NONE, i::NONE, i::NONE } },
        // 106 tomb fiends and zombies
        { { { i::BONE_FRAGMENTS, 40, 1, 2 }, none, none }, false, { i::NONE, i::NONE, i::NONE } },
        // 107 Caliph Scorpidsting
        { { { i::RUNECLOTH, 100, 2, 3 }, { i::MAJOR_HEALING_POTION, 60, 1, 2 }, none }, true,
          { i::SCORPIDSTING_GAUNTLETS, i::CALIPHS_GLOVES, i::SANDSTALKER_GRIPS } },
        // 108 Andre Firebeard
        { { { i::RUNECLOTH, 100, 2, 3 }, { i::MAJOR_HEALING_POTION, 60, 1, 2 }, none }, true,
          { i::ANDRES_SABATONS, i::CAPTAINS_SLIPPERS, i::FIREBEARD_BOOTS } },
        // 109 Omgorn the Lost
        { { { i::OGRE_TOOTH, 100, 1, 2 }, { i::RUNECLOTH, 100, 2, 3 }, none }, true,
          { i::OMGORNS_LEGPLATES, i::LEGGINGS_OF_THE_LOST, i::OGRE_HIDE_BREECHES } },
        // 110 Aggem Thorncurse
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::QUILBOAR_TUSK, 100, 1, 2 }, none }, true,
          { i::THORNCURSE_GAUNTLETS, i::THORNWEAVE_GLOVES, i::BRAMBLEHIDE_GRIPS } },
        // 111 Death Speaker Jargba
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::QUILBOAR_TUSK, 100, 1, 2 }, none }, true,
          { i::DEATH_SPEAKER_HELM, i::JARGBAS_COWL, i::SPEAKERS_MASK } },
        // 112 Overlord Ramtusk
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::QUILBOAR_TUSK, 100, 1, 2 }, none }, true,
          { i::RAMTUSKS_CLEAVER, i::RAMSTAFF, i::TUSKER_LONGBOW } },
        // 113 Agathelos the Raging
        { { { i::THICK_FUR, 100, 2, 3 }, none, none }, false,
          { i::STAMPEDE_SABATONS, i::AGAM_AR_SLIPPERS, i::BOARHIDE_BOOTS } },
        // 114 Charlga Razorflank
        { { { i::MAJOR_HEALING_POTION, 100, 2, 3 }, none, none }, true,
          { i::RAZORFLANK_HAUBERK, i::CHARLGAS_ROBE, i::CRONES_VEST } },
        // 115 Tuten'kash
        { { { i::SPIDER_SILK, 100, 2, 3 }, none, none }, false,
          { i::SILK_WRAPPED_LEGGUARDS, i::SPIDERSILK_LEGGINGS, i::FIENDHIDE_PANTS } },
        // 116 Mordresh Fire Eye
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::BONE_FRAGMENTS, 100, 1, 2 }, none }, true,
          { i::MORDRESHS_BLADE, i::STAFF_OF_THE_FIRE_EYE, i::BONE_LONGBOW } },
        // 117 Glutton
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::BONE_FRAGMENTS, 100, 1, 2 }, none }, true,
          { i::GLUTTONOUS_SABATONS, i::LARDER_SLIPPERS, i::BUTCHERS_BOOTS } },
        // 118 Amnennar the Coldbringer
        { { { i::MAJOR_HEALING_POTION, 100, 2, 3 }, none, none }, true,
          { i::COLDBRINGER_HELM, i::COLDBRINGER_COWL, i::FROSTBITTEN_MASK } },
        // 119 Antu'sul
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::TROLL_TUSK, 100, 1, 2 }, none }, true,
          { i::ANTUSULS_GAUNTLETS, i::SANDFURY_WRAPS, i::SCARABSKIN_GRIPS } },
        // 120 Theka the Martyr
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::TROLL_TUSK, 100, 1, 2 }, none }, true,
          { i::MARTYRS_BREASTPLATE, i::THEKAS_ROBE, i::MARTYRS_VEST } },
        // 121 Witch Doctor Zum'rah
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::TROLL_TUSK, 100, 1, 2 }, none }, true,
          { i::WITCH_DOCTOR_MACHETE, i::WITCH_DOCTOR_STAFF, i::VOODOO_LONGBOW } },
        // 122 Gahz'rilla
        { { { i::MAJOR_HEALING_POTION, 100, 1, 2 }, none, none }, false,
          { i::HYDRA_LEGPLATES, i::GAHZ_RILLA_LEGGINGS, i::HYDRAHIDE_PANTS } },
        // 123 Nekrum Gutchewer and Shadowpriest Sezz'ziz
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::RUNECLOTH, 100, 2, 3 }, none }, true,
          { i::GUTCHEWER_SABATONS, i::SHADOWPRIEST_SLIPPERS, i::SEZZ_ZIZS_BOOTS } },
        // 124 Ruuzlu
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::TROLL_TUSK, 100, 1, 2 }, none }, true,
          { i::RUUZLUS_AXE, i::SANDFURY_SPIRE, i::RUUZLUS_RECURVE } },
        // 125 Chief Ukorz Sandscalp
        { { { i::MAJOR_HEALING_POTION, 100, 2, 3 }, { i::TROLL_TUSK, 100, 1, 2 }, none }, true,
          { i::SANDSCALP_HELM, i::CHIEFTAINS_HEADDRESS, i::SANDSCALP_MASK } },
        // 126 Hatecrest naga
        { { { i::NAGA_SCALE, 40, 1, 1 }, { i::RUNECLOTH, 30, 1, 2 }, { i::MAJOR_HEALING_POTION, 5, 1, 1 } }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 127 shore striders
        { { { i::ELEMENTAL_EARTH, 35, 1, 1 }, none, none }, false, { i::NONE, i::NONE, i::NONE } },
        // 128 Rage Scar yetis
        { { { i::THICK_FUR, 45, 1, 1 }, { i::YETI_HORN, 25, 1, 1 }, none }, false, { i::NONE, i::NONE, i::NONE } },
        // 129 Gordunni ogres
        { { { i::OGRE_TOOTH, 40, 1, 1 }, { i::RUNECLOTH, 35, 1, 2 }, { i::MAJOR_HEALING_POTION, 5, 1, 1 } }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 130 Grimtotem tauren
        { { { i::RUNECLOTH, 35, 1, 2 }, { i::THICK_FUR, 20, 1, 1 }, { i::MAJOR_HEALING_POTION, 5, 1, 1 } }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 131 wildkin
        { { { i::WILDKIN_FEATHER, 50, 1, 1 }, none, none }, false, { i::NONE, i::NONE, i::NONE } },
        // 132 the Jademir
        { { { i::SPLINTERED_BARK, 40, 1, 1 }, none, none }, false, { i::NONE, i::NONE, i::NONE } },
        // 133 Magram centaurs
        { { { i::RUNECLOTH, 30, 1, 2 }, { i::THICK_FUR, 25, 1, 1 }, { i::MAJOR_HEALING_POTION, 5, 1, 1 } }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 134 satyrs
        { { { i::SATYR_HORN, 40, 1, 1 }, { i::FELCLOTH, 25, 1, 1 }, { i::MAJOR_HEALING_POTION, 5, 1, 1 } }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 135 sludges
        { { { i::GLOWING_SLUDGE, 45, 1, 1 }, none, none }, false, { i::NONE, i::NONE, i::NONE } },
        // 136 lashers and treants
        { { { i::SPLINTERED_BARK, 45, 1, 1 }, none, none }, false, { i::NONE, i::NONE, i::NONE } },
        // 137 the Theradrim
        { { { i::ELEMENTAL_EARTH, 45, 1, 1 }, none, none }, false, { i::NONE, i::NONE, i::NONE } },
        // 138 diemetradons
        { { { i::THICK_FUR, 40, 1, 1 }, none, none }, false, { i::NONE, i::NONE, i::NONE } },
        // 139 the Highborne dead
        { { { i::RUNECLOTH, 35, 1, 2 }, { i::GHOSTLY_ECTOPLASM, 30, 1, 1 }, { i::MAJOR_HEALING_POTION, 5, 1, 1 } }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 140 Gordok ogres
        { { { i::OGRE_TOOTH, 40, 1, 1 }, { i::RUNECLOTH, 35, 1, 2 }, { i::MAJOR_HEALING_POTION, 5, 1, 1 } }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 141 hydrolings
        { { { i::ELEMENTAL_EARTH, 20, 1, 1 }, none, none }, false, { i::NONE, i::NONE, i::NONE } },
        // 142 Lord Shalzaru
        { { { i::NAGA_SCALE, 100, 1, 2 }, { i::RUNECLOTH, 100, 2, 3 }, none }, true,
          { i::SHALZARUS_LEGPLATES, i::NAGA_LORD_LEGGINGS, i::DREADSCALE_PANTS } },
        // 143 Old Grizzlegut
        { { { i::THICK_FUR, 100, 2, 3 }, none, none }, false,
          { i::GRIZZLEGUT_GAUNTLETS, i::GRIZZLEGUT_GLOVES, i::GRIZZLEGUT_GRIPS } },
        // 144 Noxxion
        { { { i::GLOWING_SLUDGE, 100, 2, 3 }, none, none }, false,
          { i::NOXIOUS_GAUNTLETS, i::TOXIC_WRAPS, i::SLUDGE_COVERED_GRIPS } },
        // 145 Razorlash
        { { { i::SPLINTERED_BARK, 100, 2, 3 }, none, none }, false,
          { i::THORNSTRIDER_SABATONS, i::VINEWOVEN_SLIPPERS, i::RAZORLASH_BOOTS } },
        // 146 Lord Vyletongue
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::SATYR_HORN, 100, 1, 2 }, none }, true,
          { i::VYLETONGUES_BLADE, i::PUTRIDUS_STAFF, i::SATYRHORN_BOW } },
        // 147 Celebras the Cursed
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::SPLINTERED_BARK, 100, 1, 2 }, none }, true,
          { i::CELEBRAS_HELM, i::KEEPERS_COWL, i::GROVEWARDEN_MASK } },
        // 148 Tinkerer Gizlock
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::GREASY_COG, 100, 1, 2 }, none }, true,
          { i::GIZLOCKS_WRENCH, i::GIZLOCKS_STAFF, i::GIZLOCKS_HAND_CANNON } },
        // 149 Landslide
        { { { i::ELEMENTAL_EARTH, 100, 2, 3 }, none, none }, false,
          { i::ROCKSLIDE_LEGPLATES, i::EARTHWEAVE_LEGGINGS, i::STONEHIDE_PANTS } },
        // 150 Rotgrip
        { { { i::CROCOLISK_SCALE, 100, 1, 2 }, none, none }, false,
          { i::ROTGRIP_HAUBERK, i::FENWEAVE_ROBE, i::CROCSCALE_VEST } },
        // 151 Princess Theradras
        { { { i::MAJOR_HEALING_POTION, 100, 2, 3 }, { i::ELEMENTAL_EARTH, 100, 2, 3 }, none }, false,
          { i::PRINCESSS_GREATAXE, i::STAFF_OF_THERADRAS, i::EARTHSONG_LONGBOW } },
        // 152 Zevrim Thornhoof
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::SATYR_HORN, 100, 1, 2 }, none }, true,
          { i::THORNHOOF_SABATONS, i::HELLFIRE_SLIPPERS, i::FELHIDE_BOOTS } },
        // 153 Hydrospawn
        { { { i::MAJOR_HEALING_POTION, 100, 1, 2 }, none, none }, false,
          { i::HYDROSPAWN_GAUNTLETS, i::TIDAL_GLOVES, i::WATERLOGGED_GRIPS } },
        // 154 Lethtendris
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::FELCLOTH, 100, 1, 2 }, none }, true,
          { i::LETHTENDRISS_HELM, i::SHADOWWEAVE_COWL, i::WEBSPUN_MASK } },
        // 155 Alzzin the Wildshaper
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::SATYR_HORN, 100, 1, 2 }, none }, true,
          { i::WILDSHAPERS_LEGGUARDS, i::WILDWEAVE_LEGGINGS, i::WILDHIDE_PANTS } },
        // 156 Tendris Warpwood
        { { { i::SPLINTERED_BARK, 100, 2, 3 }, none, none }, false,
          { i::WARPWOOD_HAUBERK, i::BARKWEAVE_ROBE, i::IRONBARK_VEST } },
        // 157 Immol'thar
        { { { i::MAJOR_HEALING_POTION, 100, 1, 2 }, none, none }, false,
          { i::IMMOL_THARS_CLAW, i::DEMONIC_STAFF, i::FEL_LONGBOW } },
        // 158 Prince Tortheldrin
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::RUNECLOTH, 100, 2, 3 }, none }, true,
          { i::TORTHELDRINS_BLADE, i::HIGHBORNE_STAFF, i::PRINCES_LONGBOW } },
        // 159 Cho'Rush the Observer
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::OGRE_TOOTH, 100, 1, 2 }, none }, true,
          { i::OBSERVERS_GAUNTLETS, i::CHO_RUSHS_GLOVES, i::OBSERVERS_GRIPS } },
        // 160 King Gordok
        { { { i::MAJOR_HEALING_POTION, 100, 2, 3 }, { i::OGRE_TOOTH, 100, 1, 2 }, none }, true,
          { i::GORDOKS_GREATAXE, i::STAFF_OF_THE_OGRE_KING, i::GORDOKS_LONGBOW } },
        // 161 Blackrock orcs
        { { { i::RUNECLOTH, 35, 1, 2 }, { i::DARK_IRON_SCRAPS, 20, 1, 1 }, { i::MAJOR_HEALING_POTION, 5, 1, 1 } }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 162 worgs
        { { { i::THICK_FUR, 45, 1, 1 }, none, none }, false, { i::NONE, i::NONE, i::NONE } },
        // 163 Firegut ogres
        { { { i::OGRE_TOOTH, 40, 1, 1 }, { i::RUNECLOTH, 35, 1, 2 }, { i::MAJOR_HEALING_POTION, 5, 1, 1 } }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 164 dragonkin
        { { { i::BLACK_DRAGONSCALE, 40, 1, 1 }, { i::DREAM_DUST, 15, 1, 1 }, none }, false,
          { i::NONE, i::NONE, i::NONE } },
        // 165 the Dark Iron
        { { { i::DARK_IRON_SCRAPS, 40, 1, 1 }, { i::RUNECLOTH, 35, 1, 2 }, { i::MAJOR_HEALING_POTION, 5, 1, 1 } }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 166 creatures of the lava
        { { { i::SMOLDERING_COAL, 45, 1, 1 }, none, none }, false, { i::NONE, i::NONE, i::NONE } },
        // 167 Stonevault troggs
        { { { i::TROGG_STONE_TOOTH, 45, 1, 1 }, { i::RUNECLOTH, 20, 1, 1 }, none }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 168 swamp beasts
        { { { i::CROCOLISK_SCALE, 35, 1, 1 }, { i::THICK_FUR, 25, 1, 1 }, none }, false,
          { i::NONE, i::NONE, i::NONE } },
        // 169 the Atal'ai
        { { { i::ATAL_AI_TOKEN, 40, 1, 1 }, { i::TROLL_TUSK, 25, 1, 1 }, { i::MAJOR_HEALING_POTION, 5, 1, 1 } }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 170 Uldaman's stone keepers
        { { { i::TITANIC_STONE_FRAGMENT, 40, 1, 1 }, { i::ELEMENTAL_EARTH, 25, 1, 1 }, none }, false,
          { i::NONE, i::NONE, i::NONE } },
        // 171 oozes
        { { { i::GLOWING_SLUDGE, 45, 1, 1 }, none, none }, false, { i::NONE, i::NONE, i::NONE } },
        // 172 golems
        { { { i::GREASY_COG, 45, 1, 1 }, { i::DARK_IRON_SCRAPS, 25, 1, 1 }, none }, false,
          { i::NONE, i::NONE, i::NONE } },
        // 173 Gor'tesh
        { { { i::DARK_IRON_SCRAPS, 100, 1, 2 }, { i::RUNECLOTH, 100, 2, 3 }, none }, true,
          { i::GOR_TESHS_LEGPLATES, i::GOR_TESHS_LEGGINGS, i::GOR_TESHS_PANTS } },
        // 174 Gorgon'och
        { { { i::OGRE_TOOTH, 100, 1, 2 }, { i::RUNECLOTH, 100, 2, 3 }, none }, true,
          { i::GORGON_OCHS_GAUNTLETS, i::GORGON_OCHS_GLOVES, i::GORGON_OCHS_GRIPS } },
        // 175 War Reaver
        { { { i::GREASY_COG, 100, 2, 3 }, none, none }, false,
          { i::REAVER_GAUNTLETS, i::REAVER_GLOVES, i::REAVER_GRIPS } },
        // 176 Overseer Maltorius
        { { { i::DARK_IRON_SCRAPS, 100, 1, 2 }, { i::RUNECLOTH, 100, 2, 3 }, none }, true,
          { i::MALTORIUS_SABATONS, i::MALTORIUS_SLIPPERS, i::MALTORIUS_BOOTS } },
        // 177 Revelosh
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::TROGG_STONE_TOOTH, 100, 1, 2 }, none }, true,
          { i::REVELOSH_GAUNTLETS, i::REVELOSH_GLOVES, i::REVELOSH_GRIPS } },
        // 178 Grimlok
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::TROGG_STONE_TOOTH, 100, 1, 2 }, none }, true,
          { i::GRIMLOK_SABATONS, i::GRIMLOK_SLIPPERS, i::GRIMLOK_BOOTS } },
        // 179 Galgann Firehammer
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::DARK_IRON_SCRAPS, 100, 1, 2 }, none }, true,
          { i::GALGANNS_FIREHAMMER, i::GALGANNS_STAFF, i::GALGANNS_RIFLE } },
        // 180 the Ancient Stone Keeper
        { { { i::TITANIC_STONE_FRAGMENT, 100, 2, 3 }, none, none }, false,
          { i::ANCIENT_KEEPER_LEGPLATES, i::ANCIENT_KEEPER_LEGGINGS, i::ANCIENT_KEEPER_PANTS } },
        // 181 Ironaya
        { { { i::TITANIC_STONE_FRAGMENT, 100, 2, 3 }, none, none }, false,
          { i::IRONAYAS_HAUBERK, i::IRONAYAS_ROBE, i::IRONAYAS_VEST } },
        // 182 the Obsidian Sentinel
        { { { i::TITANIC_STONE_FRAGMENT, 100, 2, 3 }, none, none }, false,
          { i::OBSIDIAN_HELM, i::OBSIDIAN_COWL, i::OBSIDIAN_MASK } },
        // 183 Archaedas
        { { { i::MAJOR_HEALING_POTION, 100, 2, 3 }, { i::TITANIC_STONE_FRAGMENT, 100, 2, 3 }, none }, false,
          { i::TITAN_GREATHAMMER, i::EARTHSHAPER_STAFF, i::STONEWAKER_LONGBOW } },
        // 184 Atal'alarion
        { { { i::TITANIC_STONE_FRAGMENT, 100, 2, 3 }, none, none }, false,
          { i::ALARIONS_GAUNTLETS, i::ALARIONS_GLOVES, i::ALARIONS_GRIPS } },
        // 185 Jammal'an the Prophet
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::ATAL_AI_TOKEN, 100, 1, 2 }, none }, true,
          { i::JAMMAL_ANS_HELM, i::JAMMAL_ANS_COWL, i::JAMMAL_ANS_MASK } },
        // 186 Ogom the Wretched
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::ATAL_AI_TOKEN, 100, 1, 2 }, none }, true,
          { i::WRETCHED_SABATONS, i::WRETCHED_SLIPPERS, i::WRETCHED_BOOTS } },
        // 187 Hazzas
        { { { i::DREAM_DUST, 100, 2, 3 }, none, none }, false,
          { i::HAZZAS_LEGPLATES, i::HAZZAS_LEGGINGS, i::HAZZAS_PANTS } },
        // 188 Morphaz
        { { { i::DREAM_DUST, 100, 2, 3 }, none, none }, false,
          { i::MORPHAZ_HAUBERK, i::MORPHAZ_ROBE, i::MORPHAZ_VEST } },
        // 189 Dreamscythe
        { { { i::DREAM_DUST, 100, 2, 3 }, none, none }, false,
          { i::DREAMSCYTHE_GREATAXE, i::DREAMSCYTHE_STAFF, i::DREAMSCYTHE_LONGBOW } },
        // 190 Weaver
        { { { i::DREAM_DUST, 100, 2, 3 }, none, none }, false,
          { i::WEAVERS_GAUNTLETS, i::WEAVERS_GLOVES, i::WEAVERS_GRIPS } },
        // 191 the Shade of Eranikus
        { { { i::MAJOR_HEALING_POTION, 100, 2, 3 }, { i::DREAM_DUST, 100, 2, 3 }, none }, false,
          { i::FANG_OF_ERANIKUS, i::STAFF_OF_ERANIKUS, i::BOW_OF_ERANIKUS } },
        // 192 Lord Roccor
        { { { i::SMOLDERING_COAL, 100, 2, 3 }, none, none }, false,
          { i::ROCCORS_SABATONS, i::ROCCORS_SLIPPERS, i::ROCCORS_BOOTS } },
        // 193 High Interrogator Gerstahn
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::DARK_IRON_SCRAPS, 100, 1, 2 }, none }, true,
          { i::INTERROGATORS_GAUNTLETS, i::INTERROGATORS_GLOVES, i::INTERROGATORS_GRIPS } },
        // 194 the Ring of Law's champions
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::RUNECLOTH, 100, 1, 2 }, none }, true,
          { i::CHAMPIONS_LEGPLATES, i::CHAMPIONS_LEGGINGS, i::CHAMPIONS_PANTS } },
        // 195 Golem Lord Argelmach
        { { { i::GREASY_COG, 100, 2, 3 }, none, none }, false,
          { i::ARGELMACHS_HAUBERK, i::ARGELMACHS_ROBE, i::ARGELMACHS_VEST } },
        // 196 General Angerforge
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::DARK_IRON_SCRAPS, 100, 1, 2 }, none }, true,
          { i::ANGERFORGES_AXE, i::FORGEMASTERS_STAFF, i::ANGERFORGES_RIFLE } },
        // 197 Ambassador Flamelash
        { { { i::SMOLDERING_COAL, 100, 2, 3 }, none, none }, false,
          { i::FLAMELASH_HELM, i::FLAMELASH_COWL, i::FLAMELASH_MASK } },
        // 198 Magmus
        { { { i::SMOLDERING_COAL, 100, 2, 3 }, none, none }, false,
          { i::MAGMUS_LEGPLATES, i::MAGMUS_LEGGINGS, i::MAGMUS_PANTS } },
        // 199 Princess Moira Bronzebeard
        { { { i::MAJOR_HEALING_POTION, 100, 1, 2 }, none, none }, false,
          { i::NONE, i::NONE, i::NONE } },
        // 200 Emperor Dagran Thaurissan
        { { { i::MAJOR_HEALING_POTION, 100, 2, 3 }, { i::DARK_IRON_SCRAPS, 100, 2, 3 }, none }, true,
          { i::THAURISSANS_GREATHAMMER, i::STAFF_OF_THE_SHADOWFORGE, i::IMPERIAL_LONGBOW } },
        // 201 the Scourge of the Western Plaguelands
        { { { i::BONE_FRAGMENTS, 40, 1, 2 }, { i::RUNECLOTH, 30, 1, 2 }, { i::MAJOR_HEALING_POTION, 5, 1, 1 } }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 202 plagued beasts
        { { { i::THICK_FUR, 40, 1, 1 }, { i::SCOURGESTONE, 10, 1, 1 }, none }, false,
          { i::NONE, i::NONE, i::NONE } },
        // 203 the Scarlet Crusade
        { { { i::SCARLET_INSIGNIA, 40, 1, 1 }, { i::RUNECLOTH, 35, 1, 2 }, { i::MAJOR_HEALING_POTION, 5, 1, 1 } }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 204 the Scourge of the Eastern Plaguelands
        { { { i::SCOURGESTONE, 40, 1, 1 }, { i::NECROTIC_RUNE, 20, 1, 1 }, { i::RUNECLOTH, 30, 1, 2 } }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 205 plaguehounds
        { { { i::THICK_FUR, 45, 1, 1 }, { i::SCOURGESTONE, 15, 1, 1 }, none }, false,
          { i::NONE, i::NONE, i::NONE } },
        // 206 Araj the Summoner
        { { { i::NECROTIC_RUNE, 100, 1, 2 }, { i::RUNECLOTH, 100, 2, 3 }, none }, true,
          { i::ARAJS_SABATONS, i::ARAJS_SLIPPERS, i::ARAJS_BOOTS } },
        // 207 Grand Inquisitor Isillien
        { { { i::SCARLET_INSIGNIA, 100, 1, 2 }, { i::RUNECLOTH, 100, 2, 3 }, none }, true,
          { i::ISILLIENS_GAUNTLETS, i::ISILLIENS_GLOVES, i::ISILLIENS_GRIPS } },
        // 208 Hed'mush the Rotting
        { { { i::SCOURGESTONE, 100, 2, 3 }, none, none }, false,
          { i::HED_MUSHS_LEGPLATES, i::HED_MUSHS_LEGGINGS, i::HED_MUSHS_PANTS } },
        // 209 Crusader Lord Valdelmar
        { { { i::SCARLET_INSIGNIA, 100, 1, 2 }, { i::RUNECLOTH, 100, 2, 3 }, none }, true,
          { i::VALDELMARS_HELM, i::VALDELMARS_COWL, i::VALDELMARS_MASK } },
        // 210 Blackrock Spire's ogres, trolls and orcs
        { { { i::BLACKROCK_EMBLEM, 40, 1, 1 }, { i::RUNECLOTH, 35, 1, 2 }, { i::MAJOR_HEALING_POTION, 5, 1, 1 } }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 211 Blackrock Spire's spiders and dragonkin
        { { { i::CHROMATIC_SCALE, 30, 1, 1 }, { i::IRONWEB_SPIDER_SILK, 30, 1, 1 }, none }, false,
          { i::NONE, i::NONE, i::NONE } },
        // 212 Highlord Omokk
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::OGRE_TOOTH, 100, 1, 2 }, none }, true,
          { i::OMOKKS_GAUNTLETS, i::OMOKKS_GLOVES, i::OMOKKS_GRIPS } },
        // 213 War Master Voone
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::TROLL_TUSK, 100, 1, 2 }, none }, true,
          { i::VOONES_CLEAVER, i::VOONES_STAFF, i::VOONES_LONGBOW } },
        // 214 Mother Smolderweb
        { { { i::IRONWEB_SPIDER_SILK, 100, 2, 3 }, none, none }, false,
          { i::SMOLDERWEB_SABATONS, i::SMOLDERWEB_SLIPPERS, i::SMOLDERWEB_BOOTS } },
        // 215 Overlord Wyrmthalak
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::BLACKROCK_EMBLEM, 100, 1, 2 }, none }, true,
          { i::WYRMTHALAKS_LEGPLATES, i::WYRMTHALAKS_LEGGINGS, i::WYRMTHALAKS_PANTS } },
        // 216 Pyroguard Emberseer
        { { { i::SMOLDERING_COAL, 100, 2, 3 }, none, none }, false,
          { i::EMBERSEER_HAUBERK, i::EMBERSEER_ROBE, i::EMBERSEER_VEST } },
        // 217 The Beast
        { { { i::THICK_FUR, 100, 2, 3 }, none, none }, false,
          { i::BEASTMAW_HELM, i::BEASTMAW_COWL, i::BEASTMAW_MASK } },
        // 218 General Drakkisath
        { { { i::MAJOR_HEALING_POTION, 100, 2, 3 }, { i::CHROMATIC_SCALE, 100, 2, 3 }, none }, false,
          { i::DRACONIC_GREATSWORD, i::STAFF_OF_DRAKKISATH, i::DRAGONSPUR_LONGBOW } },
        // 219 the school of Scholomance
        { { { i::NECROTIC_RUNE, 40, 1, 1 }, { i::BONE_FRAGMENTS, 30, 1, 2 }, { i::MAJOR_HEALING_POTION, 5, 1, 1 } }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 220 Jandice Barov
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::RUNECLOTH, 100, 2, 3 }, none }, true,
          { i::ILLUSIONISTS_GAUNTLETS, i::ILLUSIONISTS_GLOVES, i::ILLUSIONISTS_GRIPS } },
        // 221 Rattlegore
        { { { i::BONE_FRAGMENTS, 100, 2, 3 }, none, none }, false,
          { i::BONECRUSHER_LEGPLATES, i::BONECRUSHER_LEGGINGS, i::BONECRUSHER_PANTS } },
        // 222 Ras Frostwhisper
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::NECROTIC_RUNE, 100, 1, 2 }, none }, false,
          { i::FROSTBITE_GREATAXE, i::FROSTWHISPER_STAFF, i::FROSTWHISPER_LONGBOW } },
        // 223 Instructor Malicia
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::NECROTIC_RUNE, 100, 1, 2 }, none }, true,
          { i::INSTRUCTORS_SABATONS, i::INSTRUCTORS_SLIPPERS, i::INSTRUCTORS_BOOTS } },
        // 224 Doctor Theolen Krastinov
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::RUNECLOTH, 100, 2, 3 }, none }, true,
          { i::SURGEONS_HAUBERK, i::SURGEONS_ROBE, i::SURGEONS_VEST } },
        // 225 Lorekeeper Polkelt
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::NECROTIC_RUNE, 100, 1, 2 }, none }, false,
          { i::LOREKEEPERS_HELM, i::LOREKEEPERS_COWL, i::LOREKEEPERS_MASK } },
        // 226 Lord Alexei Barov
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::RUNECLOTH, 100, 2, 3 }, none }, true,
          { i::ALEXEIS_GAUNTLETS, i::ALEXEIS_GLOVES, i::ALEXEIS_GRIPS } },
        // 227 Lady Illucia Barov
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::RUNECLOTH, 100, 2, 3 }, none }, true,
          { i::ILLUCIAS_SABATONS, i::ILLUCIAS_SLIPPERS, i::ILLUCIAS_BOOTS } },
        // 228 Darkmaster Gandling
        { { { i::MAJOR_HEALING_POTION, 100, 2, 3 }, { i::NECROTIC_RUNE, 100, 2, 3 }, none }, true,
          { i::HEADMASTERS_HAUBERK, i::HEADMASTERS_ROBE, i::HEADMASTERS_VEST } },
        // 229 Stratholme's Scarlet Crusade
        { { { i::SCARLET_INSIGNIA, 40, 1, 1 }, { i::RUNECLOTH, 35, 1, 2 }, { i::MAJOR_HEALING_POTION, 5, 1, 1 } }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 230 Stratholme's Scourge
        { { { i::SCOURGESTONE, 40, 1, 1 }, { i::NECROTIC_RUNE, 25, 1, 1 }, { i::RUNECLOTH, 25, 1, 2 } }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 231 Timmy the Cruel
        { { { i::SCOURGESTONE, 100, 2, 3 }, none, none }, false,
          { i::CRUEL_GAUNTLETS, i::CRUEL_GLOVES, i::CRUEL_GRIPS } },
        // 232 Malor the Zealous
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::SCARLET_INSIGNIA, 100, 1, 2 }, none }, true,
          { i::ZEALOUS_WARHAMMER, i::ZEALOUS_STAFF, i::ZEALOUS_RIFLE } },
        // 233 Balnazzar
        { { { i::MAJOR_HEALING_POTION, 100, 2, 3 }, { i::SCARLET_INSIGNIA, 100, 2, 3 }, none }, true,
          { i::DREADLORD_LEGPLATES, i::DREADLORD_LEGGINGS, i::DREADLORD_PANTS } },
        // 234 Maleki the Pallid
        { { { i::MAJOR_HEALING_POTION, 60, 1, 2 }, { i::NECROTIC_RUNE, 100, 1, 2 }, none }, false,
          { i::PALLID_SABATONS, i::PALLID_SLIPPERS, i::PALLID_BOOTS } },
        // 235 Ramstein the Gorger
        { { { i::SCOURGESTONE, 100, 2, 3 }, none, none }, false,
          { i::GORGERS_HAUBERK, i::GORGERS_ROBE, i::GORGERS_VEST } },
        // 236 Baron Rivendare
        { { { i::MAJOR_HEALING_POTION, 100, 2, 3 }, { i::SCOURGESTONE, 100, 2, 3 }, none }, true,
          { i::RIVENDARES_RUNEBLADE, i::STAFF_OF_THE_BARON, i::BARONS_LONGBOW } },
        // 237 Darkmist widows
        { { { i::IRONWEB_SPIDER_SILK, 50, 1, 1 }, none, none }, false, { i::NONE, i::NONE, i::NONE } },
        // 238 Mudrock snapjaws
        { { { i::MUDROCK_SHELL, 50, 1, 1 }, none, none }, false, { i::NONE, i::NONE, i::NONE } },
        // 239 the Wyrmbog's dragonkin
        { { { i::BLACK_DRAGONSCALE, 45, 1, 1 }, { i::MAJOR_HEALING_POTION, 4, 1, 1 }, none }, false,
          { i::NONE, i::NONE, i::NONE } },
        // 240 Onyxian Warders
        { { { i::CHROMATIC_SCALE, 50, 1, 2 }, { i::MAJOR_HEALING_POTION, 15, 1, 1 }, none }, true,
          { i::NONE, i::NONE, i::NONE } },
        // 241 Emberstrife
        { { { i::CHROMATIC_SCALE, 100, 2, 3 }, none, none }, false,
          { i::EMBERSTRIFE_SABATONS, i::EMBERSTRIFE_SLIPPERS, i::EMBERSTRIFE_BOOTS } },
        // 242 Onyxia
        { { { i::MAJOR_HEALING_POTION, 100, 2, 3 }, { i::SCALE_OF_ONYXIA, 100, 2, 3 }, none }, true,
          { i::DEATHBRINGER, i::STAFF_OF_THE_BLACK_FLIGHT, i::DRAGONBREATH_HAND_CANNON } },
    };

    // Uncommon items any enemy of a level band may drop.
    constexpr item_id band_1[] = { i::FOOTPAD_VEST, i::GLIMMERING_GLOVES, i::BRONZE_LEGGINGS, i::BANDIT_SHORTSWORD };
    constexpr item_id band_2[] = { i::FOREST_PANTS, i::SPELLBINDER_BOOTS, i::SCALEMAIL_GLOVES, i::IRONWOOD_MACE,
                                   i::ASH_LONGBOW };
    constexpr item_id band_3[] = { i::SCOUTING_BOOTS, i::SILKEN_COWL, i::IRONCLAD_LEGGINGS, i::FINE_LONGSWORD,
                                   i::HORNWOOD_BOW, i::MILITIA_SHIELD };
    constexpr item_id band_4[] = { i::BLACKENED_LEGGINGS, i::CINDERCLOTH_ROBE, i::POLISHED_BOOTS,
                                   i::MINERS_REVENGE };
    constexpr item_id band_5[] = { i::SCALED_LEATHER_HEADBAND, i::GREENWEAVE_ROBE, i::DEFENDER_GAUNTLETS,
                                   i::KNIGHTS_LONGSWORD, i::EMBERSTONE_STAFF, i::OUTRIDERS_BOW };
    constexpr item_id band_6[] = { i::BOGWALKER_BOOTS, i::FENPLATE_LEGGINGS, i::MOONGLOW_HOOD, i::KNIGHTLY_GREATSWORD,
                                   i::SAGES_STAFF, i::HAWKEYE_BOW, i::BULWARK_SHIELD };
    constexpr item_id band_7[] = { i::ALTERAC_CHAIN_HELM, i::SILKWEAVE_ROBE, i::STALKERS_LEGGINGS,
                                   i::BATTLEFORGE_GREATSWORD, i::IVORY_STAFF, i::IRONBARK_LONGBOW, i::BASTION_SHIELD };
    constexpr item_id band_8[] = { i::EMBERFORGED_HELM, i::STARSILK_ROBE, i::JUNGLESTALKER_LEGGINGS,
                                   i::CRESCENT_GREATSWORD, i::SERPENTWOOD_STAFF, i::THORNROOT_LONGBOW,
                                   i::BULWARK_OF_THE_VALE };
    constexpr item_id band_9[] = { i::SANDSTORM_HELM, i::MIRAGE_ROBE, i::DUNESHADOW_LEGGINGS, i::SCORCHING_GREATSWORD,
                                   i::STAFF_OF_THE_DUNES, i::SIROCCO_LONGBOW, i::SANDSTONE_BULWARK };
    constexpr item_id band_10[] = { i::THUNDERHEAD_HELM, i::MOONSHADOW_ROBE, i::WILDHEART_LEGGINGS,
                                    i::TWILIGHT_GREATSWORD, i::STAFF_OF_THE_ANCIENTS, i::FERALAS_LONGBOW,
                                    i::HIGHBORNE_BULWARK };
    constexpr item_id band_11[] = { i::VOLCANIC_HELM, i::FLAMEKISSED_ROBE, i::EMBERHIDE_LEGGINGS, i::BLAZING_GREATSWORD,
                                    i::STAFF_OF_EMBERS, i::ASHWOOD_LONGBOW, i::OBSIDIAN_BULWARK };
    constexpr item_id band_12[] = { i::PLAGUEBRINGER_HELM, i::NECROTIC_ROBE, i::BLIGHTWALKER_LEGGINGS, i::LIGHTS_VENGEANCE,
                                    i::STAFF_OF_THE_DEAD, i::DAWNGUARD_LONGBOW, i::BULWARK_OF_THE_DAWN };

    constexpr int world_drop_chance = 3;
    constexpr int elite_world_drop_chance = 35;

    template<int Size>
    [[nodiscard]] item_id pick(const item_id (&pool)[Size])
    {
        // One reroll towards gear the player can use.
        item_id result = pool[random_range(0, Size - 1)];

        if(! can_equip(character().player_class, get_item(result)))
        {
            result = pool[random_range(0, Size - 1)];
        }

        return result;
    }

    [[nodiscard]] item_id world_drop(int level)
    {
        if(level <= 6)
        {
            return pick(band_1);
        }

        if(level <= 11)
        {
            return pick(band_2);
        }

        if(level <= 15)
        {
            return pick(band_3);
        }

        if(level <= 20)
        {
            return pick(band_4);
        }

        if(level <= 25)
        {
            return pick(band_5);
        }

        if(level <= 30)
        {
            return pick(band_6);
        }

        if(level <= 35)
        {
            return pick(band_7);
        }

        if(level <= 40)
        {
            return pick(band_8);
        }

        if(level <= 45)
        {
            return pick(band_9);
        }

        if(level <= 50)
        {
            return pick(band_10);
        }

        if(level <= 55)
        {
            return pick(band_11);
        }

        return pick(band_12);
    }

    void add(enemy& item, item_id loot, int count)
    {
        for(loot_slot& slot : item.loot)
        {
            if(slot.item == item_id::NONE)
            {
                slot.item = loot;
                slot.count = uint8_t(count);
                return;
            }
        }
    }
}

item_id roll_world_drop(int level)
{
    return world_drop(level);
}

void roll_loot(enemy& item)
{
    int level = item.level;
    int index = item.def->loot_table;
    const loot_table& table = tables[index < int(sizeof(tables) / sizeof(tables[0])) ? index : 0];

    for(loot_slot& slot : item.loot)
    {
        slot = loot_slot();
    }

    item.loot_money = 0;

    if(table.money)
    {
        item.loot_money = random_range(level * 2, level * 5);

        if(item.elite() || item.rare())
        {
            item.loot_money *= 4;
        }
    }

    if(table.choice[0] != item_id::NONE)
    {
        int choices = table.choice[2] != item_id::NONE ? 3 : table.choice[1] != item_id::NONE ? 2 : 1;
        add(item, table.choice[random_range(0, choices - 1)], 1);
    }

    // The last bosses of the late dungeons always drop the hero's piece of their class's set, in
    // place of the world drop.
    item_id set_piece = item_set_drop(item.id);

    if(set_piece != item_id::NONE)
    {
        add(item, set_piece, 1);
    }
    else if(random_chance(item.elite() || item.rare() ? elite_world_drop_chance : world_drop_chance))
    {
        add(item, world_drop(level), 1);
    }

    for(const loot_entry& entry : table.entries)
    {
        if(entry.item != item_id::NONE && random_chance(entry.chance))
        {
            add(item, entry.item, random_range(entry.min_count, entry.max_count));
        }
    }
}

}
