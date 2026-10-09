#include "gw_trainer_screen.h"

#include "bn_keypad.h"
#include "bn_math.h"
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

    const character_data& data = character();

    // Trainers teach the subclass's abilities in the order they come, and further ranks of talent
    // and quest abilities once the talent or the quest taught the first. Riding trainers teach riding.
    class_id trainer_class = get_npc_info(npc).trainer_class;

    if(teaches(trainer_class))
    {
        for(int level = 1; level <= max_level; ++level)
        {
            for(int index = 1; index < ability_count && ! _abilities.full(); ++index)
            {
                ability_id ability = ability_id(index);
                const ability_def& def = get_ability(ability);

                if(def.player_class == trainer_class && in_kit(ability, data.subclass) &&
                   ability_level(ability) == level &&
                   (! (def.flags & (ability_flag::TALENT | ability_flag::QUEST)) || knows_ability(ability)))
                {
                    _abilities.push_back(ability);
                }
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
    int rank = trainable_rank(ability);

    if(! rank)
    {
        int known = ability_rank(ability);

        if(known >= rank_count(ability))
        {
            _status.show(known > 1 ? "You know every rank" : "You already know that", ui::color::GRAY);
        }
        else
        {
            _status.show("Your level is too low", ui::color::RED);
        }

        return;
    }

    int cost = rank_train_cost(ability, rank);

    if(data.money < cost)
    {
        _status.show("Not enough money", ui::color::RED);
        return;
    }

    data.money -= cost;
    learn_ability(ability, rank);

    bn::string<48> text = "Learned ";
    text += def.name;

    if(rank_count(ability) > 1)
    {
        text += " ";
        text += bn::to_string<4>(rank);
    }

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
        int known = ability_rank(ability);
        int next = trainable_rank(ability);
        bool maxed = known >= rank_count(ability);
        int y = list_top + row;

        if(index == _cursor.index)
        {
            ui::cursor(2, y);
        }

        // Green: something to learn now. White: known, more ranks later. Red: not yet. Gray: done.
        ui::color color = next ? ui::color::GREEN : maxed ? ui::color::GRAY : known ? ui::color::WHITE :
                                                                                     ui::color::RED;
        ui::text(4, y, def.name, color, true);

        // The rank to learn now or known, or the level the next one comes at.
        bn::string<4> right;

        if(next || maxed)
        {
            right = "R";
            right += bn::to_string<4>(next ? next : known);
        }
        else
        {
            right = bn::to_string<4>(rank_level(ability, known + 1));
        }

        ui::text_right(27, y, right, next ? ui::color::GREEN : maxed ? ui::color::GRAY : ui::color::RED, true);
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
        ability_id ability = _abilities[_cursor.index];
        const ability_def& def = get_ability(ability);
        int known = ability_rank(ability);
        int ranks = rank_count(ability);
        int next = trainable_rank(ability);
        int shown = next ? next : bn::min(known + 1, ranks);

        _rank_text = "Rank ";
        _rank_text += bn::to_string<4>(shown);
        _rank_text += " of ";
        _rank_text += bn::to_string<4>(ranks);

        if(next)
        {
            int cost = rank_train_cost(ability, next);

            if(cost > 0)
            {
                _details.add_money(_rank_text, cost);
            }
            else
            {
                _rank_text += ", free";
                _details.add(_rank_text, ui::color::WHITE);
            }
        }
        else if(known >= ranks)
        {
            _rank_text += ", known";
            _details.add(_rank_text, ui::color::GRAY);
        }
        else
        {
            _rank_text += " at level ";
            _rank_text += bn::to_string<4>(rank_level(ability, shown));
            _details.add(_rank_text, ui::color::RED);
        }

        _details.add(def.description, ui::color::GRAY);
    }

    _details.draw(2, details_top, details_rows, 0);
    ui::text(2, hint_row, "A Learn", ui::color::WHITE, true);
    ui::money_right(27, hint_row, data.money);
}

}
