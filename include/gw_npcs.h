#ifndef GW_NPCS_H
#define GW_NPCS_H

#include "bn_camera_ptr.h"
#include "bn_fixed_point.h"
#include "bn_optional.h"
#include "bn_sprite_ptr.h"
#include "bn_vector.h"

#include "gw_actor_sprite.h"
#include "gw_maps.h"
#include "gw_quests.h"

namespace gw
{

struct npc
{
    npc_id id = npc_id::NONE;
    bn::fixed_point position;
    facing direction = facing::DOWN;
    quest_marker marker = quest_marker::NONE;
    bn::optional<actor_sprite> sprite;
    bn::optional<bn::sprite_ptr> marker_sprite;
};

// The friendly characters of the current map. They turn towards the player when close and show
// quest markers above their heads. Only those near the player have sprites.
class npcs
{

public:
    static constexpr int max_npcs = 24;

    explicit npcs(const bn::camera_ptr& camera);

    void load(const map_info& map);

    // Drops the sprites of the whole map, for a full-screen scene that needs the sprite palettes; the
    // next update makes them again for those near the player.
    void release_sprites();

    void update(const bn::fixed_point& player_feet);

    // Recomputes the quest markers after quest progress changed.
    void refresh_markers();

    [[nodiscard]] int count() const
    {
        return _npcs.size();
    }

    [[nodiscard]] const npc& at(int index) const
    {
        return _npcs[index];
    }

    // The closest npc within max_distance pixels; -1 if none.
    [[nodiscard]] int nearest(const bn::fixed_point& from, int max_distance) const;

private:
    bn::camera_ptr _camera;
    bn::vector<npc, max_npcs> _npcs;
    int _frame = 0;

    void _update_sprite(npc& item, const bn::fixed_point& player_feet);
};

}

#endif
