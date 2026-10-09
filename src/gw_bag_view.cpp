#include "gw_bag_view.h"

#include "bn_algorithm.h"
#include "bn_keypad.h"
#include "bn_math.h"
#include "bn_string.h"

#include "gw_audio.h"
#include "gw_input.h"
#include "gw_items.h"
#include "gw_ui.h"

namespace gw
{

namespace
{
    // By type: equipment by slot, then consumables, other things, quest items and junk.
    enum type_group : uint8_t
    {
        HEAD,
        CHEST,
        HANDS,
        LEGS,
        FEET,
        WEAPONS,
        OFF_HAND,
        RANGED,
        CONSUMABLES,
        OTHER,
        QUEST_ITEMS,
        JUNK,
        TYPE_GROUPS
    };

    constexpr const char* type_headings[TYPE_GROUPS] = {
        "Head", "Chest", "Hands", "Legs", "Feet", "Weapons", "Off-hand", "Ranged", "Consumables", "Other",
        "Quest items", "Junk"
    };

    constexpr const char* quality_headings[] = { "Epic", "Rare", "Uncommon", "Common", "Junk" };

    constexpr int level_brackets = max_level / 5 + 1;

    struct sort_entry
    {
        uint32_t key;   // the group in the top byte
        int16_t row;
    };

    BN_DATA_EWRAM_BSS sort_entry scratch[bag_rows];

    [[nodiscard]] int type_group_of(const item_def& def)
    {
        if(def.quality == item_quality::POOR || def.type == item_type::JUNK)
        {
            return JUNK;
        }

        if(def.slot != equip_slot::NONE)
        {
            return def.slot == equip_slot::MAIN_HAND ? WEAPONS : def.slot == equip_slot::OFF_HAND ? OFF_HAND :
                   def.slot == equip_slot::RANGED ? RANGED : int(def.slot);
        }

        switch(def.type)
        {

        case item_type::POTION:
        case item_type::FOOD:
        case item_type::DRINK:
            return CONSUMABLES;

        case item_type::QUEST:
            return QUEST_ITEMS;

        default:
            return OTHER;
        }
    }

    // Potions, then food, then drink.
    [[nodiscard]] int consumable_order(const item_def& def)
    {
        return def.type == item_type::POTION ? 0 : def.type == item_type::FOOD ? 1 :
               def.type == item_type::DRINK ? 2 : 3;
    }

    [[nodiscard]] uint32_t sort_key(bag_sort sort, const item_def& def)
    {
        uint32_t quality = 4 - uint32_t(def.quality);
        uint32_t level = 255 - uint32_t(def.level);
        uint32_t type = uint32_t(type_group_of(def));

        switch(sort)
        {

        case bag_sort::TYPE:
            return (type << 24) | (uint32_t(consumable_order(def)) << 16) | (quality << 8) | level;

        case bag_sort::QUALITY:
            return (quality << 24) | (type << 16) | level;

        case bag_sort::LEVEL:
            return (uint32_t(level_brackets - 1 - def.level / 5) << 24) | (level << 16) | (quality << 8) | type;

        default:
            return 0;
        }
    }

    // The heading over a group.
    [[nodiscard]] bn::string<16> heading(bag_sort sort, int group)
    {
        if(sort == bag_sort::TYPE)
        {
            return type_headings[group];
        }

        if(sort == bag_sort::QUALITY)
        {
            return quality_headings[bn::min(group, 4)];
        }

        int low = (level_brackets - 1 - group) * 5;

        if(low >= max_level)
        {
            bn::string<16> text = "Level ";
            text += bn::to_string<4>(max_level);
            return text;
        }

        bn::string<16> text = "Levels ";
        text += bn::to_string<4>(bn::max(1, low));
        text += "-";
        text += bn::to_string<4>(low + 4);
        return text;
    }
}

const char* bag_view::sort_name(bag_sort sort)
{
    constexpr const char* names[] = { "Type", "Quality", "Level", "Newest" };
    return names[int(sort)];
}

void bag_view::rebuild()
{
    const character_data& data = character();
    bag_sort sort = data.sort;
    int rows = bag_row_count();

    for(int row = 0; row < rows; ++row)
    {
        scratch[row] = sort_entry{ sort_key(sort, get_item(data.bags[row].item)), int16_t(row) };
    }

    // Newest first among equals.
    bn::sort(scratch, scratch + rows, [](const sort_entry& a, const sort_entry& b)
    {
        return a.key != b.key ? a.key < b.key : a.row > b.row;
    });

    _entries.clear();
    int last_group = -1;

    for(int index = 0; index < rows; ++index)
    {
        int group = int(scratch[index].key >> 24);

        if(sort != bag_sort::NEWEST && group != last_group && ! _entries.full())
        {
            _entries.push_back(int16_t(-1 - group));
            last_group = group;
        }

        if(! _entries.full())
        {
            _entries.push_back(scratch[index].row);
        }
    }

    _settle(1);
}

void bag_view::reset()
{
    _index = 0;
    _scroll = 0;
    _settle(1);
}

void bag_view::_settle(int direction)
{
    // Onto an item: the next one, or the one before at the end of the list.
    _index = bn::max(0, bn::min(_index, _entries.size() - 1));

    if(! _is_item(_index))
    {
        int step = direction;

        for(int tries = 0; tries < 2 && ! _is_item(_index); ++tries, step = -step)
        {
            int index = _index;

            while(index >= 0 && index < _entries.size() && ! _is_item(index))
            {
                index += step;
            }

            if(_is_item(index))
            {
                _index = index;
            }
        }
    }

    _scroll_to_cursor();
}

void bag_view::_scroll_to_cursor()
{
    // The heading of the cursor's group shows with its first item.
    int top = _index > 0 && ! _is_item(_index - 1) ? _index - 1 : _index;

    if(top < _scroll)
    {
        _scroll = top;
    }
    else if(_index >= _scroll + _visible)
    {
        _scroll = _index - _visible + 1;
    }

    _scroll = bn::max(0, bn::min(_scroll, _entries.size() - _visible));
}

bool bag_view::update(int visible)
{
    _visible = visible;
    int old_index = _index;

    if(input::repeated(bn::keypad::key_type::UP))
    {
        for(int index = _index - 1; index >= 0; --index)
        {
            if(_is_item(index))
            {
                _index = index;
                break;
            }
        }
    }
    else if(input::repeated(bn::keypad::key_type::DOWN))
    {
        for(int index = _index + 1; index < _entries.size(); ++index)
        {
            if(_is_item(index))
            {
                _index = index;
                break;
            }
        }
    }
    else if(input::repeated(bn::keypad::key_type::LEFT))
    {
        // The first item of this group, or of the group before when already there.
        int index = _index;

        while(_is_item(index - 1))
        {
            --index;
        }

        if(index == _index)
        {
            index -= 2;

            while(_is_item(index - 1))
            {
                --index;
            }
        }

        if(_is_item(index))
        {
            _index = index;
        }
    }
    else if(input::repeated(bn::keypad::key_type::RIGHT))
    {
        int index = _index;

        while(_is_item(index + 1))
        {
            ++index;
        }

        if(_is_item(index + 2))
        {
            _index = index + 2;
        }
        else if(! _is_item(index + 1))
        {
            _index = index;     // the last group: its last item
        }
    }

    _scroll_to_cursor();

    if(_index != old_index)
    {
        play_sound(sound_id::SELECT);
        return true;
    }

    return false;
}

int bag_view::selected_row() const
{
    return _is_item(_index) ? _entries[_index] : -1;
}

void bag_view::draw(int top, int visible) const
{
    const character_data& data = character();

    for(int line = 0; line < visible; ++line)
    {
        int index = _scroll + line;

        if(index >= _entries.size())
        {
            break;
        }

        int y = top + line;
        int entry = _entries[index];

        if(entry < 0)
        {
            ui::text(3, y, heading(data.sort, -1 - entry), ui::color::YELLOW, true);
            continue;
        }

        const item_stack& stack = data.bags[entry];
        const item_def& def = get_item(stack.item);
        bool usable = (def.slot == equip_slot::NONE || can_equip(data.player_class, def)) && def.level <= data.level;

        if(index == _index)
        {
            ui::cursor(2, y);
        }

        int width = ui::text(4, y, def.name, usable ? ui::color(quality_color(def.quality)) : ui::color::RED, true);

        if(stack.count > 1)
        {
            bn::string<8> text = "x";
            text += bn::to_string<4>(stack.count);
            ui::text(5 + width, y, text, ui::color::GRAY, true);
        }
    }

    if(_scroll > 0)
    {
        ui::scroll_arrow(28, top, true);
    }

    if(_scroll + visible < _entries.size())
    {
        ui::scroll_arrow(28, top + visible - 1, false);
    }
}

}
