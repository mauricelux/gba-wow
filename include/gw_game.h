#ifndef GW_GAME_H
#define GW_GAME_H

#include "bn_camera_ptr.h"
#include "bn_optional.h"
#include "bn_regular_bg_ptr.h"
#include "bn_sprite_text_generator.h"

#include "gw_maps.h"
#include "gw_player.h"
#include "gw_zone_banner.h"

namespace gw
{

// The in-game scene: the current map, the player and everything around them.
class game
{

public:
    game(map_id map, const bn::fixed_point& position);

    void update();

private:
    bn::camera_ptr _camera;
    bn::optional<bn::regular_bg_ptr> _ground;
    bn::optional<bn::regular_bg_ptr> _overhead;
    player _player;
    bn::sprite_text_generator _text_generator;
    zone_banner _banner;
    const area_def* _area = nullptr;
    const warp_def* _warp = nullptr;
    int _warp_frames = 0;
    int _area_check_frames = 0;

    void _load_map(map_id map, const bn::fixed_point& position);
    void _follow_camera();
    void _check_warps();
    void _update_warp();
    void _check_area(bool force);
};

}

#endif
