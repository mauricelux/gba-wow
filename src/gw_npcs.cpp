#include "gw_npcs.h"

#include "bn_math.h"

#include "bn_sprite_items_fx_markers.h"

#include "gw_character.h"
#include "gw_npc_data.h"
#include "gw_sprite_palettes.h"
#include "gw_world.h"

namespace gw
{

namespace
{
    constexpr int sprite_margin_x = 160;
    constexpr int sprite_margin_y = 120;
    constexpr int face_player_distance = 48;
    constexpr int marker_bg_priority = 1;

    // Lady Prestor stands beside Bolvar until the masquerade ends.
    [[nodiscard]] bool present(npc_id id)
    {
        return id != npc_id::PRESTOR ||
               quest_state(quest_id::THE_GREAT_MASQUERADE).status != quest_status::TURNED_IN;
    }

    [[nodiscard]] int marker_frame(quest_marker marker)
    {
        switch(marker)
        {

        case quest_marker::AVAILABLE:
            return 0;

        case quest_marker::COMPLETE:
            return 1;

        case quest_marker::TRAINER:
            return 5;

        default:
            return 2;
        }
    }
}

npcs::npcs(const bn::camera_ptr& camera) :
    _camera(camera)
{
}

void npcs::release_sprites()
{
    for(npc& item : _npcs)
    {
        item.sprite.reset();
        item.marker_sprite.reset();
    }
}

void npcs::load(const map_info& map)
{
    _npcs.clear();

    for(const npc_def& def : map.npcs)
    {
        if(def.npc == npc_id::NONE || _npcs.full() || ! present(def.npc))
        {
            continue;
        }

        _npcs.emplace_back();
        npc& item = _npcs.back();
        item.id = def.npc;
        item.position = bn::fixed_point(def.x, def.y);
    }

    refresh_markers();
}

void npcs::refresh_markers()
{
    // Quests can send an npc away for good.
    for(auto it = _npcs.begin(); it != _npcs.end();)
    {
        if(present(it->id))
        {
            ++it;
        }
        else
        {
            it = _npcs.erase(it);
        }
    }

    for(npc& item : _npcs)
    {
        quest_marker marker = npc_quest_marker(item.id);
        const npc_info& info = get_npc_info(item.id);

        if(marker == quest_marker::NONE && (info.flags & npc_flag::TRAINER) && teaches(info.trainer_class) &&
           trainable_count(info.trainer_class) > 0)
        {
            marker = quest_marker::TRAINER;
        }

        if(marker != item.marker)
        {
            item.marker = marker;
            item.marker_sprite.reset();
        }
    }
}

void npcs::update(const bn::fixed_point& player_feet)
{
    ++_frame;

    for(npc& item : _npcs)
    {
        // Turn towards the player when they come close, back to the default when they leave.
        int distance = distance_squared(item.position, player_feet);
        item.direction = distance < face_player_distance * face_player_distance ?
                    facing_towards(item.position, player_feet) : facing::DOWN;
        _update_sprite(item, player_feet);
    }
}

int npcs::nearest(const bn::fixed_point& from, int max_distance) const
{
    int best = -1;
    int best_distance = max_distance * max_distance + 1;

    for(int index = 0, limit = _npcs.size(); index < limit; ++index)
    {
        int distance = distance_squared(_npcs[index].position, from);

        if(distance < best_distance)
        {
            best = index;
            best_distance = distance;
        }
    }

    return best;
}

void npcs::_update_sprite(npc& item, const bn::fixed_point& player_feet)
{
    bool near = bn::abs(item.position.x() - player_feet.x()) < sprite_margin_x &&
                bn::abs(item.position.y() - player_feet.y()) < sprite_margin_y;

    if(! near)
    {
        item.sprite.reset();
        item.marker_sprite.reset();
        return;
    }

    if(! item.sprite)
    {
        look_id look = get_npc_info(item.id).look;

        if(! sprite_palettes::npc_fits(get_look(look).palette))
        {
            return;
        }

        item.sprite.emplace(look, _camera);
    }

    item.sprite->update(item.position, item.direction, false, 0);

    if(item.marker == quest_marker::NONE)
    {
        return;
    }

    if(! item.marker_sprite)
    {
        if(! sprite_palettes::fits(bn::sprite_items::fx_markers.palette_item()))
        {
            return;
        }

        item.marker_sprite = bn::sprite_items::fx_markers.create_sprite(0, 0, marker_frame(item.marker));
        item.marker_sprite->set_camera(_camera);
        item.marker_sprite->set_bg_priority(marker_bg_priority);
    }

    // A gentle bob above the head.
    int bob = ((_frame >> 4) & 1);
    bn::fixed_point screen = world::to_screen_space(item.position);
    item.marker_sprite->set_position(screen.x().floor_integer(),
                                     screen.y().floor_integer() - item.sprite->height() - 8 - bob);
}

}
