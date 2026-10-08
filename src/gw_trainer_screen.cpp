#include "gw_trainer_screen.h"

#include "bn_keypad.h"
#include "bn_string.h"

#include "gw_character.h"
#include "gw_npc_data.h"
#include "gw_ui.h"

namespace gw
{

namespace
{
    constexpr int list_top = 3;
    constexpr int list_rows = 9;
    constexpr int details_top = 13;
    constexpr int details_rows = 4;
    constexpr int hint_row = 18;
}

trainer_screen::trainer_screen() :
    _details(26)
{
}

void trainer_screen::open(npc_id npc)
{
    _npc = npc;
    _cursor = list_cursor();
    _status = status_line();
    _abilities.clear();

    class_id trainer_class = get_npc_info(npc).trainer_class;

    // Trainers teach every ability of their class except talents, cheapest first.
    for(int level = 1; level <= max_level; ++level)
    {
        for(int index = 1; index < ability_count && ! _abilities.full(); ++index)
        {
            const ability_def& def = get_ability(ability_id(index));

            if(def.player_class == trainer_class && def.level == level)
            {
                _abilities.push_back(ability_id(index));
            }
        }
    }

    _dirty = true;
    ui::clear();
}

bool trainer_screen::update()
{
    if(bn::keypad::b_pressed())
    {
        ui::clear();
        return false;
    }

    if(_cursor.update(_abilities.size(), list_rows))
    {
        _dirty = true;
    }

    if(bn::keypad::a_pressed() && ! _abilities.empty())
    {
        _learn();
        _dirty = true;
    }

    if(_status.update())
    {
        _dirty = true;
    }

    if(_dirty)
    {
        _draw();
        _dirty = false;
    }

    return true;
}

void trainer_screen::_learn()
{
    ability_id ability = _abilities[_cursor.index];
    const ability_def& def = get_ability(ability);
    character_data& data = character();

    if(knows_ability(ability))
    {
        _status.show("You already know that", ui::color::GRAY);
        return;
    }

    if(data.level < def.level)
    {
        _status.show("Your level is too low", ui::color::RED);
        return;
    }

    if(data.money < def.train_cost)
    {
        _status.show("Not enough money", ui::color::RED);
        return;
    }

    data.money -= def.train_cost;
    learn_ability(ability);

    bn::string<48> text = "Learned ";
    text += def.name;
    _status.show(text, ui::color::GREEN);
}

void trainer_screen::_draw()
{
    const npc_info& info = get_npc_info(_npc);
    const character_data& data = character();
    ui::panel(0, 0, ui::columns, ui::rows);
    ui::text(2, 1, info.name, ui::color::YELLOW, true);
    ui::divider(1, 2, ui::columns - 2);
    ui::divider(1, details_top - 1, ui::columns - 2);
    ui::divider(1, hint_row - 1, ui::columns - 2);
    _status.draw(details_top - 1);

    for(int row = 0; row < list_rows; ++row)
    {
        int index = _cursor.scroll + row;

        if(index >= _abilities.size())
        {
            break;
        }

        ability_id ability = _abilities[index];
        const ability_def& def = get_ability(ability);
        bool known = knows_ability(ability);
        bool can_learn = ! known && data.level >= def.level;
        int y = list_top + row;

        if(index == _cursor.index)
        {
            ui::cursor(2, y);
        }

        ui::text(4, y, def.name, known ? ui::color::GRAY : can_learn ? ui::color::GREEN : ui::color::RED, true);

        bn::string<8> level = known ? "" : "Lv ";

        if(! known)
        {
            level += bn::to_string<4>(def.level);
        }

        ui::text_right(27, y, known ? "Known" : level, ui::color::GRAY, true);
    }

    if(_cursor.scroll > 0)
    {
        ui::scroll_arrow(28, list_top, true);
    }

    if(_cursor.scroll + list_rows < _abilities.size())
    {
        ui::scroll_arrow(28, list_top + list_rows - 1, false);
    }

    _details.clear();

    if(! _abilities.empty())
    {
        const ability_def& def = get_ability(_abilities[_cursor.index]);

        if(! knows_ability(_abilities[_cursor.index]))
        {
            if(def.train_cost > 0)
            {
                _details.add_money("Cost", def.train_cost);
            }
            else
            {
                _details.add("Cost: free", ui::color::WHITE);
            }
        }

        _details.add(def.description, ui::color::GRAY);
    }

    _details.draw(2, details_top, details_rows, 0);
    ui::text(2, hint_row, "A Learn", ui::color::WHITE, true);
    ui::money_right(27, hint_row, data.money);
}

}
