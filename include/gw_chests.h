#ifndef GW_CHESTS_H
#define GW_CHESTS_H

#include "bn_camera_ptr.h"
#include "bn_fixed_point.h"
#include "bn_optional.h"
#include "bn_sprite_ptr.h"
#include "bn_vector.h"

#include "gw_maps.h"

namespace gw
{

class hud;

// The treasure chests of the current map. A closed chest twinkles now and then, even under tree tops
// and rock, so it can be spotted from nearby. Opening one hands out coins and gear for its level and
// is saved: every chest opens once per character. A chest of level 0 is a brazier instead: lighting
// it gives nothing, but all of a map's braziers lit open its sealed doors.
class chests
{

public:
    static constexpr int max_map_chests = 8;

    explicit chests(const bn::camera_ptr& camera);

    void load(const map_info& map);

    // Drops the sprites of the whole map, for a full-screen scene that needs the sprite palettes; the
    // next update makes them again for those near the player.
    void release_sprites();

    void update(const bn::fixed_point& player_feet);

    // The closest closed chest within max_distance pixels; -1 if none.
    [[nodiscard]] int nearest_closed(const bn::fixed_point& from, int max_distance) const;

    [[nodiscard]] const bn::fixed_point& position(int index) const
    {
        return _chests[index].position;
    }

    [[nodiscard]] bool brazier(int index) const
    {
        return _chests[index].level == 0;
    }

    // Whether every brazier of the map is lit (true if it has none).
    [[nodiscard]] bool all_braziers_lit() const;

    // Opens the chest or lights the brazier. Returns false (and leaves a chest closed) if every bag row
    // is in use.
    bool open(int index, hud& hud_ref);

private:
    struct chest
    {
        uint16_t id = 0;
        uint8_t level = 1;
        int8_t frame = -1;          // of fx_chest, while it has a sprite
        bn::fixed_point position;
        bn::optional<bn::sprite_ptr> sprite;
        bn::optional<bn::sprite_ptr> sparkle;
    };

    bn::camera_ptr _camera;
    bn::vector<chest, max_map_chests> _chests;
    int _frame = 0;

    void _update_sprite(chest& item, const bn::fixed_point& player_feet);
};

}

#endif
