#include "gw_game.h"

#include "bn_display.h"
#include "bn_math.h"

#include "common_variable_8x16_sprite_font.h"

#include "gw_fade.h"
#include "gw_world.h"

namespace gw
{

namespace
{
    // Background priorities: lower numbers are drawn on top. Characters use priority 2.
    constexpr int ground_priority = 3;
    constexpr int overhead_priority = 1;

    constexpr int warp_fade_frames = 16;
    constexpr int area_check_interval = 15;
}

game::game(map_id map, const bn::fixed_point& position) :
    _camera(bn::camera_ptr::create(0, 0)),
    _player(position, _camera),
    _text_generator(common::variable_8x16_sprite_font),
    _banner(_text_generator)
{
    _load_map(map, position);
}

void game::update()
{
    bool warping = _warp != nullptr;
    _player.update(! warping);

    if(warping)
    {
        _update_warp();
    }
    else
    {
        _check_warps();
        _check_area(false);
    }

    _follow_camera();
    _banner.update();
}

void game::_load_map(map_id map, const bn::fixed_point& position)
{
    // Free the old map's VRAM before loading the new one.
    _ground.reset();
    _overhead.reset();

    const map_info& info = get_map(map);
    world::set_map(info);

    _ground = info.ground.create_bg(0, 0);
    _ground->set_priority(ground_priority);
    _ground->set_camera(_camera);

    _overhead = info.overhead.create_bg(0, 0);
    _overhead->set_priority(overhead_priority);
    _overhead->set_camera(_camera);

    _player.set_position(position);
    _follow_camera();
    _area = nullptr;
    _check_area(true);
}

void game::_follow_camera()
{
    int max_x = (world::width() - bn::display::width()) / 2;
    int max_y = (world::height() - bn::display::height()) / 2;

    bn::fixed_point target = world::to_screen_space(_player.position());
    int x = bn::clamp(target.x().floor_integer(), -max_x, max_x);
    int y = bn::clamp(target.y().floor_integer(), -max_y, max_y);
    _camera.set_position(x, y);
}

void game::_check_warps()
{
    // Doors sit in walls the feet can't reach, so test the whole hitbox against the warp.
    int x = _player.position().x().floor_integer();
    int y = _player.position().y().floor_integer();
    int left = x + player::hitbox_left;
    int right = x + player::hitbox_right;
    int top = y + player::hitbox_top - 1;
    int bottom = y + 1;

    for(const warp_def& warp : world::map().warps)
    {
        if(right >= warp.x && bottom >= warp.y && left < warp.x + warp.width && top < warp.y + warp.height)
        {
            _warp = &warp;
            _warp_frames = 0;
            return;
        }
    }
}

void game::_update_warp()
{
    ++_warp_frames;

    if(_warp_frames <= warp_fade_frames)
    {
        set_fade(bn::fixed(_warp_frames) / warp_fade_frames);

        if(_warp_frames == warp_fade_frames)
        {
            _load_map(_warp->target, bn::fixed_point(_warp->target_x, _warp->target_y));
        }
    }
    else
    {
        int frames_in = _warp_frames - warp_fade_frames;
        set_fade(bn::fixed(warp_fade_frames - frames_in) / warp_fade_frames);

        if(frames_in == warp_fade_frames)
        {
            _warp = nullptr;
        }
    }
}

void game::_check_area(bool force)
{
    if(! force && --_area_check_frames > 0)
    {
        return;
    }

    _area_check_frames = area_check_interval;

    const area_def* area = area_at(world::map(), _player.position().x().floor_integer(),
                                   _player.position().y().floor_integer());

    if(area && area != _area && (force || ! _area || area->name != _area->name))
    {
        _banner.show(area->name);
    }

    _area = area;
}

}
