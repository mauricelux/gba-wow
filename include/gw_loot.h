#ifndef GW_LOOT_H
#define GW_LOOT_H

namespace gw
{

struct enemy;

// Fills the enemy's loot slots and money from its loot table when it dies.
void roll_loot(enemy& item);

}

#endif
