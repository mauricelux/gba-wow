#ifndef GW_VENDORS_H
#define GW_VENDORS_H

#include "bn_span.h"

#include "gw_item_ids.h"

namespace gw
{

// What a vendor sells (gw::npc_info::vendor).
[[nodiscard]] bn::span<const item_id> vendor_stock(int vendor);

}

#endif
