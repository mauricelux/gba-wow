#ifndef GW_IDS_H
#define GW_IDS_H

#include <cstdint>

// Identifiers shared by the generated map headers (tools/gen_world.py) and the game data.
// Keep the names in sync with the generator. Saves store map ids by value: only append. The world
// will need more than 255 maps, npcs and enemy types, so those ids are 16 bits wide.

namespace gw
{

enum class race_id : uint8_t
{
    HUMAN,
    DWARF,
    NIGHT_ELF,
    COUNT
};

enum class class_id : uint8_t
{
    WARRIOR,
    MAGE,
    HUNTER,
    COUNT
};

// Three per class, in the order of the class's talent trees. Saves store these values; NONE is a
// character from before subclasses, who picks one when the save loads.
enum class subclass_id : uint8_t
{
    NONE,
    ARMS,
    FURY,
    PROTECTION,
    ARCANE,
    FIRE,
    FROST,
    BEAST_MASTERY,
    MARKSMANSHIP,
    SURVIVAL,
    COUNT
};

constexpr int subclasses_per_class = 3;

[[nodiscard]] constexpr class_id subclass_class(subclass_id subclass)
{
    return class_id((int(subclass) - 1) / subclasses_per_class);
}

// The subclass's place among its class's three, which is also its talent tree.
[[nodiscard]] constexpr int subclass_index(subclass_id subclass)
{
    return (int(subclass) - 1) % subclasses_per_class;
}

[[nodiscard]] constexpr subclass_id class_subclass(class_id player_class, int index)
{
    return subclass_id(1 + int(player_class) * subclasses_per_class + index);
}

enum class map_id : uint16_t
{
    NONE,
    ELWYNN,
    ABBEY,
    INN,
    WESTFALL,
    DEADMINES,
    ECHO_RIDGE,
    FARGODEEP,
    STORMWIND,
    STOCKADE,
    DEEPRUN_TRAM,
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
    IRONCLAD_COVE,
    ECHO_RIDGE,
    MOONBROOK
};

enum class npc_id : uint16_t
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
    KHELDEN,
    THORGAS,
    EAGAN,
    MILLY,
    ANDREW,
    FURLBROW,
    LEWIS,
    HEATHER,
    MARCUS_JONATHAN,
    SW_GUARD_GATE,
    SW_GUARD_KEEP,
    SW_GUARD_TRADE,
    BOLVAR,
    BENEDICTUS,
    ALLISON,
    GUNTHER,
    LINA,
    THURMAN,
    JENNEA,
    EINRIS,
    BRANN,
    ANDER,
    THELWATER,
    SW_GUARD_STOCKADE,
    DUNGAR,
    THOR,
    RANDAL,
    MONTY,
    COUNT
};

enum class enemy_id : uint16_t
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
    DEFIAS_PRISONER,
    DEFIAS_CONVICT,
    DEFIAS_INSURGENT,
    TARGORR,
    KAM_DEEPFURY,
    BAZIL_THREDD,
    DEFIAS_RIOTER,
    COUNT
};

}

#endif
