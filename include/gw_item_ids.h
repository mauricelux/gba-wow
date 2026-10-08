#ifndef GW_ITEM_IDS_H
#define GW_ITEM_IDS_H

#include <cstdint>

namespace gw
{

// Keep in the same order as the table in gw_items.cpp. Saves store these values: only append.
enum class item_id : uint8_t
{
    NONE,
    // starting gear
    WORN_SHORTSWORD,
    BENT_STAFF,
    WORN_HATCHET,
    CRACKED_SHORTBOW,
    RECRUITS_VEST,
    RECRUITS_PANTS,
    RECRUITS_BOOTS,
    APPRENTICES_ROBE,
    APPRENTICES_PANTS,
    APPRENTICES_BOOTS,
    TRAPPERS_VEST,
    TRAPPERS_PANTS,
    TRAPPERS_BOOTS,
    COUNT
};

}

#endif
