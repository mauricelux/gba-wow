#include "gw_game.h"

#include "bn_bg_palettes.h"
#include "bn_display.h"
#include "bn_keypad.h"
#include "bn_math.h"
#include "bn_sprite_palettes.h"
#include "bn_string.h"

#include "common_variable_8x16_sprite_font.h"

#include "gw_audio.h"
#include "gw_character.h"
#include "gw_fade.h"
#include "gw_input.h"
#include "gw_npc_data.h"
#include "gw_quests.h"
#include "gw_save.h"
#include "gw_ui.h"
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
    constexpr int loot_range = 24;
    constexpr int talk_range = 32;

    constexpr int death_gray_frames = 60;
    constexpr int death_wait_frames = 150;
    constexpr int death_fade_frames = 30;

    [[nodiscard]] bn::fixed_point saved_position()
    {
        return bn::fixed_point(character().x, character().y);
    }
}

game::game() :
    _camera(bn::camera_ptr::create(0, 0)),
    _player(player_look(character().race, character().player_class), saved_position(), _camera),
    _text_generator(common::variable_8x16_sprite_font),
    _banner(_text_generator),
    _texts(_camera),
    _effects(_camera),
    _enemies(_camera),
    _npcs(_camera),
    _chests(_camera),
    _combat(_player, _enemies, _texts, _effects, _hud),
    _dialog(_combat, _hud, _npcs),
    _menu(_combat, _hud, _npcs)
{
    // The UI layer goes first so it owns the first palette banks and tile block.
    ui::init();
    _enemies.set_combat(_combat);
    _combat.on_kill = _on_kill;
    _combat.callback_context = this;

    if(character().play_frames > 0)
    {
        _hud.message("Welcome back!", ui::color::YELLOW);
    }

    _load_map(character().map, saved_position());
}

void game::update()
{
    input::update();
    ++character().play_frames;

    if(_update_overlays())
    {
        ui::commit();
        return;
    }

    bool warping = _warp != nullptr;
    bool dead = _combat.dead();
    bool input = ! warping && ! dead;
    bool abilities_held = bn::keypad::r_held();

    if(input && ! abilities_held && bn::keypad::a_pressed())
    {
        _interact();

        if(_dialog.is_open())
        {
            ui::commit();
            return;
        }
    }

    if(input && ! abilities_held && bn::keypad::select_pressed())
    {
        _combat.quick_use();
    }

    if(input && bn::keypad::start_pressed())
    {
        _set_paused(true);
        _menu.open();
        ui::commit();
        return;
    }

    _player.update(input && ! abilities_held, ! _combat.in_combat());
    _enemies.update(_player.position(), ! dead);
    _npcs.update(_player.position());
    _chests.update(_player.position());
    _combat.update(input);

    if(warping)
    {
        _update_warp();
    }
    else if(_combat.dead())
    {
        _update_death();
    }
    else
    {
        _check_warps();
        _check_area(false);
    }

    play_music(_enemies.elite_in_combat() && ! _combat.dead() ? music_id::BOSS : world::map().music);
    _follow_camera();
    _texts.update();
    _banner.update();
    _hud.update(_combat, _enemies);
    ui::commit();
}

bool game::_update_overlays()
{
    if(_dialog.is_open())
    {
        if(! _dialog.update())
        {
            _set_paused(false);

            if(_dialog.ending_requested)
            {
                _dialog.ending_requested = false;
                _set_paused(true);
                _ending.open();
            }
        }

        return true;
    }

    if(_ending.is_open())
    {
        if(! _ending.update())
        {
            _set_paused(false);
        }

        return true;
    }

    if(_menu.is_open())
    {
        if(! _menu.update())
        {
            _set_paused(false);

            if(_menu.teleport.map != map_id::NONE)
            {
                // Reuse the door fade: a warp that leads where the debug page asked.
                _teleport = warp_def{ 0, 0, 0, 0, _menu.teleport.map, int16_t(_menu.teleport.x),
                                      int16_t(_menu.teleport.y) };
                _warp = &_teleport;
                _warp_frames = 0;
                _combat.clear_target();
            }
        }

        return true;
    }

    return false;
}

void game::_set_paused(bool paused)
{
    // The UI layer belongs to the dialog or menu while paused; the hud redraws itself after.
    _hud.set_visible(! paused);
    ui::clear();

    if(paused)
    {
        _banner.hide();
    }
}

void game::_on_kill(void* context, int index)
{
    game& self = *static_cast<game*>(context);

    switch(self._enemies.at(index).id)
    {

    case enemy_id::PRINCESS:
        set_flag(story_flag::PRINCESS_KILLED);
        break;

    case enemy_id::HOGGER:
        set_flag(story_flag::HOGGER_KILLED);
        break;

    case enemy_id::SNEED:
        set_flag(story_flag::SNEED_KILLED);
        break;

    case enemy_id::VANCLEEF:
        set_flag(story_flag::VANCLEEF_KILLED);
        self._hud.message("The Brotherhood is broken!", ui::color::YELLOW);
        break;

    case enemy_id::BAZIL_THREDD:
        set_flag(story_flag::BAZIL_KILLED);
        self._hud.message("The riot is over!", ui::color::YELLOW);
        break;

    default:
        break;
    }

    if(quests_on_kill(self._enemies.at(index).id, self._hud))
    {
        self._npcs.refresh_markers();
    }
}

void game::_interact()
{
    int npc_index = _npcs.nearest(_player.position(), talk_range);

    if(npc_index >= 0 && ! _combat.in_combat())
    {
        const npc& target = _npcs.at(npc_index);
        _player.face(target.position);
        _player.update(false, false);
        _combat.clear_target();
        _set_paused(true);
        _dialog.open(target.id);
        return;
    }

    int chest = _chests.nearest_closed(_player.position(), loot_range);

    if(chest >= 0 && ! _combat.in_combat())
    {
        _player.face(_chests.position(chest));

        if(_chests.open(chest, _hud))
        {
            if(quests_on_chest(_hud))
            {
                _npcs.refresh_markers();
            }

            save_game();
        }

        return;
    }

    int corpse = _enemies.nearest_corpse(_player.position(), loot_range);

    if(corpse >= 0)
    {
        _loot(corpse);
        return;
    }

    _combat.engage();
}

void game::_loot(int index)
{
    enemy& item = _enemies.at(index);
    character_data& data = character();

    if(item.loot_money)
    {
        data.money += item.loot_money;
        play_sound(sound_id::COIN);
        bn::string<28> text = "You loot ";
        int money = item.loot_money;

        if(money >= 100)
        {
            text += bn::to_string<6>(money / 100);
            text += "s ";
        }

        if(money % 100)
        {
            text += bn::to_string<4>(money % 100);
            text += "c";
        }
        _hud.message(text, ui::color::YELLOW);
        item.loot_money = 0;
    }

    bool full = false;

    for(loot_slot& slot : item.loot)
    {
        if(slot.item == item_id::NONE)
        {
            continue;
        }

        int left = add_item(slot.item, slot.count);

        if(left == slot.count)
        {
            full = true;
            continue;
        }

        bn::string<28> text = get_item(slot.item).name;

        if(slot.count - left > 1)
        {
            text += " x";
            text += bn::to_string<4>(slot.count - left);
        }

        _hud.message(text, ui::color(quality_color(get_item(slot.item).quality)));
        slot.count = left;

        if(left == 0)
        {
            slot.item = item_id::NONE;
        }
        else
        {
            full = true;
        }
    }

    if(full)
    {
        _hud.message("Inventory is full", ui::color::RED);
    }

    if(! item.has_loot())
    {
        _enemies.corpse_looted(index);
    }
}

void game::_load_map(map_id map, const bn::fixed_point& position)
{
    // Free the old map's VRAM before loading the new one.
    _ground.reset();
    _overhead.reset();
    _combat.on_map_change();

    const map_info& info = get_map(map);
    world::set_map(info);
    character().map = map;

    _ground = info.ground.create_bg(0, 0);
    _ground->set_priority(ground_priority);
    _ground->set_camera(_camera);

    _overhead = info.overhead.create_bg(0, 0);
    _overhead->set_priority(overhead_priority);
    _overhead->set_camera(_camera);

    _enemies.load(info);
    _npcs.load(info);
    _chests.load(info);
    _player.set_position(position);
    _save_position();
    _follow_camera();
    _area = nullptr;
    _check_area(true);
    save_game();
}

void game::_save_position()
{
    character().x = _player.position().x().integer();
    character().y = _player.position().y().integer();
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

void game::_update_death()
{
    ++_death_frames;

    if(_death_frames == 1)
    {
        _hud.message("You have died", ui::color::RED);
    }

    if(_death_frames <= death_gray_frames)
    {
        bn::fixed intensity = bn::fixed(_death_frames) / death_gray_frames;
        bn::bg_palettes::set_grayscale_intensity(intensity);
        bn::sprite_palettes::set_grayscale_intensity(intensity);
    }
    else if(_death_frames > death_wait_frames && _death_frames <= death_wait_frames + death_fade_frames)
    {
        set_fade(bn::fixed(_death_frames - death_wait_frames) / death_fade_frames);

        if(_death_frames == death_wait_frames + death_fade_frames)
        {
            // Back to life at the nearest graveyard.
            bn::bg_palettes::set_grayscale_intensity(0);
            bn::sprite_palettes::set_grayscale_intensity(0);
            const point_def& graveyard = nearest_graveyard(world::map(), _player.position().x().integer(),
                                                           _player.position().y().integer());
            _combat.revive();
            _load_map(character().map, bn::fixed_point(graveyard.x, graveyard.y));
            _death_frames = 0;
            _warp_frames = warp_fade_frames;
            _warp = &world::map().warps[0];
            _hud.message("You return to life", ui::color::GREEN);
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
    _save_position();

    const area_def* area = area_at(world::map(), _player.position().x().floor_integer(),
                                   _player.position().y().floor_integer());

    if(area && area != _area && (force || ! _area || area->name != _area->name))
    {
        _banner.show(area->name);
    }

    if(quests_on_explore(world::map(), _player.position().x().floor_integer(),
                         _player.position().y().floor_integer(), _hud))
    {
        _npcs.refresh_markers();
    }

    _area = area;
}

}
