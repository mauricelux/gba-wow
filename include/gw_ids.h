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
    REDRIDGE,
    DUSKWOOD,
    SILVERPINE,
    SHADOWFANG,
    IRONFORGE,
    DUN_MOROGH,
    WETLANDS,
    DARKSHORE,
    BLACKFATHOM_DEEPS,
    GNOMEREGAN,
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
    BOSS,
    REDRIDGE,
    DUSKWOOD,
    IRONFORGE,
    WETLANDS
};

enum class area_id : uint8_t
{
    NONE,
    FARGODEEP,
    IRONCLAD_COVE,
    ECHO_RIDGE,
    MOONBROOK,
    LAKE_EVERSTILL,     // fishing waters
    MISTMANTLE_MANOR,
    THANDOL_SPAN,
    RADIATION           // fallout that burns whoever stands in it; unnamed, under a named room
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
    // Redridge
    SOLOMON,
    MARRIS,
    OSLOW,
    BAREN,
    BREANNA,
    BRIANNA,
    OSGOOD,
    BRAY,
    BERTON,
    ARIENA,
    KAREN,
    GUARD_LAKERIDGE,
    // Duskwood
    ELLO,
    ALTHEA,
    EVA,
    SIRRA,
    CALOR,
    TRELAYNE,
    FELICIA,
    NIGHT_WATCH_GATE,
    NIGHT_WATCH_SQUARE,
    DARKSHIRE_VENDOR,
    SVEN,
    ABERCROMBIE,
    // Silverpine Forest
    VALDAN,
    GRYPHON_SILVERPINE,
    // Ironforge
    MAGNI,
    IRONFORGE_GUARD_SEAT,
    GERRIG,
    IRONFORGE_GUARD_GATE,
    GRYTH,
    FIREBREW,
    IRONFORGE_VENDOR,
    IRONFORGE_SMITH,
    MEKKATORQUE,
    SHONI,
    KELSTRUM,
    OLMIN,
    JULI,
    // Dun Morogh
    IRONFORGE_GUARD_OUTSIDE,
    OZZIE,
    MOUNTAINEER,
    // Wetlands
    STOUTFIST,
    HELBREK,
    MENETHIL_VENDOR,
    SHELLEI,
    HALLORAN,
    HARBORMASTER,
    MENETHIL_GUARD,
    WHELGAR,
    ORMER,
    // Darkshore
    SHAUSSIY,
    SHAEDLASS,
    CAYLAIS,
    SENTINEL,
    // Blackfathom Deeps
    THAELRID,
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
    // Redridge
    REDRIDGE_MONGREL,
    SHADOWHIDE_GNOLL,
    SHADOWHIDE_MYSTIC,
    RIBCHASER,
    MURLOC_FLESHEATER,
    GREAT_GORETUSK,
    BELLYGRUB,
    TARANTULA,
    BLACKROCK_OUTRUNNER,
    BLACKROCK_GRUNT,
    BLACKROCK_SHADOWCASTER,
    BLACKROCK_RENEGADE,
    BLACKROCK_SUMMONER,
    GATH_ILZOGG,
    // Duskwood
    DIRE_WOLF,
    RABID_DIRE_WOLF,
    VENOM_WEB_SPIDER,
    NIGHTBANE_DARK_RUNNER,
    NIGHTBANE_SHADOW_WEAVER,
    NIGHTBANE_TAINTED_ONE,
    SKELETAL_WARRIOR,
    SKELETAL_MAGE,
    SKELETAL_SERVANT,
    ROTTING_GHOUL,
    PLAGUE_SPREADER,
    SPLINTER_FIST_OGRE,
    SPLINTER_FIST_TASKMASTER,
    MOR_LADIM,
    STALVAN_MISTMANTLE,
    MORBENT_FEL,
    STITCHES,
    // Silverpine Forest and Shadowfang Keep
    BLEAK_WORG,
    SHADOWFANG_MOONWALKER,
    SHADOWFANG_DARKCASTER,
    SHADOWFANG_WOLFGUARD,
    HAUNTED_SERVITOR,
    WAILING_GUARDSMAN,
    RETHILGORE,
    RAZORCLAW_THE_BUTCHER,
    BARON_SILVERLAINE,
    COMMANDER_SPRINGVALE,
    ODO_THE_BLINDWATCHER,
    ARUGAL,
    LUPINE_HORROR,
    // Wetlands
    YOUNG_CROCOLISK,
    GIANT_CROCOLISK,
    MOTTLED_RAPTOR,
    MOTTLED_SCREECHER,
    SARLTOOTH,
    MOSSHIDE_GNOLL,
    MOSSHIDE_MYSTIC,
    BLUEGILL_MURLOC,
    DARK_IRON_DWARF,
    DARK_IRON_SABOTEUR,
    BALGARAS_THE_FOUL,
    DRAGONMAW_GRUNT,
    DRAGONMAW_SHADOWWARDER,
    NEK_ROSH,
    // Blackfathom Deeps
    BLACKFATHOM_MYRMIDON,
    BLACKFATHOM_TIDE_PRIESTESS,
    AKU_MAI_SNAPJAW,
    BLINDLIGHT_MURLOC,
    TWILIGHT_ACOLYTE,
    TWILIGHT_REAVER,
    AKU_MAI_SERVANT,
    GHAMOO_RA,
    LADY_SAREVESS,
    GELIHAST,
    TWILIGHT_LORD_KELRIS,
    AKU_MAI,
    // Gnomeregan
    LEPER_GNOME,
    IRRADIATED_PILLAGER,
    CAVERNDEEP_BURROWER,
    IRRADIATED_SLIME,
    DARK_IRON_AGENT,
    MECHANO_TANK,
    ARCANE_NULLIFIER,
    WALKING_BOMB,
    GRUBBIS,
    VISCOUS_FALLOUT,
    ELECTROCUTIONER_6000,
    CROWD_PUMMELER,
    MEKGINEER_THERMAPLUGG,
    COUNT
};

}

#endif
