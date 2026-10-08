#include "gw_menu.h"

#include "bn_keypad.h"
#include "bn_string.h"

#include "gw_hud.h"
#include "gw_input.h"
#include "gw_menu_layout.h"
#include "gw_npcs.h"
#include "gw_quest_text.h"
#include "gw_quests.h"
#include "gw_ui.h"

namespace gw
{

using namespace menu_layout;

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
        if(_confirm)
        {
            if(bn::keypad::a_pressed())
            {
                bn::string<48> text = "Abandoned: ";
                text += get_quest(_quest).title;
                _hud.message(text, ui::color::RED);
                abandon_quest(_quest);
                _npcs.refresh_markers();
                _confirm = false;
                _quest = quest_id::NONE;
                _cursor.clamp(quest_log_count(), content_rows);
                _dirty = true;
            }
            else if(bn::keypad::b_pressed())
            {
                _confirm = false;
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
            _confirm = true;
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

    if(_cursor.update(count, content_rows))
    {
        _dirty = true;
    }

    if(bn::keypad::a_pressed() && count > 0)
    {
        _show_quest_details(quest_log_at(_cursor.index));
    }
}

void menu::_draw_quests()
{
    if(_quest != quest_id::NONE)
    {
        ui::fill_panel(1, title_row, ui::columns - 2, 1);
        ui::text(page_x, title_row, get_quest(_quest).title, ui::color::YELLOW, true);
        _page.draw(page_x, content_top, content_rows, _page_scroll);

        if(_confirm)
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

    int count = quest_log_count();

    if(count == 0)
    {
        ui::text_center(8, "Your quest log is empty.", ui::color::GRAY, true);
        ui::text_center(10, "Look for people with a", ui::color::GRAY, true);
        ui::text_center(11, "yellow ! over their head.", ui::color::GRAY, true);
    }

    for(int row = 0; row < content_rows; ++row)
    {
        int index = _cursor.scroll + row;

        if(index >= count)
        {
            break;
        }

        quest_id quest = quest_log_at(index);
        const quest_def& def = get_quest(quest);
        int y = content_top + row;

        if(index == _cursor.index)
        {
            ui::cursor(2, y);
        }

        if(quest_state(quest).status == quest_status::COMPLETE)
        {
            ui::text(4, y, "?", ui::color::YELLOW, true);
        }

        ui::text(6, y, def.title, ui::color(level_color(def.level)), true);
    }

    if(_cursor.scroll > 0)
    {
        ui::scroll_arrow(28, content_top, true);
    }

    if(_cursor.scroll + content_rows < count)
    {
        ui::scroll_arrow(28, content_top + content_rows - 1, false);
    }

    bn::string<16> total = bn::to_string<4>(count);
    total += count == 1 ? " quest" : " quests";
    ui::text(page_x, hint_row, count ? "A Details" : "", ui::color::WHITE, true);
    ui::text_right(27, hint_row, total, ui::color::GRAY, true);
}

}
