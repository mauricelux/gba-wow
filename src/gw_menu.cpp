#include "gw_menu.h"

#include "bn_keypad.h"
#include "bn_math.h"
#include "bn_string.h"

#include "gw_hud.h"
#include "gw_input.h"
#include "gw_npcs.h"
#include "gw_quest_text.h"
#include "gw_quests.h"
#include "gw_ui.h"

namespace gw
{

namespace
{
    constexpr int content_top = 3;
    constexpr int content_rows = 14;
    constexpr int hint_row = 18;
    constexpr int page_x = 2;
    constexpr int page_width = 26;

    constexpr const char* tab_names[] = { "Quest Log" };
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
    _confirm_abandon = false;
    ui::clear();
}

bool menu::update()
{
    if(! _confirm_abandon && _quest == quest_id::NONE)
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

    case tab::QUESTS:
        _update_quests();
        break;

    default:
        break;
    }

    if(_dirty && _open)
    {
        _draw_frame();

        switch(_tab)
        {

        case tab::QUESTS:
            _draw_quests();
            break;

        default:
            break;
        }

        _dirty = false;
    }

    return _open;
}

void menu::_switch_tab(int direction)
{
    int count = int(tab::COUNT);

    if(count > 1)
    {
        _tab = tab((int(_tab) + count + direction) % count);
        _dirty = true;
    }
}

void menu::_draw_frame()
{
    ui::panel(0, 0, ui::columns, ui::rows);
    ui::divider(1, 2, ui::columns - 2);
    ui::divider(1, hint_row - 1, ui::columns - 2);

    if(int(tab::COUNT) > 1)
    {
        ui::text(2, 1, "L", ui::color::GRAY, true);
        ui::text_right(27, 1, "R", ui::color::GRAY, true);
    }
}

void menu::_show_quest_details(quest_id quest)
{
    const quest_def& def = get_quest(quest);
    _quest = quest;
    _page_scroll = 0;
    _page.clear();

    bn::string<32> level = "Level ";
    level += bn::to_string<4>(def.level);
    _page.add_copy(level, ui::color(level_color(def.level)));
    _page.add_blank();
    add_quest_objectives(_page, quest, true);
    _page.add_blank();
    _page.add(def.text);
    _page.add_blank();
    add_quest_rewards(_page, quest, -1);
    _dirty = true;
}

void menu::_update_quests()
{
    if(_quest != quest_id::NONE)
    {
        if(_confirm_abandon)
        {
            if(bn::keypad::a_pressed())
            {
                bn::string<48> text = "Abandoned: ";
                text += get_quest(_quest).title;
                _hud.message(text, ui::color::RED);
                abandon_quest(_quest);
                _npcs.refresh_markers();
                _confirm_abandon = false;
                _quest = quest_id::NONE;
                _quest_cursor = bn::max(0, bn::min(_quest_cursor, quest_log_count() - 1));
                _dirty = true;
            }
            else if(bn::keypad::b_pressed())
            {
                _confirm_abandon = false;
                _dirty = true;
            }

            return;
        }

        int max_scroll = _page.max_scroll(content_rows);

        if(input::repeated(bn::keypad::key_type::UP) && _page_scroll > 0)
        {
            --_page_scroll;
            _dirty = true;
        }
        else if(input::repeated(bn::keypad::key_type::DOWN) && _page_scroll < max_scroll)
        {
            ++_page_scroll;
            _dirty = true;
        }

        if(bn::keypad::select_pressed())
        {
            _confirm_abandon = true;
            _dirty = true;
        }
        else if(bn::keypad::b_pressed() || bn::keypad::a_pressed())
        {
            _quest = quest_id::NONE;
            _dirty = true;
        }

        return;
    }

    int count = quest_log_count();

    if(input::repeated(bn::keypad::key_type::UP) && _quest_cursor > 0)
    {
        --_quest_cursor;
        _dirty = true;
    }
    else if(input::repeated(bn::keypad::key_type::DOWN) && _quest_cursor < count - 1)
    {
        ++_quest_cursor;
        _dirty = true;
    }

    if(_quest_cursor < _quest_scroll)
    {
        _quest_scroll = _quest_cursor;
    }
    else if(_quest_cursor >= _quest_scroll + content_rows)
    {
        _quest_scroll = _quest_cursor - content_rows + 1;
    }

    if(bn::keypad::a_pressed() && count > 0)
    {
        _show_quest_details(quest_log_at(_quest_cursor));
    }
}

void menu::_draw_quests()
{
    if(_quest != quest_id::NONE)
    {
        ui::text(page_x, 1, get_quest(_quest).title, ui::color::YELLOW, true);
        _page.draw(page_x, content_top, content_rows, _page_scroll);

        if(_confirm_abandon)
        {
            ui::text(page_x, hint_row, "Abandon quest?", ui::color::RED, true);
            ui::text_right(27, hint_row, "A Yes  B No", ui::color::WHITE, true);
        }
        else
        {
            ui::text(page_x, hint_row, "SELECT Abandon", ui::color::GRAY, true);
            ui::text_right(27, hint_row, "B Back", ui::color::GRAY, true);
        }

        return;
    }

    ui::text_center(1, tab_names[int(_tab)], ui::color::YELLOW, true);
    int count = quest_log_count();

    if(count == 0)
    {
        ui::text_center(8, "Your quest log is empty.", ui::color::GRAY, true);
        ui::text_center(10, "Look for people with a", ui::color::GRAY, true);
        ui::text_center(11, "yellow ! over their head.", ui::color::GRAY, true);
    }

    for(int row = 0; row < content_rows; ++row)
    {
        int index = _quest_scroll + row;

        if(index >= count)
        {
            break;
        }

        quest_id quest = quest_log_at(index);
        const quest_def& def = get_quest(quest);
        int y = content_top + row;

        if(index == _quest_cursor)
        {
            ui::cursor(2, y);
        }

        if(quest_state(quest).status == quest_status::COMPLETE)
        {
            ui::text(4, y, "?", ui::color::YELLOW, true);
        }

        ui::text(6, y, def.title, ui::color(level_color(def.level)), true);
    }

    if(_quest_scroll > 0)
    {
        ui::scroll_arrow(28, content_top, true);
    }

    if(_quest_scroll + content_rows < count)
    {
        ui::scroll_arrow(28, content_top + content_rows - 1, false);
    }

    bn::string<16> total = bn::to_string<4>(count);
    total += count == 1 ? " quest" : " quests";
    ui::text(page_x, hint_row, count ? "A Details" : "", ui::color::WHITE, true);
    ui::text_right(27, hint_row, total, ui::color::GRAY, true);
}

}
