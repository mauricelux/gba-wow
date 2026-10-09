#ifndef GW_QUEST_IDS_H
#define GW_QUEST_IDS_H

#include <cstdint>

namespace gw
{

// Keep in the same order as the table in gw_quests.cpp. Saves index quest progress by these values:
// only append, and keep COUNT within gw::max_quests.
enum class quest_id : uint16_t
{
    NONE,
    // Northshire
    A_THREAT_WITHIN,
    KOBOLD_CAMP_CLEANUP,
    WOLVES_ACROSS_THE_BORDER,
    MILLYS_HARVEST,
    BROTHERHOOD_OF_THIEVES,
    REPORT_TO_GOLDSHIRE,
    // Elwynn Forest
    THE_FARGODEEP_MINE,
    GOLD_DUST_EXCHANGE,
    BOUNTY_ON_MURLOCS,
    PROTECT_THE_FRONTIER,
    BOAR_MEAT,
    PRINCESS_MUST_DIE,
    THE_RIVERPAW_THREAT,
    WANTED_HOGGER,
    REPORT_TO_GRYAN,
    // Westfall
    THE_PEOPLES_MILITIA,
    THE_HARVEST_WATCHERS,
    RED_LEATHER_BANDANAS,
    RIVERPAW_GNOLL_BOUNTY,
    THE_DEFIAS_BROTHERHOOD,
    // The Deadmines
    RED_SILK_BANDANAS,
    SNEEDS_SHREDDER,
    EDWIN_VANCLEEF,
    // Stormwind
    THE_ROAD_TO_STORMWIND,
    AN_AUDIENCE_WITH_THE_HIGHLORD,
    LOST_TREASURES,
    // The Stockade
    THE_UNSENT_LETTER,
    THE_STOCKADE_RIOTS,
    BAZIL_THREDD,
    // Beast Mastery
    TAMING_THE_BEAST,
    COUNT
};

}

#endif
