#include "gw_enemy_data.h"

namespace gw
{

namespace
{
    using l = look_id;
    using namespace enemy_flag;

    // name, look, min level, max level, flags, health %, damage %, attack speed, scale %, respawn s, loot
    constexpr enemy_def enemies[] = {
        { "", l::YOUNG_WOLF, 1, 1, PASSIVE, 100, 100, 20, 100, 60, 0 },
        { "Young Wolf", l::YOUNG_WOLF, 1, 2, FAST, 90, 90, 20, 85, 45, 1 },
        { "Kobold Vermin", l::KOBOLD_VERMIN, 2, 3, 0, 100, 100, 20, 100, 45, 2 },
        { "Defias Thug", l::DEFIAS_THUG, 3, 4, 0, 105, 100, 20, 100, 60, 3 },
        { "Timber Wolf", l::TIMBER_WOLF, 5, 6, FAST, 100, 100, 20, 100, 60, 1 },
        { "Forest Spider", l::FOREST_SPIDER, 5, 7, 0, 95, 105, 18, 90, 60, 4 },
        { "Kobold Tunneler", l::KOBOLD_TUNNELER, 6, 8, 0, 100, 100, 20, 100, 60, 5 },
        { "Murloc Forager", l::MURLOC, 7, 9, 0, 100, 100, 20, 100, 60, 6 },
        { "Rockhide Boar", l::BOAR, 7, 8, PASSIVE, 110, 100, 20, 100, 60, 7 },
        { "Princess", l::PRINCESS, 9, 9, ELITE, 260, 140, 20, 140, 240, 8 },
        { "Riverpaw Gnoll", l::RIVERPAW_GNOLL, 8, 10, 0, 105, 100, 22, 100, 60, 9 },
        { "Hogger", l::HOGGER, 11, 11, ELITE, 320, 160, 22, 145, 240, 10 },
        { "Harvest Watcher", l::HARVEST_WATCHER, 12, 14, 0, 115, 110, 26, 110, 60, 11 },
        { "Defias Trapper", l::DEFIAS_TRAPPER, 11, 13, 0, 100, 100, 20, 100, 60, 12 },
        { "Defias Smuggler", l::DEFIAS_SMUGGLER, 12, 14, 0, 100, 105, 20, 100, 60, 12 },
        { "Riverpaw Brute", l::GNOLL_BRUTE, 14, 16, 0, 115, 110, 24, 110, 60, 9 },
        { "Defias Miner", l::DEFIAS_MINER, 16, 17, 0, 130, 105, 20, 100, 90, 13 },
        { "Goblin Engineer", l::GOBLIN_ENGINEER, 17, 18, 0, 125, 110, 20, 100, 90, 14 },
        { "Sneed", l::SNEED, 19, 19, ELITE | BOSS | NO_RESPAWN, 260, 130, 20, 160, 0, 15 },
        { "Defias Pirate", l::DEFIAS_PIRATE, 18, 19, 0, 130, 110, 20, 100, 90, 13 },
        { "Edwin VanCleef", l::VANCLEEF, 21, 21, ELITE | BOSS | NO_RESPAWN, 330, 110, 18, 140, 0, 16 },
        { "Blackguard", l::DEFIAS_BLACKGUARD, 19, 19, NO_RESPAWN, 70, 75, 20, 100, 0, 0 },
    };

    static_assert(sizeof(enemies) / sizeof(enemies[0]) == int(enemy_id::COUNT));
}

const enemy_def& get_enemy_def(enemy_id enemy)
{
    return enemies[int(enemy)];
}

int enemy_base_health(int level)
{
    return (30 + 10 * level + level * level) * 65 / 100;
}

int enemy_base_damage(int level)
{
    return 3 + level * 3 / 2;
}

}
