#ifndef GW_LOOT_H
#define GW_LOOT_H

#include "gw_item_ids.h"

namespace gw
{

struct enemy;

// Fills the enemy's loot slots and money from its loot table when it dies.
void roll_loot(enemy& item);

// An uncommon item of the level's band, the kind any enemy may drop (treasure chests always hold one).
[[nodiscard]] item_id roll_world_drop(int level);

}

#endif
