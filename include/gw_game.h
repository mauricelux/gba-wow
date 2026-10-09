#ifndef GW_GAME_H
#define GW_GAME_H

#include "bn_camera_ptr.h"
#include "bn_optional.h"
#include "bn_regular_bg_ptr.h"
#include "bn_sprite_text_generator.h"

#include "gw_chests.h"
#include "gw_combat.h"
#include "gw_dialog.h"
#include "gw_effects.h"
#include "gw_enemies.h"
#include "gw_ending.h"
#include "gw_flight.h"
#include "gw_floating_text.h"
#include "gw_hud.h"
#include "gw_maps.h"
#include "gw_menu.h"
#include "gw_npcs.h"
#include "gw_pet.h"
#include "gw_player.h"
#include "gw_zone_banner.h"

namespace gw
{

// The in-game scene: the current map, the player and everything around them.
class game
{

public:
    // Starts at the character's saved map and position.
    game();

    void update();

private:
    bn::camera_ptr _camera;
    bn::optional<bn::regular_bg_ptr> _ground;
    bn::optional<bn::regular_bg_ptr> _overhead;
    player _player;
    bn::sprite_text_generator _text_generator;
    zone_banner _banner;
    floating_texts _texts;
    effects _effects;
    enemies _enemies;
    npcs _npcs;
    chests _chests;
    hud _hud;
    pet _pet;
    combat _combat;
    dialog _dialog;
    menu _menu;
    ending _ending;
    flight _flight;
    const area_def* _area = nullptr;
    const warp_def* _warp = nullptr;
    warp_def _teleport = {};
    int _warp_frames = 0;
    int _rest_frames = 0;
    int _area_check_frames = 0;
    int _death_frames = 0;

    void _load_map(map_id map, const bn::fixed_point& position);
    void _follow_camera();
    void _check_warps();
    void _update_warp();
    void _update_rest();
    [[nodiscard]] bool _near_innkeeper() const;
    void _rest(bool loaded);
    void _check_area(bool force);
    void _interact();
    bool _update_overlays();
    void _set_paused(bool paused);
    static void _on_kill(void* context, int index);
    static void _on_level_up(void* context);
    void _start_teleport(map_id map, int x, int y);
    void _land();
    void _loot(int index);
    void _update_death();
    void _save_position();
};

}

#endif
