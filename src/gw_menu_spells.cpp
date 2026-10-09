#include "gw_menu.h"

#include "bn_keypad.h"
#include "bn_string.h"

#include "gw_character.h"
#include "gw_menu_layout.h"
#include "gw_ui.h"

namespace gw
{

using namespace menu_layout;

namespace
{
    constexpr char slot_labels[action_slots] = { 'A', 'B', 'L', '^', '>', 'v', '<' };
}

int menu::_known_count() const
{
    int count = 0;

    for(int index = 1; index < ability_count; ++index)
    {
        if(knows_ability(ability_id(index)))
        {
            ++count;
        }
    }

    return count;
}

int menu::_known_at(int row) const
{
    for(int index = 1; index < ability_count; ++index)
    {
        if(knows_ability(ability_id(index)))
        {
            if(row == 0)
            {
                return index;
            }

            --row;
        }
    }

    return 0;
}

void menu::_update_spells()
{
    ability_id ability = ability_id(_known_at(_cursor.index));

    if(_assign_slot >= 0)
    {
        if(bn::keypad::left_pressed())
        {
            _assign_slot = (_assign_slot + action_slots - 1) % action_slots;
            _dirty = true;
        }
        else if(bn::keypad::right_pressed())
        {
            _assign_slot = (_assign_slot + 1) % action_slots;
            _dirty = true;
        }
        else if(bn::keypad::a_pressed())
        {
            // An ability sits in one slot only: the one it leaves gets what was in the new slot.
            ability_id* bar = character().action_bar;

            for(int slot = 0; slot < action_slots; ++slot)
            {
                if(bar[slot] == ability)
                {
                    bar[slot] = bar[_assign_slot];
                }
            }

            bar[_assign_slot] = ability;
            _assign_slot = -1;
            _status.show("Placed on the action bar", ui::color::GREEN);
            _dirty = true;
        }
        else if(bn::keypad::b_pressed())
        {
            _assign_slot = -1;
            _dirty = true;
        }

        return;
    }

    if(_cursor.update(_known_count(), list_rows))
    {
        _dirty = true;
    }

    if(bn::keypad::a_pressed() && ability != ability_id::NONE)
    {
        _assign_slot = 0;

        for(int slot = 0; slot < action_slots; ++slot)
        {
            if(character().action_bar[slot] == ability)
            {
                _assign_slot = slot;
            }
        }

        _dirty = true;
    }
}

void menu::_draw_spells()
{
    const character_data& data = character();
    int count = _known_count();
    ability_id selected = ability_id::NONE;

    for(int row = 0; row < list_rows; ++row)
    {
        int index = _cursor.scroll + row;

        if(index >= count)
        {
            break;
        }

        ability_id ability = ability_id(_known_at(index));
        int y = content_top + row;

        if(index == _cursor.index)
        {
            ui::cursor(2, y);
            selected = ability;
        }

        // "Fireball (Rank 5)" when it fits before the action bar label.
        bn::string<32> name = get_ability(ability).name;

        if(rank_count(ability) > 1)
        {
            bn::string<12> rank = " (Rank ";
            rank += bn::to_string<4>(ability_rank(ability));
            rank += ")";

            if(name.size() + rank.size() <= 21)
            {
                name += rank;
            }
        }

        ui::text(4, y, name, ui::color::WHITE, true);

        for(int slot = 0; slot < action_slots; ++slot)
        {
            if(data.action_bar[slot] == ability)
            {
                char label[2] = { slot_labels[slot], 0 };
                ui::text(26, y, label, ui::color::YELLOW, true);
            }
        }
    }

    if(_cursor.scroll > 0)
    {
        ui::scroll_arrow(28, content_top, true);
    }

    if(_cursor.scroll + list_rows < count)
    {
        ui::scroll_arrow(28, content_top + list_rows - 1, false);
    }

    ui::divider(1, details_top - 1, ui::columns - 2);
    _page.clear();

    if(selected != ability_id::NONE)
    {
        const ability_def& def = get_ability(selected);
        int ability_cost_now = ability_cost(selected);
        bn::string<48> cost;

        if(rank_count(selected) > 1)
        {
            cost += "Rank ";
            cost += bn::to_string<4>(ability_rank(selected));
            cost += ": ";
        }

        if(ability_cost_now)
        {
            cost += bn::to_string<4>(ability_cost_now);
            cost += uses_mana() ? " mana" : " rage";
        }
        else
        {
            cost += "No cost";
        }

        if(def.cooldown)
        {
            cost += ", ";
            cost += bn::to_string<4>(def.cooldown / 60);
            cost += "s cooldown";
        }

        _page.add_copy(cost, ui::color::GRAY);
        _page.add(def.description);
    }

    _page.draw(page_x, details_top, details_rows, 0);

    if(_assign_slot >= 0)
    {
        ui::text(page_x, hint_row, "Slot:", ui::color::WHITE, true);

        for(int slot = 0; slot < action_slots; ++slot)
        {
            char label[2] = { slot_labels[slot], 0 };
            ui::text(9 + slot * 2, hint_row, label, slot == _assign_slot ? ui::color::YELLOW : ui::color::GRAY, true);
        }

        if(_assign_slot >= 0)
        {
            ui::cursor(8 + _assign_slot * 2, hint_row);
        }

        return;
    }

    ui::text(page_x, hint_row, "A Place on action bar", ui::color::WHITE, true);
}

}
