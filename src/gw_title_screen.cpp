#include "gw_title_screen.h"

#include "bn_keypad.h"
#include "bn_string.h"

#include "bn_sprite_items_title_logo.h"

#include "gw_audio.h"
#include "gw_character.h"
#include "gw_fade.h"
#include "gw_input.h"
#include "gw_maps.h"
#include "gw_save.h"
#include "gw_ui.h"

namespace gw
{

namespace
{
    // The camera drifts back and forth over Goldshire, in world pixels.
    constexpr int pan_left = 760;
    constexpr int pan_right = 1290;
    constexpr int pan_y = 1190;
    constexpr bn::fixed pan_speed = 0.25;

    constexpr int logo_top = 6;
    constexpr int fade_in_frames = 30;
    constexpr int fade_out_frames = 20;

    constexpr int panel_x = 6;
    constexpr int panel_y = 11;
    constexpr int panel_width = 18;

    [[nodiscard]] const map_info& title_map()
    {
        return get_map(map_id::ELWYNN);
    }
}

title_screen::title_screen() :
    _camera(bn::camera_ptr::create(0, 0)),
    _ground((ui::init(), title_map().ground.create_bg(0, 0))),
    _overhead(title_map().overhead.create_bg(0, 0)),
    _pan(pan_left)
{
    _ground.set_priority(3);
    _ground.set_camera(_camera);
    _overhead.set_priority(1);
    _overhead.set_camera(_camera);

    // Six 64x32 pieces, three across and two down.
    for(int index = 0; index < 6; ++index)
    {
        int x = (index % 3 - 1) * 64;
        int y = -80 + logo_top + 16 + (index / 3) * 32;
        bn::sprite_ptr sprite = bn::sprite_items::title_logo.create_sprite(x, y, index);
        sprite.set_bg_priority(0);
        _logo.push_back(bn::move(sprite));
    }

    _has_save = load_game();
    _option = _has_save ? option::CONTINUE : option::NEW_GAME;
    set_fade(1);
    play_music(music_id::TITLE);
}

title_screen::result title_screen::update()
{
    input::update();
    ++_frame;

    // Back and forth across the village.
    _pan += pan_speed * _pan_direction;

    if(_pan >= pan_right || _pan <= pan_left)
    {
        _pan_direction = -_pan_direction;
    }

    const map_info& map = title_map();
    _camera.set_position(_pan - map.width / 2, pan_y - map.height / 2);

    if(_frame <= fade_in_frames)
    {
        set_fade(bn::fixed(fade_in_frames - _frame) / fade_in_frames);
    }

    if(_chosen != result::WAITING)
    {
        // Fading out before handing over.
        ++_fade_frames;
        set_fade(bn::fixed(_fade_frames) / fade_out_frames);

        if(_fade_frames >= fade_out_frames)
        {
            ui::clear();
            ui::commit();
            return _chosen;
        }

        ui::commit();
        return result::WAITING;
    }

    if(_confirming)
    {
        if(bn::keypad::a_pressed())
        {
            _choose(result::NEW_GAME);
        }
        else if(bn::keypad::b_pressed())
        {
            _confirming = false;
            _dirty = true;
        }
    }
    else
    {
        if(_has_save && (input::repeated(bn::keypad::key_type::UP) || input::repeated(bn::keypad::key_type::DOWN)))
        {
            _option = _option == option::CONTINUE ? option::NEW_GAME : option::CONTINUE;
            play_sound(sound_id::SELECT);
            _dirty = true;
        }

        if(bn::keypad::a_pressed() || bn::keypad::start_pressed())
        {
            if(_option == option::CONTINUE)
            {
                _choose(result::CONTINUE);
            }
            else if(_has_save)
            {
                _confirming = true;
                _dirty = true;
            }
            else
            {
                _choose(result::NEW_GAME);
            }
        }
    }

    if(_dirty)
    {
        _draw();
        _dirty = false;
    }

    ui::commit();
    return result::WAITING;
}

void title_screen::_choose(result chosen)
{
    _chosen = chosen;
    _fade_frames = 0;
    play_sound(sound_id::SELECT);
}

void title_screen::_draw()
{
    ui::clear();

    if(_confirming)
    {
        ui::panel(panel_x - 2, panel_y, panel_width + 4, 6);
        ui::text_center(panel_y + 1, "Start a new hero?", ui::color::YELLOW, true);
        ui::text_center(panel_y + 2, "Your saved hero", ui::color::WHITE, true);
        ui::text_center(panel_y + 3, "will be lost.", ui::color::WHITE, true);
        ui::text(panel_x, panel_y + 4, "A Yes", ui::color::WHITE, true);
        ui::text_right(panel_x + panel_width - 1, panel_y + 4, "B No", ui::color::GRAY, true);
    }
    else if(_has_save)
    {
        const character_data& data = character();
        ui::panel(panel_x, panel_y, panel_width, 7);

        bool on_continue = _option == option::CONTINUE;
        ui::cursor(panel_x + 2, on_continue ? panel_y + 1 : panel_y + 2);
        ui::text(panel_x + 4, panel_y + 1, "Continue", on_continue ? ui::color::WHITE : ui::color::GRAY, true);
        ui::text(panel_x + 4, panel_y + 2, "New Game", on_continue ? ui::color::GRAY : ui::color::WHITE, true);
        ui::divider(panel_x + 1, panel_y + 3, panel_width - 2);

        bn::string<24> line = race_name(data.race);
        line += " ";
        line += class_name(data.player_class);
        ui::text(panel_x + 2, panel_y + 4, line, ui::color::YELLOW, true);

        line = "Level ";
        line += bn::to_string<4>(data.level);
        ui::text(panel_x + 2, panel_y + 5, line, ui::color::WHITE, true);

        int minutes = int(data.play_frames / 3600);
        line = bn::to_string<6>(minutes / 60);
        line += "h ";
        line += bn::to_string<4>(minutes % 60);
        line += "m";
        ui::text_right(panel_x + panel_width - 3, panel_y + 5, line, ui::color::GRAY, true);
    }
    else
    {
        ui::panel(panel_x + 2, panel_y + 1, panel_width - 4, 3);
        ui::cursor(panel_x + 4, panel_y + 2);
        ui::text(panel_x + 6, panel_y + 2, "New Game", ui::color::WHITE, true);
    }

    ui::text_center(19, "A fan-made adventure", ui::color::GRAY);
}

}
