#include "gw_menu.h"

#include "bn_keypad.h"
#include "bn_string.h"

#include "gw_character.h"
#include "gw_combat.h"
#include "gw_item_text.h"
#include "gw_menu_layout.h"
#include "gw_ui.h"

namespace gw
{

using namespace menu_layout;

namespace
{
    enum item_action : uint8_t
    {
        USE,
        EQUIP,
        ITEM_BAR,
        DROP
    };

    constexpr const char* action_names[] = { "Use", "Equip", "Item bar", "Drop" };

    constexpr int max_actions = 3;
    constexpr int actions_x = 16;

    // What A offers for the item, in order.
    int item_actions(item_id item, item_action* actions)
    {
        const item_def& def = get_item(item);
        int count = 0;

        if(def.slot != equip_slot::NONE)
        {
            actions[count++] = EQUIP;
        }

        if(usable_item(item))
        {
            actions[count++] = USE;
            actions[count++] = ITEM_BAR;
        }

        if(def.type != item_type::HEARTHSTONE)
        {
            actions[count++] = DROP;
        }

        return count;
    }

    // Items bar slots by direction: up, right, down, left.
    constexpr const char* slot_arrows[item_slots] = { "^", ">", "v", "<" };

    [[nodiscard]] const char* kind_name(item_type type)
    {
        return type == item_type::POTION ? "(healing potion)" : type == item_type::FOOD ? "(food)" :
               type == item_type::DRINK ? "(drink)" : "(hearthstone)";
    }
}

void menu::_update_bags()
{
    int row = _bags.selected_row();
    item_id item = row >= 0 ? character().bags[row].item : item_id::NONE;

    if(_confirm)
    {
        if(bn::keypad::a_pressed() && row >= 0)
        {
            bn::string<48> text = "Dropped ";
            text += get_item(item).name;
            remove_from_row(row, character().bags[row].count);
            _status.show(text, ui::color::GRAY);
            _confirm = false;
            _bags.rebuild();
            _dirty = true;
        }
        else if(bn::keypad::b_pressed())
        {
            _confirm = false;
            _dirty = true;
        }

        return;
    }

    if(_item_bar_pick)
    {
        int slot = bn::keypad::up_pressed() ? 0 : bn::keypad::right_pressed() ? 1 :
                   bn::keypad::down_pressed() ? 2 : bn::keypad::left_pressed() ? 3 : -1;

        if(slot >= 0 && item != item_id::NONE)
        {
            character().item_bar[slot] = item;
            _status.show("On the Items bar", ui::color::GREEN);
            _item_bar_pick = false;
            _dirty = true;
        }
        else if(bn::keypad::b_pressed())
        {
            _item_bar_pick = false;
            _dirty = true;
        }

        return;
    }

    if(_item_action >= 0)
    {
        item_action actions[max_actions];
        int count = item != item_id::NONE ? item_actions(item, actions) : 0;

        if(bn::keypad::b_pressed() || count == 0)
        {
            _item_action = -1;
            _dirty = true;
        }
        else if(bn::keypad::up_pressed() || bn::keypad::down_pressed())
        {
            _item_action = (_item_action + count + (bn::keypad::down_pressed() ? 1 : -1)) % count;
            _dirty = true;
        }
        else if(bn::keypad::a_pressed())
        {
            int action = actions[_item_action];
            _item_action = -1;
            _dirty = true;
            _do_item_action(action);
        }

        return;
    }

    if(_bags.update(list_rows))
    {
        _dirty = true;
    }

    if(bn::keypad::select_pressed())
    {
        // The next sort order, remembered in the save.
        character_data& data = character();
        data.sort = bag_sort((int(data.sort) + 1) % int(bag_sort::COUNT));
        _bags.rebuild();
        _bags.reset();

        bn::string<24> text = "Sorted by ";
        text += bag_view::sort_name(data.sort);
        _status.show(text, ui::color::WHITE);
        _dirty = true;
        return;
    }

    if(bn::keypad::a_pressed() && row >= 0)
    {
        _item_action = 0;
        _dirty = true;
    }
}

void menu::_do_item_action(int action)
{
    int row = _bags.selected_row();

    if(row < 0)
    {
        return;
    }

    item_id item = character().bags[row].item;

    switch(action)
    {

    case EQUIP:
        switch(equip_item(row))
        {

        case equip_result::OK:
            _combat.refresh_stats();
            _status.show("Equipped", ui::color::GREEN);
            _bags.rebuild();
            break;

        case equip_result::WRONG_CLASS:
            _status.show("Your class can't use that", ui::color::RED);
            break;

        case equip_result::LEVEL_TOO_LOW:
            _status.show("Your level is too low", ui::color::RED);
            break;

        default:
            break;
        }
        break;

    case USE:
    {
        // Using it from the menu closes the menu so you see it work.
        const char* error = nullptr;

        if(_combat.use_item(item, &error))
        {
            _open = false;
        }
        else if(error)
        {
            _status.show(error, ui::color::RED);
        }
        break;
    }

    case ITEM_BAR:
        _item_bar_pick = true;
        break;

    default:
        _confirm = true;
        break;
    }
}

void menu::_draw_bags()
{
    const character_data& data = character();
    int row = _bags.selected_row();
    item_id selected = row >= 0 ? data.bags[row].item : item_id::NONE;

    // The sort order beside the page name.
    ui::text_right(25, title_row, bag_view::sort_name(data.sort), ui::color::GRAY, true);
    _bags.draw(content_top, list_rows);

    if(_bags.empty())
    {
        ui::text_center(content_top + 3, "Your bags are empty.", ui::color::GRAY, true);
    }

    ui::divider(1, details_top - 1, ui::columns - 2);
    _page.clear();

    if(_item_bar_pick)
    {
        // What each slot uses now.
        for(int slot = 0; slot < item_slots; ++slot)
        {
            int y = details_top + slot;
            item_id own = item_bar_item(slot);
            ui::text(page_x, y, slot_arrows[slot], ui::color::YELLOW, true);

            if(own != item_id::NONE)
            {
                const item_def& def = get_item(own);
                ui::text(page_x + 2, y, def.name, ui::color(quality_color(def.quality)), true);
            }
            else
            {
                item_id set = data.item_bar[slot];
                ui::text(page_x + 2, y, kind_name(set != item_id::NONE ? get_item(set).type : item_bar_default(slot)),
                         ui::color::GRAY, true);
            }
        }

        ui::text(page_x, hint_row, "Press a direction", ui::color::WHITE, true);
        ui::text_right(27, hint_row, "B Back", ui::color::GRAY, true);
        return;
    }

    if(selected != item_id::NONE)
    {
        add_item_details(_page, selected);
    }

    _page.draw(page_x, details_top, details_rows, 0);

    if(_item_action >= 0 && selected != item_id::NONE)
    {
        item_action actions[max_actions];
        int count = item_actions(selected, actions);
        int top = content_top + 1;
        ui::panel(actions_x, top - 1, 12, count + 2);

        for(int index = 0; index < count; ++index)
        {
            if(index == _item_action)
            {
                ui::cursor(actions_x + 1, top + index);
            }

            ui::text(actions_x + 3, top + index, action_names[actions[index]],
                     actions[index] == DROP ? ui::color::RED : ui::color::WHITE, true);
        }

        ui::text(page_x, hint_row, "A Choose", ui::color::WHITE, true);
        ui::text_right(27, hint_row, "B Back", ui::color::GRAY, true);
        return;
    }

    if(_confirm)
    {
        ui::text(page_x, hint_row, "Drop it?", ui::color::RED, true);
        ui::text_right(27, hint_row, "A Yes  B No", ui::color::WHITE, true);
        return;
    }

    ui::text(page_x, hint_row, "A Item SEL Sort", ui::color::WHITE, true);
    ui::money_right(27, hint_row, data.money);
}

}
