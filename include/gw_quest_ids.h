#ifndef GW_QUEST_IDS_H
#define GW_QUEST_IDS_H

#include <cstdint>

namespace gw
{

// Keep in the same order as the table in gw_quests.cpp. Saves index quest progress by these values:
// only append, and keep COUNT within gw::max_quests.
enum class quest_id : uint8_t
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
    COUNT
};

}

#endif
