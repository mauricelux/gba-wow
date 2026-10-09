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

    // The buttons that open each bar.
    constexpr const char* bar_keys[bar_count] = { "R", "L", "LR" };
    constexpr const char* bar_names[bar_count] = { "Combat", "Utility", "Buffs" };

    // Where the ability is on the bars, or false.
    bool find_on_bars(ability_id ability, int& bar, int& slot)
    {
        for(bar = 0; bar < bar_count; ++bar)
        {
            for(slot = 0; slot < action_slots; ++slot)
            {
                if(character().action_bars[bar][slot] == ability)
                {
                    return true;
                }
            }
        }

        return false;
    }

    // The next slot the bar has, going left or right.
    int step_slot(int bar, int slot, int direction)
    {
        do
        {
            slot = (slot + action_slots + direction) % action_slots;
        }
        while(! bar_has_slot(bar_id(bar), slot));

        return slot;
    }
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
        if(bn::keypad::l_pressed() || bn::keypad::r_pressed())
        {
            // L and R pick the bar.
            _assign_bar = (_assign_bar + bar_count + (bn::keypad::r_pressed() ? 1 : -1)) % bar_count;

            if(! bar_has_slot(bar_id(_assign_bar), _assign_slot))
            {
                _assign_slot = step_slot(_assign_bar, _assign_slot, 1);
            }

            _dirty = true;
        }
        else if(bn::keypad::left_pressed() || bn::keypad::right_pressed())
        {
            _assign_slot = step_slot(_assign_bar, _assign_slot, bn::keypad::right_pressed() ? 1 : -1);
            _dirty = true;
        }
        else if(bn::keypad::a_pressed())
        {
            // An ability sits in one slot only: the one it leaves gets what was in the new slot.
            ability_id& target = character().action_bars[_assign_bar][_assign_slot];
            ability_id replaced = target;
            int old_bar;
            int old_slot;

            if(find_on_bars(ability, old_bar, old_slot))
            {
                character().action_bars[old_bar][old_slot] = replaced;
            }

            target = ability;
            _assign_slot = -1;
            _status.show("Placed on the bar", ui::color::GREEN);
            _dirty = true;
        }
        else if(bn::keypad::select_pressed())
        {
            set_bar_slot(bar_id::COMBAT, -1, ability);
            _assign_slot = -1;
            _status.show("Taken off the bars", ui::color::GRAY);
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
        // Starts where it is now, or on the first slot of its usual bar.
        int bar;
        int slot;

        if(! find_on_bars(ability, bar, slot))
        {
            bar = int(default_bar(ability));
            slot = 0;
        }

        _assign_bar = bar;
        _assign_slot = slot;
        _dirty = true;
    }
}

void menu::_draw_spells()
{
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
        int bar;
        int slot;

        if(find_on_bars(ability, bar, slot))
        {
            // The keys that use it: "RA", "L^", "LR<".
            bn::string<4> keys = bar_keys[bar];
            keys += slot_labels[slot];
            ui::text_right(27, y, keys, ui::color::YELLOW, true);
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

    if(_assign_slot >= 0)
    {
        // What the slot holds now.
        ability_id current = character().action_bars[_assign_bar][_assign_slot];
        bn::string<48> line = bar_names[_assign_bar];
        line += " bar, hold ";
        line += bar_keys[_assign_bar];
        _page.add_copy(line, ui::color::YELLOW);
        line = "Now: ";
        line += current != ability_id::NONE ? get_ability(current).name : "empty";
        _page.add_copy(line, ui::color::GRAY);
        _page.add("L/R: bar  Left/Right: slot", ui::color::GRAY);
        _page.add("SEL: take off the bars", ui::color::GRAY);
    }
    else if(selected != ability_id::NONE)
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
        ui::text(page_x, hint_row, bar_names[_assign_bar], ui::color::YELLOW, true);

        for(int slot = 0; slot < action_slots; ++slot)
        {
            if(bar_has_slot(bar_id(_assign_bar), slot))
            {
                char label[2] = { slot_labels[slot], 0 };
                ui::text(12 + slot * 2, hint_row, label, slot == _assign_slot ? ui::color::YELLOW : ui::color::GRAY,
                         true);
            }
        }

        ui::cursor(11 + _assign_slot * 2, hint_row);
        return;
    }

    ui::text(page_x, hint_row, "A Place on a bar", ui::color::WHITE, true);
}

}
