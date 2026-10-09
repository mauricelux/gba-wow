#include "gw_enemy_data.h"

namespace gw
{

namespace
{
    using l = look_id;
    using f = enemy_family;
    using a = enemy_ability_id;
    using namespace enemy_flag;

    constexpr ai_style MELEE = ai_style::MELEE;
    constexpr ai_style RUNNER = ai_style::RUNNER;

    // name, look, min level, max level, flags, health %, damage %, attack speed, scale %, respawn s, loot,
    // family, AI style, abilities
    constexpr enemy_def enemies[] = {
        { "", l::YOUNG_WOLF, 1, 1, PASSIVE, 100, 100, 20, 100, 60, 0, f::BEAST, MELEE, {} },
        { "Young Wolf", l::YOUNG_WOLF, 1, 2, FAST, 90, 90, 20, 85, 45, 1, f::BEAST, MELEE, {} },
        { "Kobold Vermin", l::KOBOLD_VERMIN, 2, 3, 0, 100, 100, 20, 100, 45, 2, f::KOBOLD, RUNNER, {} },
        { "Defias Thug", l::DEFIAS_THUG, 3, 4, 0, 105, 100, 20, 100, 60, 3, f::DEFIAS, MELEE, { a::REND } },
        { "Timber Wolf", l::TIMBER_WOLF, 5, 6, FAST, 100, 100, 20, 100, 60, 1, f::BEAST, MELEE, {} },
        { "Forest Spider", l::FOREST_SPIDER, 5, 7, 0, 95, 105, 18, 90, 60, 4, f::BEAST, MELEE, { a::POISON } },
        { "Kobold Tunneler", l::KOBOLD_TUNNELER, 6, 8, 0, 100, 100, 20, 100, 60, 5,
          f::KOBOLD, RUNNER, { a::CANDLE_THROW } },
        { "Murloc Forager", l::MURLOC, 7, 9, 0, 100, 100, 20, 100, 60, 6, f::MURLOC, RUNNER, { a::CALL_FOR_HELP } },
        { "Rockhide Boar", l::BOAR, 7, 8, PASSIVE, 110, 100, 20, 100, 60, 7, f::BEAST, MELEE, { a::CHARGE } },
        { "Princess", l::PRINCESS, 9, 9, ELITE, 260, 140, 20, 140, 240, 8, f::BEAST, MELEE, { a::CHARGE, a::ENRAGE } },
        { "Riverpaw Gnoll", l::RIVERPAW_GNOLL, 8, 10, 0, 105, 100, 22, 100, 60, 9, f::GNOLL, RUNNER, { a::THRASH } },
        { "Hogger", l::HOGGER, 11, 11, ELITE, 300, 160, 22, 145, 240, 10,
          f::GNOLL, MELEE, { a::THRASH, a::KNOCKDOWN } },
        { "Harvest Watcher", l::HARVEST_WATCHER, 12, 14, 0, 115, 110, 26, 110, 60, 11, f::CONSTRUCT, MELEE, {} },
        { "Defias Trapper", l::DEFIAS_TRAPPER, 11, 13, 0, 100, 100, 20, 100, 60, 12, f::DEFIAS, MELEE, { a::NET } },
        { "Defias Smuggler", l::DEFIAS_SMUGGLER, 12, 14, 0, 100, 105, 20, 100, 60, 12, f::DEFIAS, MELEE, {} },
        { "Riverpaw Brute", l::GNOLL_BRUTE, 14, 16, 0, 115, 110, 24, 110, 60, 9, f::GNOLL, MELEE, { a::ENRAGE } },
        { "Defias Miner", l::DEFIAS_MINER, 16, 17, 0, 130, 105, 20, 100, 90, 13, f::DEFIAS, MELEE, {} },
        { "Goblin Engineer", l::GOBLIN_ENGINEER, 17, 18, 0, 125, 110, 20, 100, 90, 14, f::DEFIAS, MELEE, {} },
        { "Sneed", l::SNEED, 19, 19, ELITE | BOSS | NO_RESPAWN, 240, 130, 20, 160, 0, 15, f::DEFIAS, MELEE, {} },
        { "Defias Pirate", l::DEFIAS_PIRATE, 18, 19, 0, 130, 110, 20, 100, 90, 13, f::DEFIAS, MELEE, { a::CLEAVE } },
        { "Edwin VanCleef", l::VANCLEEF, 21, 21, ELITE | BOSS | NO_RESPAWN, 300, 110, 18, 140, 0, 16,
          f::DEFIAS, MELEE, {} },
        { "Blackguard", l::DEFIAS_BLACKGUARD, 19, 19, NO_RESPAWN, 70, 75, 20, 100, 0, 0, f::DEFIAS, MELEE, {} },
        { "Defias Prisoner", l::DEFIAS_PRISONER, 19, 20, 0, 120, 100, 20, 100, 90, 17, f::DEFIAS, MELEE, {} },
        { "Defias Convict", l::DEFIAS_CONVICT, 19, 20, 0, 130, 105, 20, 100, 90, 17, f::DEFIAS, MELEE, {} },
        { "Defias Insurgent", l::DEFIAS_INSURGENT, 20, 21, 0, 135, 110, 20, 100, 90, 17, f::DEFIAS, MELEE, {} },
        { "Targorr", l::TARGORR, 20, 20, ELITE | NO_RESPAWN, 260, 135, 22, 135, 0, 18,
          f::DEFIAS, MELEE, { a::CHARGE, a::MORTAL_STRIKE } },
        { "Kam Deepfury", l::KAM_DEEPFURY, 21, 21, ELITE | NO_RESPAWN, 280, 125, 22, 115, 0, 19,
          f::DEFIAS, MELEE, { a::KNOCKDOWN, a::SHIELD_WALL } },
        { "Bazil Thredd", l::BAZIL_THREDD, 21, 21, ELITE | BOSS | NO_RESPAWN, 250, 95, 18, 140, 0, 20,
          f::DEFIAS, MELEE, {} },
        { "Defias Rioter", l::DEFIAS_CONVICT, 19, 19, NO_RESPAWN, 60, 70, 20, 100, 0, 0, f::DEFIAS, MELEE, {} },
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
