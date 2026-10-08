#ifndef GW_ENEMY_DATA_H
#define GW_ENEMY_DATA_H

#include "gw_ids.h"
#include "gw_look_ids.h"

namespace gw
{

namespace enemy_flag
{
    constexpr uint8_t ELITE = 1;
    constexpr uint8_t BOSS = 2;
    constexpr uint8_t PASSIVE = 4;      // only fights back
    constexpr uint8_t FAST = 8;         // runs faster than the player walks
    constexpr uint8_t NO_RESPAWN = 16;  // stays dead until the map is reloaded
}

struct enemy_def
{
    const char* name;
    look_id look;
    uint8_t min_level;
    uint8_t max_level;
    uint8_t flags;
    uint16_t health_percent;    // of the standard health for its level
    uint8_t damage_percent;     // of the standard damage for its level
    uint8_t attack_speed;       // tenths of a second between swings
    uint8_t scale_percent;      // sprite size
    uint8_t respawn_seconds;
    uint8_t loot_table;         // index into the loot tables (gw_loot)
};

[[nodiscard]] const enemy_def& get_enemy_def(enemy_id enemy);

// Standard health and damage per swing for an enemy of the level.
[[nodiscard]] int enemy_base_health(int level);

[[nodiscard]] int enemy_base_damage(int level);

}

#endif
