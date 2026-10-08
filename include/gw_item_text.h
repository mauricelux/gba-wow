#ifndef GW_ITEM_TEXT_H
#define GW_ITEM_TEXT_H

#include "gw_item_ids.h"

namespace gw
{

class text_page;

// A few lines about the item: what it is, its armor or damage, stats, required level and how it
// compares with what is equipped in its slot.
void add_item_details(text_page& page, item_id item);

// "Sword", "Mail chest", "Food"...
[[nodiscard]] const char* item_kind_name(item_id item);

}

#endif
