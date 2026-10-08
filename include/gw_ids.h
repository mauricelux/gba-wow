#ifndef GW_IDS_H
#define GW_IDS_H

#include <cstdint>

// Identifiers shared by the generated map headers (tools/gen_world.py) and the game data.
// Keep the names in sync with the generator.

namespace gw
{

enum class map_id : uint8_t
{
    NONE,
    ELWYNN,
    ABBEY,
    INN,
    WESTFALL,
    DEADMINES,
    COUNT
};

enum class music_id : uint8_t
{
    NONE,
    TITLE,
    ELWYNN,
    TOWN,
    WESTFALL,
    DUNGEON,
    BOSS
};

enum class area_id : uint8_t
{
    NONE,
    FARGODEEP,
    IRONCLAD_COVE
};

enum class npc_id : uint8_t
{
    NONE,
    WILLEM,
    MCBRIDE,
    LLANE,
    DANIL,
    GUARD_NS,
    DUGHAN,
    LYRIA,
    CORINA,
    FARLEY,
    REMY,
    GUARD_GS,
    GUARD_WEST,
    MA_STONEFIELD,
    GRYAN,
    SALMA,
    GUARD_WF,
    COUNT
};

enum class enemy_id : uint8_t
{
    NONE,
    YOUNG_WOLF,
    KOBOLD_VERMIN,
    DEFIAS_THUG,
    TIMBER_WOLF,
    FOREST_SPIDER,
    KOBOLD_TUNNELER,
    MURLOC,
    BOAR,
    PRINCESS,
    RIVERPAW_GNOLL,
    HOGGER,
    HARVEST_WATCHER,
    DEFIAS_TRAPPER,
    DEFIAS_SMUGGLER,
    GNOLL_BRUTE,
    DEFIAS_MINER,
    GOBLIN_ENGINEER,
    SNEED,
    DEFIAS_PIRATE,
    VANCLEEF,
    DEFIAS_BLACKGUARD,
    COUNT
};

}

#endif
