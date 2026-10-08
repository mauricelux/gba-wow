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
// is saved: every chest opens once per character.
class chests
{

public:
    static constexpr int max_map_chests = 8;

    explicit chests(const bn::camera_ptr& camera);

    void load(const map_info& map);

    void update(const bn::fixed_point& player_feet);

    // The closest closed chest within max_distance pixels; -1 if none.
    [[nodiscard]] int nearest_closed(const bn::fixed_point& from, int max_distance) const;

    [[nodiscard]] const bn::fixed_point& position(int index) const
    {
        return _chests[index].position;
    }

    // Opens the chest. Returns false (and leaves it closed) if the bags are full.
    bool open(int index, hud& hud_ref);

private:
    struct chest
    {
        uint16_t id = 0;
        uint8_t level = 1;
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
