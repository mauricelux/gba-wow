#include "gw_loot.h"

#include "gw_character.h"
#include "gw_enemies.h"
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
    };

    // Uncommon items any enemy of a level band may drop.
    constexpr item_id band_1[] = { i::FOOTPAD_VEST, i::GLIMMERING_GLOVES, i::BRONZE_LEGGINGS, i::BANDIT_SHORTSWORD };
    constexpr item_id band_2[] = { i::FOREST_PANTS, i::SPELLBINDER_BOOTS, i::SCALEMAIL_GLOVES, i::IRONWOOD_MACE,
                                   i::ASH_LONGBOW };
    constexpr item_id band_3[] = { i::SCOUTING_BOOTS, i::SILKEN_COWL, i::IRONCLAD_LEGGINGS, i::FINE_LONGSWORD,
                                   i::HORNWOOD_BOW, i::MILITIA_SHIELD };
    constexpr item_id band_4[] = { i::BLACKENED_LEGGINGS, i::CINDERCLOTH_ROBE, i::POLISHED_BOOTS,
                                   i::MINERS_REVENGE };

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

        return pick(band_4);
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

    if(random_chance(item.elite() || item.rare() ? elite_world_drop_chance : world_drop_chance))
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
