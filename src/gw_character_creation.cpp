#include "gw_character_creation.h"

#include "bn_keypad.h"
#include "bn_string.h"

#include "gw_character.h"
#include "gw_input.h"
#include "gw_save.h"
#include "gw_types.h"
#include "gw_ui.h"

namespace gw
{

namespace
{
    constexpr const char* race_texts[] = {
        "The adaptable folk of Stormwind. +2 Spirit.",
        "Stout folk of Ironforge. +2 Strength, +3 Stamina, -4 Agility.",
        "Graceful elves of Teldrassil. +5 Agility, -3 Strength.",
    };

    constexpr const char* class_texts[] = {
        "Fights up close in mail and builds rage by dealing and taking blows.",
        "Hurls fire, frost and arcane spells from afar. Wears cloth and uses mana.",
        "Shoots with a bow or gun and wears leather. Uses mana for special shots.",
    };

    constexpr facing turn_order[] = { facing::DOWN, facing::LEFT, facing::UP, facing::RIGHT };

    constexpr int list_x = 4;
    constexpr int race_top = 4;
    constexpr int class_top = 9;
    constexpr int stats_x = 16;
    constexpr int stats_top = 10;
    constexpr int subclass_x = 15;
    constexpr int subclass_top = 9;
    constexpr int description_top = 14;
    constexpr int hint_row = 18;

    // The preview's feet, in screen coordinates from the center.
    constexpr int preview_x = 60;
    constexpr int preview_y = -12;
    constexpr int preview_scale = 2;
    constexpr int turn_frames = 90;
}

character_creation::character_creation(bool subclass_only) :
    _camera(bn::camera_ptr::create(0, 0)),
    _subclass_only(subclass_only)
{
    ui::init();

    if(subclass_only)
    {
        const character_data& data = character();
        subclass_id suggested = suggested_subclass();
        _race = data.race;
        _class = data.player_class;
        _step = step::SUBCLASS;
        _subclass = suggested != subclass_id::NONE && subclass_class(suggested) == _class ?
                    subclass_index(suggested) : 0;
    }

    _refresh();
}

character_creation::result character_creation::update()
{
    input::update();
    ++_frame;

    bool up = input::repeated(bn::keypad::key_type::UP);
    bool down = input::repeated(bn::keypad::key_type::DOWN);

    switch(_step)
    {

    case step::RACE:
        if(up || down)
        {
            int count = int(race_id::COUNT);
            _race = race_id((int(_race) + count + (down ? 1 : -1)) % count);

            if(! class_allowed(_race, _class))
            {
                _class = class_id::WARRIOR;
                _subclass = 0;
            }

            _refresh();
        }
        else if(bn::keypad::a_pressed())
        {
            _step = step::CLASS;
            _dirty = true;
        }
        else if(bn::keypad::b_pressed())
        {
            return result::BACK;
        }
        break;

    case step::CLASS:
        if(up || down)
        {
            _choose_class(down ? 1 : -1);
        }
        else if(bn::keypad::a_pressed())
        {
            _step = step::SUBCLASS;
            _dirty = true;
        }
        else if(bn::keypad::b_pressed())
        {
            _step = step::RACE;
            _dirty = true;
        }
        break;

    case step::SUBCLASS:
        if(up || down)
        {
            _subclass = (_subclass + subclasses_per_class + (down ? 1 : -1)) % subclasses_per_class;
            _refresh();
        }
        else if(bn::keypad::a_pressed())
        {
            _step = step::CONFIRM;
            _dirty = true;
        }
        else if(bn::keypad::b_pressed())
        {
            if(_subclass_only)
            {
                return result::BACK;
            }

            _step = step::CLASS;
            _dirty = true;
        }
        break;

    case step::CONFIRM:
        if(bn::keypad::a_pressed())
        {
            if(_subclass_only)
            {
                choose_subclass(class_subclass(_class, _subclass));
            }
            else
            {
                // _refresh() already made the character; start it fresh in case stats were browsed.
                new_character(_race, _class, class_subclass(_class, _subclass));
            }

            ui::clear();
            ui::commit();
            return result::CREATED;
        }

        if(bn::keypad::b_pressed())
        {
            _step = step::SUBCLASS;
            _dirty = true;
        }
        break;

    default:
        break;
    }

    // Turns on the spot, walking.
    facing direction = turn_order[(_frame / turn_frames) % 4];
    _preview->update(bn::fixed_point(preview_x, preview_y), direction, true, _frame);
    _preview->sprite().set_bg_priority(0);

    if(_dirty)
    {
        _draw();
        _dirty = false;
    }

    ui::commit();
    return result::CHOOSING;
}

void character_creation::_choose_class(int direction)
{
    int count = int(class_id::COUNT);
    int index = int(_class);

    // The next class this race may be.
    for(int tries = 0; tries < count; ++tries)
    {
        index = (index + count + direction) % count;

        if(class_allowed(_race, class_id(index)))
        {
            break;
        }
    }

    _class = class_id(index);
    _subclass = 0;
    _refresh();
}

void character_creation::_refresh()
{
    // A fresh character of the choice, so the stats shown are the real ones. A loaded save stays as
    // it is until the choice is made.
    if(! _subclass_only)
    {
        new_character(_race, _class, class_subclass(_class, _subclass));
    }

    look_id look = player_look(_race, _class);

    if(_preview)
    {
        _preview->set_look(look);
    }
    else
    {
        _preview.emplace(look, _camera, preview_scale);
    }

    _dirty = true;
}

void character_creation::_draw()
{
    ui::clear();
    ui::panel(0, 0, ui::columns, ui::rows);
    ui::text_center(1, _subclass_only ? "Choose Your Spec" : "Create Your Hero", ui::color::YELLOW, true);
    ui::divider(1, 2, ui::columns - 2);

    bool choosing_race = _step == step::RACE;
    bool choosing_class = _step == step::CLASS;
    ui::text(2, race_top - 1, "Race", choosing_race ? ui::color::YELLOW : ui::color::GRAY, true);

    for(int index = 0; index < int(race_id::COUNT); ++index)
    {
        bool selected = index == int(_race);
        int y = race_top + index;

        if(selected && choosing_race)
        {
            ui::cursor(2, y);
        }

        ui::text(list_x, y, race_name(race_id(index)),
                 selected ? (choosing_race ? ui::color::WHITE : ui::color::GREEN) : ui::color::GRAY, true);
    }

    ui::text(2, class_top - 1, "Class", choosing_class ? ui::color::YELLOW : ui::color::GRAY, true);

    for(int index = 0; index < int(class_id::COUNT); ++index)
    {
        bool selected = index == int(_class);
        bool allowed = class_allowed(_race, class_id(index));
        int y = class_top + index;

        if(selected && choosing_class)
        {
            ui::cursor(2, y);
        }

        ui::color color = ! allowed ? ui::color::GRAY : selected ? (choosing_class ? ui::color::WHITE :
                          choosing_race ? ui::color::WHITE : ui::color::GREEN) : ui::color::WHITE;

        if(choosing_race && allowed && ! selected)
        {
            color = ui::color::GRAY;
        }

        ui::text(list_x, y, allowed ? class_name(class_id(index)) : "-", color, true);
    }

    if(_step == step::SUBCLASS || _step == step::CONFIRM)
    {
        // The subclass list takes the place of the numbers.
        bool choosing_subclass = _step == step::SUBCLASS;
        ui::text(subclass_x - 2, subclass_top - 1, "Spec", choosing_subclass ? ui::color::YELLOW : ui::color::GRAY,
                 true);

        for(int index = 0; index < subclasses_per_class; ++index)
        {
            bool selected = index == _subclass;
            int y = subclass_top + index;

            if(selected && choosing_subclass)
            {
                ui::cursor(subclass_x - 2, y);
            }

            ui::text(subclass_x, y, subclass_name(class_subclass(_class, index)),
                     selected ? (choosing_subclass ? ui::color::WHITE : ui::color::GREEN) :
                                (choosing_subclass ? ui::color::WHITE : ui::color::GRAY), true);
        }

        _draw_description();
        return;
    }

    // Starting numbers of the choice.
    const character_data& data = character();
    stats s = compute_stats();
    bn::string<16> line = "Str ";
    line += bn::to_string<4>(s.strength);
    ui::text(stats_x, stats_top, line, ui::color::WHITE, true);
    line = "Agi ";
    line += bn::to_string<4>(s.agility);
    ui::text(stats_x + 7, stats_top, line, ui::color::WHITE, true);
    line = "Sta ";
    line += bn::to_string<4>(s.stamina);
    ui::text(stats_x, stats_top + 1, line, ui::color::WHITE, true);
    line = "Int ";
    line += bn::to_string<4>(s.intellect);
    ui::text(stats_x + 7, stats_top + 1, line, ui::color::WHITE, true);
    line = data.player_class == class_id::WARRIOR ? "Rage" : "Mana ";

    if(data.player_class != class_id::WARRIOR)
    {
        line += bn::to_string<4>(s.max_power);
    }

    ui::text(stats_x, stats_top + 2, line, data.player_class == class_id::WARRIOR ? ui::color::RED :
             ui::color::BLUE, true);

    _draw_description();
}

void character_creation::_draw_description()
{
    subclass_id subclass = class_subclass(_class, _subclass);
    ui::divider(1, description_top - 1, ui::columns - 2);

    if(_step == step::CONFIRM && _subclass_only)
    {
        bn::string<96> text = "Train as a ";
        text += subclass_name(subclass);
        text += " ";
        text += class_name(_class);
        text += "? Your talent points come back to spend again.";
        ui::text_wrapped(2, description_top, 26, 3, text, ui::color::WHITE, true);
    }
    else if(_step == step::CONFIRM)
    {
        bn::string<96> text = "Begin as a ";
        text += race_name(_race);
        text += " ";
        text += subclass_name(subclass);
        text += " ";
        text += class_name(_class);
        text += " in Northshire Valley?";
        ui::text_wrapped(2, description_top, 26, 3, text, ui::color::WHITE, true);
    }
    else
    {
        const char* text = _step == step::RACE ? race_texts[int(_race)] :
                           _step == step::CLASS ? class_texts[int(_class)] : subclass_description(subclass);
        ui::text_wrapped(2, description_top, 26, 3, text, ui::color::WHITE, true);
    }

    ui::divider(1, hint_row - 1, ui::columns - 2);
    ui::text(2, hint_row, _step != step::CONFIRM ? "A Choose" : _subclass_only ? "A Train" : "A Begin",
             ui::color::WHITE, true);
    ui::text_right(27, hint_row, "B Back", ui::color::GRAY, true);
}

}
