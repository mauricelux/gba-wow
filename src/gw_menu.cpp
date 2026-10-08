#include "gw_menu.h"

#include "bn_keypad.h"
#include "bn_math.h"
#include "bn_string.h"

#include "gw_character.h"
#include "gw_combat.h"
#include "gw_hud.h"
#include "gw_input.h"
#include "gw_map_deadmines.h"
#include "gw_map_elwynn.h"
#include "gw_map_westfall.h"
#include "gw_menu_layout.h"
#include "gw_save.h"
#include "gw_ui.h"

namespace gw
{

using namespace menu_layout;

namespace
{
    constexpr const char* tab_names[] = { "Character", "Bags", "Spellbook", "Quest Log", "System" };

    constexpr const char* slot_names[] = { "Head", "Chest", "Hands", "Legs", "Feet", "Main", "Off", "Range" };

    constexpr const char* race_names[] = { "Human", "Dwarf", "Night Elf" };

    constexpr const char* class_names[] = { "Warrior", "Mage", "Hunter" };

    struct destination
    {
        const char* name;
        map_id map;
        point_def point;
    };

    constexpr destination destinations[] = {
        { "Northshire Abbey", map_id::ELWYNN, map_data::elwynn::start },
        { "Goldshire", map_id::ELWYNN, map_data::elwynn::goldshire_respawn },
        { "Stonefield Farm", map_id::ELWYNN, { 1700, 1270 } },
        { "Forest's Edge", map_id::ELWYNN, { 330, 1450 } },
        { "Sentinel Hill", map_id::WESTFALL, map_data::westfall::sentinel_respawn },
        { "Moonbrook", map_id::WESTFALL, map_data::westfall::deadmines_exit },
        { "The Deadmines", map_id::DEADMINES, map_data::deadmines::entry },
        { "Ironclad Cove", map_id::DEADMINES, { 600, 164 } },
    };

    constexpr int destination_count = sizeof(destinations) / sizeof(destinations[0]);

    // System page entries.
    enum class system_entry
    {
        SAVE,
        TELEPORT,
        LEVEL_UP,
        GOLD,
        COUNT
    };

    constexpr const char* system_names[] = { "Save game", "Debug: teleport", "Debug: level up", "Debug: +10 gold" };

    void stat(bn::string<32>& text, const char* name, int value)
    {
        text += name;
        text += " ";
        text += bn::to_string<6>(value);
    }

    void pad(bn::string<32>& text, int size)
    {
        while(text.size() < size)
        {
            text += ' ';
        }
    }
}

menu::menu(combat& combat_ref, hud& hud_ref, npcs& npcs_ref) :
    _combat(combat_ref),
    _hud(hud_ref),
    _npcs(npcs_ref),
    _page(page_width)
{
}

void menu::open()
{
    _open = true;
    _dirty = true;
    _quest = quest_id::NONE;
    _confirm = false;
    _assign_slot = -1;
    _teleport_list = false;
    _cursor = list_cursor();
    teleport = teleport_request();
    ui::clear();
}

bool menu::_in_submode() const
{
    return _quest != quest_id::NONE || _confirm || _assign_slot >= 0 || _teleport_list;
}

bool menu::update()
{
    if(! _in_submode())
    {
        if(bn::keypad::start_pressed() || bn::keypad::b_pressed())
        {
            _open = false;
            ui::clear();
            return false;
        }

        if(bn::keypad::l_pressed())
        {
            _switch_tab(-1);
        }
        else if(bn::keypad::r_pressed())
        {
            _switch_tab(1);
        }
    }

    switch(_tab)
    {

    case tab::CHARACTER:
        _update_character();
        break;

    case tab::BAGS:
        _update_bags();
        break;

    case tab::SPELLS:
        _update_spells();
        break;

    case tab::QUESTS:
        _update_quests();
        break;

    case tab::SYSTEM:
        _update_system();
        break;

    default:
        break;
    }

    if(! _open)
    {
        ui::clear();
        return false;
    }

    if(_status.update())
    {
        _dirty = true;
    }

    if(_dirty)
    {
        _draw_frame();

        switch(_tab)
        {

        case tab::CHARACTER:
            _draw_character();
            break;

        case tab::BAGS:
            _draw_bags();
            break;

        case tab::SPELLS:
            _draw_spells();
            break;

        case tab::QUESTS:
            _draw_quests();
            break;

        case tab::SYSTEM:
            _draw_system();
            break;

        default:
            break;
        }

        _status.draw(hint_row - 1);
        _dirty = false;
    }

    return true;
}

void menu::_switch_tab(int direction)
{
    int count = int(tab::COUNT);
    _tab = tab((int(_tab) + count + direction) % count);
    _cursor = list_cursor();
    _dirty = true;
}

void menu::_draw_frame()
{
    ui::panel(0, 0, ui::columns, ui::rows);
    ui::divider(1, 2, ui::columns - 2);
    ui::divider(1, hint_row - 1, ui::columns - 2);
    ui::text(2, title_row, "L", ui::color::GRAY, true);
    ui::text_right(27, title_row, "R", ui::color::GRAY, true);
    ui::text_center(title_row, tab_names[int(_tab)], ui::color::YELLOW, true);
}

// --- character ----------------------------------------------------------------------------------

void menu::_update_character()
{
    if(_cursor.update(int(equip_slot::COUNT), int(equip_slot::COUNT)))
    {
        _dirty = true;
    }

    if(bn::keypad::a_pressed())
    {
        equip_slot slot = equip_slot(_cursor.index);

        if(character().equipment[_cursor.index] == item_id::NONE)
        {
            return;
        }

        if(unequip_item(slot))
        {
            _combat.refresh_stats();
            _status.show("Moved to your bags", ui::color::WHITE);
        }
        else
        {
            _status.show("Inventory is full", ui::color::RED);
        }

        _dirty = true;
    }
}

void menu::_draw_character()
{
    const character_data& data = character();
    const stats& s = _combat.player_stats();

    for(int index = 0; index < int(equip_slot::COUNT); ++index)
    {
        int y = content_top + index;
        bool selected = index == _cursor.index;
        ui::text(2, y, slot_names[index], selected ? ui::color::YELLOW : ui::color::GRAY, true);
        item_id item = data.equipment[index];

        if(item != item_id::NONE)
        {
            const item_def& def = get_item(item);
            ui::text(8, y, def.name, ui::color(quality_color(def.quality)), true);
        }
        else
        {
            ui::text(8, y, "-", ui::color::GRAY, true);
        }
    }

    int y = content_top + int(equip_slot::COUNT);
    ui::divider(1, y, ui::columns - 2);

    bn::string<32> line = "Level ";
    line += bn::to_string<4>(data.level);
    line += " ";
    line += race_names[int(data.race)];
    line += " ";
    line += class_names[int(data.player_class)];
    ui::text(2, y + 1, line, ui::color::WHITE, true);

    line.clear();
    stat(line, "Str", s.strength);
    pad(line, 9);
    stat(line, "Agi", s.agility);
    pad(line, 18);
    stat(line, "Sta", s.stamina);
    ui::text(2, y + 2, line, ui::color::WHITE, true);

    line.clear();
    stat(line, "Int", s.intellect);
    pad(line, 9);
    stat(line, "Spi", s.spirit);
    pad(line, 18);
    stat(line, "Arm", s.armor);
    ui::text(2, y + 3, line, ui::color::WHITE, true);

    line = "Damage ";
    line += bn::to_string<6>(s.melee_min);
    line += "-";
    line += bn::to_string<6>(s.melee_max);
    pad(line, 18);
    stat(line, "AP", s.attack_power);
    ui::text(2, y + 4, line, ui::color::WHITE, true);

    line = "Crit ";
    line += bn::to_string<4>(uses_mana() && data.player_class == class_id::MAGE ? s.spell_crit : s.crit);
    line += "%";
    pad(line, 9);
    line += "Dodge ";
    line += bn::to_string<4>(s.dodge);
    line += "%";
    ui::text(2, y + 5, line, ui::color::WHITE, true);

    ui::text(page_x, hint_row, "A Unequip", ui::color::WHITE, true);
    ui::money_right(27, hint_row, data.money);
}

// --- system -------------------------------------------------------------------------------------

void menu::_update_system()
{
    if(_teleport_list)
    {
        if(_cursor.update(destination_count, content_rows))
        {
            _dirty = true;
        }

        if(bn::keypad::b_pressed())
        {
            _teleport_list = false;
            _cursor = list_cursor();
            _cursor.index = int(system_entry::TELEPORT);
            _dirty = true;
        }
        else if(bn::keypad::a_pressed())
        {
            const destination& target = destinations[_cursor.index];
            teleport.map = target.map;
            teleport.x = target.point.x;
            teleport.y = target.point.y;
            _teleport_list = false;
            _open = false;
        }

        return;
    }

    if(_cursor.update(int(system_entry::COUNT), int(system_entry::COUNT)))
    {
        _dirty = true;
    }

    if(! bn::keypad::a_pressed())
    {
        return;
    }

    character_data& data = character();
    _dirty = true;

    switch(system_entry(_cursor.index))
    {

    case system_entry::SAVE:
        save_game();
        _status.show("Game saved", ui::color::GREEN);
        break;

    case system_entry::TELEPORT:
        _teleport_list = true;
        _cursor = list_cursor();
        break;

    case system_entry::LEVEL_UP:
        if(data.level < max_level)
        {
            _combat.gain_xp(xp_for_level(data.level) - data.xp);
            _status.show("Level up!", ui::color::YELLOW);
        }
        break;

    case system_entry::GOLD:
        data.money += 100000;
        _status.show("+10 gold", ui::color::YELLOW);
        break;

    default:
        break;
    }
}

void menu::_draw_system()
{
    if(_teleport_list)
    {
        for(int row = 0; row < destination_count; ++row)
        {
            int y = content_top + row;

            if(row == _cursor.index)
            {
                ui::cursor(2, y);
            }

            ui::text(4, y, destinations[row].name, ui::color::WHITE, true);
        }

        ui::text(page_x, hint_row, "A Go", ui::color::WHITE, true);
        ui::text_right(27, hint_row, "B Back", ui::color::GRAY, true);
        return;
    }

    for(int row = 0; row < int(system_entry::COUNT); ++row)
    {
        int y = content_top + row;

        if(row == _cursor.index)
        {
            ui::cursor(2, y);
        }

        ui::text(4, y, system_names[row], row == 0 ? ui::color::WHITE : ui::color::GRAY, true);
    }

    int y = content_top + int(system_entry::COUNT) + 1;
    ui::text(2, y, "Controls", ui::color::YELLOW, true);
    ui::text(2, y + 1, "A     Talk, loot, attack", ui::color::WHITE, true);
    ui::text(2, y + 2, "B     Run", ui::color::WHITE, true);
    ui::text(2, y + 3, "L     Next target", ui::color::WHITE, true);
    ui::text(2, y + 4, "R+key Use action bar", ui::color::WHITE, true);
    ui::text(2, y + 5, "SEL   Potion / food", ui::color::WHITE, true);

    bn::string<32> played = "Played ";
    int minutes = int(character().play_frames / 3600);
    played += bn::to_string<6>(minutes / 60);
    played += "h ";
    played += bn::to_string<4>(minutes % 60);
    played += "m";
    ui::text(page_x, hint_row, "A Select", ui::color::WHITE, true);
    ui::text_right(27, hint_row, played, ui::color::GRAY, true);
}

}
