#include "gw_loot.h"

#include "gw_enemies.h"
#include "gw_types.h"

namespace gw
{

void roll_loot(enemy& item)
{
    int level = item.level;
    item.loot_money = random_range(level * 2, level * 5);

    if(item.elite())
    {
        item.loot_money *= 4;
    }
}

}
