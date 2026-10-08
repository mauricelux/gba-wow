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

int menu::_bag_slot(int row) const
{
    // The row-th occupied bag slot.
    for(int index = 0; index < bag_slots; ++index)
    {
        if(character().bags[index].item != item_id::NONE)
        {
            if(row == 0)
            {
                return index;
            }

            --row;
        }
    }

    return -1;
}

void menu::_update_bags()
{
    int count = bag_slots - free_bag_slots();
    int slot_index = _bag_slot(_cursor.index);

    if(_confirm)
    {
        if(bn::keypad::a_pressed() && slot_index >= 0)
        {
            bn::string<48> text = "Dropped ";
            text += get_item(character().bags[slot_index].item).name;
            character().bags[slot_index] = item_stack();
            _status.show(text, ui::color::GRAY);
            _confirm = false;
            _cursor.clamp(count - 1, list_rows);
            _dirty = true;
        }
        else if(bn::keypad::b_pressed())
        {
            _confirm = false;
            _dirty = true;
        }

        return;
    }

    if(_cursor.update(count, list_rows))
    {
        _dirty = true;
    }

    if(slot_index < 0)
    {
        return;
    }

    if(bn::keypad::select_pressed())
    {
        _confirm = true;
        _dirty = true;
        return;
    }

    if(! bn::keypad::a_pressed())
    {
        return;
    }

    item_id item = character().bags[slot_index].item;
    const item_def& def = get_item(item);
    _dirty = true;

    if(def.slot != equip_slot::NONE)
    {
        switch(equip_item(slot_index))
        {

        case equip_result::OK:
            _combat.refresh_stats();
            _status.show("Equipped", ui::color::GREEN);
            break;

        case equip_result::WRONG_CLASS:
            _status.show("Your class can't use that", ui::color::RED);
            break;

        case equip_result::LEVEL_TOO_LOW:
            _status.show("Your level is too low", ui::color::RED);
            break;

        default:
            _status.show("Inventory is full", ui::color::RED);
            break;
        }

        _cursor.clamp(bag_slots - free_bag_slots(), list_rows);
        return;
    }

    if(def.type == item_type::FOOD || def.type == item_type::DRINK || def.type == item_type::POTION)
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
            _dirty = true;
        }

        return;
    }

    _status.show("Sell it to a vendor", ui::color::GRAY);
}

void menu::_draw_bags()
{
    const character_data& data = character();
    int count = bag_slots - free_bag_slots();
    item_id selected = item_id::NONE;

    for(int row = 0; row < list_rows; ++row)
    {
        int index = _cursor.scroll + row;

        if(index >= count)
        {
            break;
        }

        const item_stack& slot = data.bags[_bag_slot(index)];
        const item_def& def = get_item(slot.item);
        bool usable = (def.slot == equip_slot::NONE || can_equip(data.player_class, def)) && def.level <= data.level;
        int y = content_top + row;

        if(index == _cursor.index)
        {
            ui::cursor(2, y);
            selected = slot.item;
        }

        int width = ui::text(4, y, def.name, usable ? ui::color(quality_color(def.quality)) : ui::color::RED, true);

        if(slot.count > 1)
        {
            bn::string<8> text = "x";
            text += bn::to_string<4>(slot.count);
            ui::text(5 + width, y, text, ui::color::GRAY, true);
        }
    }

    if(count == 0)
    {
        ui::text_center(content_top + 3, "Your bags are empty.", ui::color::GRAY, true);
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

    if(selected != item_id::NONE)
    {
        add_item_details(_page, selected);
    }

    _page.draw(page_x, details_top, details_rows, 0);

    if(_confirm)
    {
        ui::text(page_x, hint_row, "Drop it?", ui::color::RED, true);
        ui::text_right(27, hint_row, "A Yes  B No", ui::color::WHITE, true);
        return;
    }

    bn::string<16> slots = bn::to_string<4>(count);
    slots += "/";
    slots += bn::to_string<4>(bag_slots);
    ui::text(page_x, hint_row, "A Use SEL Drop", ui::color::WHITE, true);
    ui::text(17, hint_row, slots, ui::color::GRAY, true);
    ui::money_right(27, hint_row, data.money);
}

}
