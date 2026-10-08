#include "gw_vendor_screen.h"

#include "bn_keypad.h"
#include "bn_string.h"

#include "gw_character.h"
#include "gw_item_text.h"
#include "gw_npc_data.h"
#include "gw_ui.h"
#include "gw_vendors.h"

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

vendor_screen::vendor_screen() :
    _details(26)
{
}

void vendor_screen::open(npc_id npc)
{
    _npc = npc;
    _selling = false;
    _cursor = list_cursor();
    _status = status_line();
    _dirty = true;
    ui::clear();
}

int vendor_screen::_count() const
{
    if(! _selling)
    {
        return vendor_stock(get_npc_info(_npc).vendor).size();
    }

    return bag_slots - free_bag_slots();
}

int vendor_screen::_bag_slot(int row) const
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

bool vendor_screen::update()
{
    if(bn::keypad::b_pressed())
    {
        ui::clear();
        return false;
    }

    if(bn::keypad::l_pressed() || bn::keypad::r_pressed())
    {
        _selling = ! _selling;
        _cursor = list_cursor();
        _dirty = true;
    }

    if(_cursor.update(_count(), list_rows))
    {
        _dirty = true;
    }

    if(bn::keypad::a_pressed() && _count() > 0)
    {
        if(_selling)
        {
            _sell();
        }
        else
        {
            _buy();
        }

        _dirty = true;
    }
    else if(bn::keypad::select_pressed() && _selling)
    {
        _sell_junk();
        _dirty = true;
    }

    if(_status.update())
    {
        _dirty = true;
    }

    if(_dirty)
    {
        _cursor.clamp(_count(), list_rows);
        _draw();
        _dirty = false;
    }

    return true;
}

void vendor_screen::_buy()
{
    item_id item = vendor_stock(get_npc_info(_npc).vendor)[_cursor.index];
    int price = buy_price(item);
    character_data& data = character();

    if(data.money < price)
    {
        _status.show("Not enough money", ui::color::RED);
        return;
    }

    if(add_item(item) > 0)
    {
        _status.show("Inventory is full", ui::color::RED);
        return;
    }

    data.money -= price;
    bn::string<48> text = "Bought ";
    text += get_item(item).name;
    _status.show(text, ui::color::YELLOW);
}

void vendor_screen::_sell()
{
    int slot_index = _bag_slot(_cursor.index);

    if(slot_index < 0)
    {
        return;
    }

    item_stack& slot = character().bags[slot_index];

    if(! sell_price(slot.item))
    {
        _status.show("They won't buy that", ui::color::RED);
        return;
    }

    character().money += sell_price(slot.item) * slot.count;
    bn::string<48> text = "Sold ";
    text += get_item(slot.item).name;
    _status.show(text, ui::color::YELLOW);
    slot = item_stack();
}

void vendor_screen::_sell_junk()
{
    int total = 0;

    for(item_stack& slot : character().bags)
    {
        if(slot.item != item_id::NONE && get_item(slot.item).quality == item_quality::POOR)
        {
            total += sell_price(slot.item) * slot.count;
            slot = item_stack();
        }
    }

    if(total == 0)
    {
        _status.show("No junk to sell", ui::color::GRAY);
        return;
    }

    character().money += total;
    _status.show("Sold all junk", ui::color::YELLOW);
}

void vendor_screen::_draw()
{
    const npc_info& info = get_npc_info(_npc);
    const character_data& data = character();
    ui::panel(0, 0, ui::columns, ui::rows);
    ui::text(2, 1, info.name, ui::color::YELLOW, true);
    ui::text_right(22, 1, "Buy", _selling ? ui::color::GRAY : ui::color::WHITE, true);
    ui::text_right(27, 1, "Sell", _selling ? ui::color::WHITE : ui::color::GRAY, true);
    ui::divider(1, 2, ui::columns - 2);
    ui::divider(1, details_top - 1, ui::columns - 2);
    ui::divider(1, hint_row - 1, ui::columns - 2);
    _status.draw(details_top - 1);

    int count = _count();
    item_id selected = item_id::NONE;
    int selected_count = 1;

    for(int row = 0; row < list_rows; ++row)
    {
        int index = _cursor.scroll + row;

        if(index >= count)
        {
            break;
        }

        item_id item;
        int stack = 1;

        if(_selling)
        {
            const item_stack& slot = data.bags[_bag_slot(index)];
            item = slot.item;
            stack = slot.count;
        }
        else
        {
            item = vendor_stock(info.vendor)[index];
        }

        const item_def& def = get_item(item);
        bool usable = (def.slot == equip_slot::NONE || can_equip(data.player_class, def)) && def.level <= data.level;
        int y = list_top + row;

        if(index == _cursor.index)
        {
            ui::cursor(2, y);
            selected = item;
            selected_count = stack;
        }

        int width = ui::text(4, y, def.name, usable ? ui::color(quality_color(def.quality)) : ui::color::RED, true);

        if(stack > 1)
        {
            bn::string<8> text = "x";
            text += bn::to_string<4>(stack);
            ui::text(5 + width, y, text, ui::color::GRAY, true);
        }
    }

    if(_cursor.scroll > 0)
    {
        ui::scroll_arrow(28, list_top, true);
    }

    if(_cursor.scroll + list_rows < count)
    {
        ui::scroll_arrow(28, list_top + list_rows - 1, false);
    }

    _details.clear();

    if(selected != item_id::NONE)
    {
        if(_selling)
        {
            _details.add_money("Sells for", sell_price(selected) * selected_count);
        }
        else
        {
            _details.add_money("Price", buy_price(selected));
        }

        add_item_details(_details, selected);
    }
    else if(_selling)
    {
        _details.add("Nothing to sell.", ui::color::GRAY);
    }

    _details.draw(2, details_top, details_rows, 0);

    ui::text(2, hint_row, _selling ? "A Sell SEL Junk" : "A Buy  L/R Sell", ui::color::WHITE, true);
    ui::money_right(27, hint_row, data.money);
}

}
