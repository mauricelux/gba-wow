#include "gw_menu.h"

#include "bn_keypad.h"
#include "bn_math.h"
#include "bn_string.h"

#include "gw_character.h"
#include "gw_combat.h"
#include "gw_hud.h"
#include "gw_input.h"
#include "gw_map_deadmines.h"
#include "gw_map_deeprun_tram.h"
#include "gw_map_echo_ridge.h"
#include "gw_map_elwynn.h"
#include "gw_map_fargodeep.h"
#include "gw_map_stockade.h"
#include "gw_map_stormwind.h"
#include "gw_map_westfall.h"
#include "gw_menu_layout.h"
#include "gw_save.h"
#include "gw_types.h"
#include "gw_ui.h"

namespace gw
{

using namespace menu_layout;

namespace
{
    constexpr const char* tab_names[] = { "Character", "Bags", "Spellbook", "Talents", "Quest Log", "World Map",
                                          "System" };

    constexpr const char* slot_names[] = { "Head", "Chest", "Hands", "Legs", "Feet", "Main", "Off", "Range" };

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
        { "Echo Ridge Mine", map_id::ECHO_RIDGE, map_data::echo_ridge::entry },
        { "Fargodeep Mine", map_id::FARGODEEP, map_data::fargodeep::entry },
        { "Stormwind", map_id::STORMWIND, map_data::stormwind::from_elwynn },
        { "Sentinel Hill", map_id::WESTFALL, map_data::westfall::sentinel_respawn },
        { "Moonbrook", map_id::WESTFALL, map_data::westfall::deadmines_exit },
        { "The Deadmines", map_id::DEADMINES, map_data::deadmines::entry },
        { "Goblin Foundry", map_id::DEADMINES, { 640, 390 } },
        { "Ironclad Cove", map_id::DEADMINES, { 744, 200 } },
        { "The Stockade", map_id::STOCKADE, map_data::stockade::entry },
        { "Warden's Hall", map_id::STOCKADE, { 680, 88 } },
        { "Stormwind Gryphons", map_id::STORMWIND, map_data::stormwind::flight },
        { "Sentinel Gryphons", map_id::WESTFALL, map_data::westfall::flight },
        { "Dwarven District", map_id::STORMWIND, { 930, 240 } },
        { "Deeprun Tram", map_id::DEEPRUN_TRAM, map_data::deeprun_tram::entry },
    };

    constexpr int destination_count = sizeof(destinations) / sizeof(destinations[0]);

    // System page entries.
    enum class system_entry
    {
        SAVE,
        TELEPORT,
        LEVEL_UP,
        GOLD,
        GEAR,
        TRAIN,
        LOOT,
        COUNT
    };

    constexpr const char* system_names[] = { "Save game", "Debug: teleport", "Debug: level up", "Debug: +10 gold",
                                             "Debug: gear up", "Debug: train all", "Debug: loot" };

    // Learns every rank the trainer would teach now, for free.
    void train_all()
    {
        for(int index = 1; index < ability_count; ++index)
        {
            auto ability = ability_id(index);

            while(int rank = trainable_rank(ability))
            {
                learn_ability(ability, rank);
            }
        }
    }

    // A bagful of random items up to the character's level, to try the bags with.
    void loot()
    {
        for(int count = 0; count < 30; ++count)
        {
            auto item = item_id(random_range(1, int(item_id::COUNT) - 1));
            const item_def& def = get_item(item);

            if(def.level <= character().level && def.quality != item_quality::EPIC && def.type != item_type::QUEST &&
               def.type != item_type::HEARTHSTONE)
            {
                add_item(item, stackable(item) ? random_range(1, 5) : 1);
            }
        }
    }

    // Equips the best item the character can use in every slot, for testing later content. Epics are
    // left out: they are the story's last reward.
    void gear_up()
    {
        character_data& data = character();
        int best_level[int(equip_slot::COUNT)] = {};

        for(int slot = 0; slot < int(equip_slot::COUNT); ++slot)
        {
            item_id equipped = data.equipment[slot];
            best_level[slot] = equipped == item_id::NONE ? -1 : get_item(equipped).level * 4 +
                                                                int(get_item(equipped).quality);
        }

        for(int index = 1; index < int(item_id::COUNT); ++index)
        {
            const item_def& def = get_item(item_id(index));

            if(def.slot == equip_slot::NONE || def.level > data.level || def.quality == item_quality::EPIC ||
               ! can_equip(data.player_class, def))
            {
                continue;
            }

            int score = def.level * 4 + int(def.quality);
            int slot = int(def.slot);

            if(score > best_level[slot])
            {
                best_level[slot] = score;
                data.equipment[slot] = item_id(index);
            }
        }

        // No shield next to a two-handed weapon.
        item_id main_hand = data.equipment[int(equip_slot::MAIN_HAND)];

        if(main_hand != item_id::NONE && is_two_handed(get_item(main_hand)))
        {
            data.equipment[int(equip_slot::OFF_HAND)] = item_id::NONE;
        }
    }

    void stat(bn::istring& text, const char* name, int value)
    {
        text += name;
        text += " ";
        text += bn::to_string<6>(value);
    }

    void pad(bn::istring& text, int size)
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
    _item_action = -1;
    _item_bar_pick = false;
    _assign_slot = -1;
    _teleport_list = false;
    _cursor = list_cursor();
    _bags.rebuild();
    _status = status_line();
    teleport = teleport_request();
    _map_zone = -1;
    _map_continent = -1;
    ui::clear();
}

bool menu::_in_submode() const
{
    return _quest != quest_id::NONE || _confirm || _item_action >= 0 || _item_bar_pick || _assign_slot >= 0 ||
           _teleport_list;
}

bool menu::update()
{
    if(! _in_submode())
    {
        if(bn::keypad::start_pressed() || (bn::keypad::b_pressed() && ! _map_goes_up()))
        {
            _open = false;
            _clear_map();
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

    case tab::TALENTS:
        _update_talents();
        break;

    case tab::QUESTS:
        _update_quests();
        break;

    case tab::MAP:
        _update_map();
        break;

    case tab::SYSTEM:
        _update_system();
        break;

    default:
        break;
    }

    if(! _open)
    {
        _clear_map();
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

        case tab::TALENTS:
            _draw_talents();
            break;

        case tab::QUESTS:
            _draw_quests();
            break;

        case tab::MAP:
            _draw_map();
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
    _clear_map();
    _tab = tab((int(_tab) + count + direction) % count);
    _cursor = list_cursor();
    _bags.rebuild();
    _map_zone = -1;
    _map_continent = -1;
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
            _status.show("Your bags are full", ui::color::RED);
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

    // "Level 12 Human Fire Mage", leaving out the race, then shortening the level, until it fits.
    bn::string<48> line;

    for(int attempt = 0; attempt < 3; ++attempt)
    {
        line = attempt < 2 ? "Level " : "Lv ";
        line += bn::to_string<4>(data.level);
        line += " ";

        if(attempt == 0)
        {
            line += race_name(data.race);
            line += " ";
        }

        line += subclass_name(data.subclass);
        line += " ";
        line += class_name(data.player_class);

        if(line.size() <= ui::columns - 4)
        {
            break;
        }
    }

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

    if(s.block)
    {
        pad(line, 19);
        line += "Blk ";
        line += bn::to_string<4>(s.block);
        line += "%";
    }

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

    case system_entry::GEAR:
        gear_up();
        _combat.refresh_stats();
        _status.show("Geared up for your level", ui::color::YELLOW);
        break;

    case system_entry::TRAIN:
        train_all();
        _status.show("Trained every rank", ui::color::YELLOW);
        break;

    case system_entry::LOOT:
        loot();
        _status.show("Bags filled", ui::color::YELLOW);
        break;

    default:
        break;
    }
}

void menu::_draw_system()
{
    if(_teleport_list)
    {
        int rows = bn::min(destination_count - _cursor.scroll, content_rows);

        for(int line = 0; line < rows; ++line)
        {
            int row = _cursor.scroll + line;
            int y = content_top + line;

            if(row == _cursor.index)
            {
                ui::cursor(2, y);
            }

            ui::text(4, y, destinations[row].name, ui::color::WHITE, true);
        }

        if(_cursor.scroll > 0)
        {
            ui::scroll_arrow(28, content_top, true);
        }

        if(_cursor.scroll + content_rows < destination_count)
        {
            ui::scroll_arrow(28, content_top + content_rows - 1, false);
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
    ui::text(2, y + 1, "A    Talk, loot, attack", ui::color::WHITE, true);
    ui::text(2, y + 2, "B    Run     L tap Target", ui::color::WHITE, true);
    ui::text(2, y + 3, "R+   Combat  L+    Utility", ui::color::WHITE, true);
    ui::text(2, y + 4, "L+R+ Buffs   SEL+  Items", ui::color::WHITE, true);
    ui::text(2, y + 5, "SEL tap: potion or food", ui::color::WHITE, true);

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
