#include "gw_dialog.h"

#include "bn_keypad.h"
#include "bn_math.h"
#include "bn_string.h"

#include "gw_audio.h"
#include "gw_combat.h"
#include "gw_homes.h"
#include "gw_hud.h"
#include "gw_input.h"
#include "gw_npc_data.h"
#include "gw_npcs.h"
#include "gw_quest_text.h"
#include "gw_quests.h"
#include "gw_save.h"
#include "gw_talents.h"
#include "gw_ui.h"

namespace gw
{

namespace
{
    // Greeting panel at the bottom of the screen.
    constexpr int gossip_top = 7;
    constexpr int gossip_height = ui::rows - gossip_top;
    constexpr int options_top = 14;
    constexpr int visible_options = 5;
    constexpr int unlearn_cost = 1000;      // copper

    // Full screen quest pages.
    constexpr int page_x = 2;
    constexpr int page_top = 3;
    constexpr int page_rows = 14;
    constexpr int page_width = 26;
    constexpr int hint_row = 18;
}

dialog::dialog(combat& combat_ref, hud& hud_ref, npcs& npcs_ref) :
    _combat(combat_ref),
    _hud(hud_ref),
    _npcs(npcs_ref),
    _page(page_width)
{
}

void dialog::open(npc_id npc)
{
    _npc = npc;
    _cursor = 0;
    _option_scroll = 0;
    _discovered = discover_flight(master_flight(npc));

    if(_discovered)
    {
        play_sound(sound_id::QUEST);
        save_game();
    }

    _show_gossip();
}

bool dialog::update()
{
    switch(_state)
    {

    case state::GOSSIP:
        _update_gossip();
        break;

    case state::QUEST_OFFER:
    case state::QUEST_PROGRESS:
    case state::QUEST_REWARD:
        _update_quest();
        break;

    case state::VENDOR:
        if(! _vendor.update())
        {
            _show_gossip();
        }

        return true;

    case state::TRAINER:
        if(! _trainer.update())
        {
            _show_gossip();
        }

        return true;

    default:
        break;
    }

    if(_dirty && _state != state::CLOSED)
    {
        if(_state == state::GOSSIP)
        {
            _draw_gossip();
        }
        else
        {
            _draw_quest();
        }

        _dirty = false;
    }

    return _state != state::CLOSED;
}

void dialog::_build_options()
{
    _options.clear();

    quest_id quests[max_quest_options];
    int count = npc_quests(_npc, quests, max_quest_options);

    for(int index = 0; index < count; ++index)
    {
        _options.push_back(option{ option_kind::QUEST, quests[index] });
    }

    const npc_info& info = get_npc_info(_npc);

    home_id home = innkeeper_home(_npc);

    if(home != home_id::COUNT)
    {
        _options.push_back(option{ option_kind::REST, quest_id::NONE });
    }

    if(home != home_id::COUNT && int(home) != character().home)
    {
        _options.push_back(option{ option_kind::HOME, quest_id::NONE });
    }

    if(info.flags & npc_flag::VENDOR)
    {
        _options.push_back(option{ option_kind::VENDOR, quest_id::NONE });
    }

    if((info.flags & npc_flag::TRAINER) && teaches(info.trainer_class))
    {
        _options.push_back(option{ option_kind::TRAINER, quest_id::NONE });

        if(talent_points_total() > 0 && info.trainer_class != any_class)
        {
            _options.push_back(option{ option_kind::UNLEARN, quest_id::NONE });
        }
    }

    // Flights to every path the player knows on this continent.
    flight_id here = master_flight(_npc);

    if(here != flight_id::COUNT)
    {
        for(int index = 0; index < int(flight_id::COUNT); ++index)
        {
            flight_id flight = flight_id(index);

            if(flight != here && flight_known(flight) &&
               get_flight(flight).continent == get_flight(here).continent)
            {
                _options.push_back(option{ option_kind::FLIGHT, quest_id::NONE, flight });
            }
        }
    }

    _options.push_back(option{ option_kind::GOODBYE, quest_id::NONE });
    _cursor = bn::min(_cursor, _options.size() - 1);
    _confirm_unlearn = false;
}

void dialog::_show_gossip()
{
    _build_options();
    _state = state::GOSSIP;
    _dirty = true;
    ui::clear();
}

void dialog::_show_quest(quest_id quest)
{
    _quest = quest;
    _scroll = 0;
    _reward = 0;
    _reward_count = quest_reward_count(quest);

    quest_status status = quest_state(quest).status;
    _state = status == quest_status::NOT_STARTED ? state::QUEST_OFFER :
             status == quest_status::COMPLETE ? state::QUEST_REWARD : state::QUEST_PROGRESS;
    _build_page();
    ui::clear();
}

void dialog::_build_page()
{
    const quest_def& def = get_quest(_quest);
    _page.clear();

    switch(_state)
    {

    case state::QUEST_OFFER:
        _page.add(def.text);
        _page.add_blank();
        add_quest_objectives(_page, _quest, false);
        _page.add_blank();
        add_quest_rewards(_page, _quest, _reward);
        break;

    case state::QUEST_REWARD:
        _page.add(def.completion);
        _page.add_blank();
        add_quest_rewards(_page, _quest, _reward);
        break;

    default:
        _page.add(def.progress);
        _page.add_blank();
        add_quest_objectives(_page, _quest, true);
        break;
    }

    _dirty = true;
}

void dialog::_update_gossip()
{
    int count = _options.size();

    if(input::repeated(bn::keypad::key_type::UP) && _cursor > 0)
    {
        --_cursor;
        _confirm_unlearn = false;
        _dirty = true;
    }
    else if(input::repeated(bn::keypad::key_type::DOWN) && _cursor < count - 1)
    {
        ++_cursor;
        _confirm_unlearn = false;
        _dirty = true;
    }

    if(_cursor < _option_scroll)
    {
        _option_scroll = _cursor;
    }
    else if(_cursor >= _option_scroll + visible_options)
    {
        _option_scroll = _cursor - visible_options + 1;
    }

    if(bn::keypad::b_pressed())
    {
        _close();
        return;
    }

    if(bn::keypad::a_pressed())
    {
        const option& selected = _options[_cursor];

        switch(selected.kind)
        {

        case option_kind::QUEST:
            _show_quest(selected.quest);
            break;

        case option_kind::VENDOR:
            _state = state::VENDOR;
            _vendor.open(_npc);
            break;

        case option_kind::TRAINER:
            _state = state::TRAINER;
            _trainer.open(_npc);
            break;

        case option_kind::UNLEARN:
            _unlearn();
            break;

        case option_kind::REST:
            rest_requested = true;
            _close();
            break;

        case option_kind::FLIGHT:
        {
            flight_id here = master_flight(_npc);
            int cost = flight_cost(here, selected.flight);

            if(character().money >= cost)
            {
                character().money -= cost;
                play_sound(sound_id::COIN);
                flight_from = here;
                flight_requested = selected.flight;
                _close();
            }
            break;
        }

        case option_kind::HOME:
            character().home = uint8_t(innkeeper_home(_npc));
            _hud.message("This inn is now your home", ui::color::YELLOW);
            save_game();
            _close();
            break;

        default:
            _close();
            break;
        }
    }
}

void dialog::_update_quest()
{
    int max_scroll = _page.max_scroll(page_rows);

    if(input::repeated(bn::keypad::key_type::UP) && _scroll > 0)
    {
        --_scroll;
        _dirty = true;
    }
    else if(input::repeated(bn::keypad::key_type::DOWN) && _scroll < max_scroll)
    {
        ++_scroll;
        _dirty = true;
    }

    if(_state == state::QUEST_REWARD && _reward_count > 1)
    {
        int reward = _reward;

        if(bn::keypad::left_pressed())
        {
            reward = (reward + _reward_count - 1) % _reward_count;
        }
        else if(bn::keypad::right_pressed())
        {
            reward = (reward + 1) % _reward_count;
        }

        if(reward != _reward)
        {
            _reward = reward;
            _build_page();
        }
    }

    if(bn::keypad::b_pressed())
    {
        _show_gossip();
        return;
    }

    if(bn::keypad::a_pressed())
    {
        switch(_state)
        {

        case state::QUEST_OFFER:
            _accept();
            break;

        case state::QUEST_REWARD:
            _complete();
            break;

        default:
            _show_gossip();
            break;
        }
    }
}

void dialog::_accept()
{
    accept_quest(_quest);

    bn::string<48> text = "Accepted: ";
    text += get_quest(_quest).title;
    _hud.message(text, ui::color::YELLOW);

    if(quest_state(_quest).status == quest_status::COMPLETE && get_quest(_quest).ender == _npc)
    {
        _show_quest(_quest);
    }
    else
    {
        _close_or_continue();
    }
}

void dialog::_complete()
{
    const quest_def& def = get_quest(_quest);

    if(! turn_in_quest(_quest, _reward_count ? _reward : -1))
    {
        _hud.message("Inventory is full", ui::color::RED);
        return;
    }

    bn::string<48> text = def.title;
    text += " completed";
    _hud.message(text, ui::color::GREEN);
    play_sound(sound_id::QUEST);

    if(_reward_count)
    {
        const item_def& item = get_item(def.rewards[_reward]);
        _hud.message(item.name, ui::color(quality_color(item.quality)));
        _combat.refresh_stats();
    }

    _combat.gain_xp(def.xp);
    save_game();

    if(_quest == quest_id::BAZIL_THREDD)
    {
        ending_requested = true;
    }

    _close_or_continue();
}

void dialog::_unlearn()
{
    character_data& data = character();

    if(! _confirm_unlearn)
    {
        _confirm_unlearn = true;
        _dirty = true;
        return;
    }

    if(data.money < unlearn_cost)
    {
        _hud.message("Not enough money", ui::color::RED);
        _close();
        return;
    }

    data.money -= unlearn_cost;
    reset_talents();
    _combat.refresh_stats();
    _hud.message("Your talents are unlearned", ui::color::YELLOW);
    save_game();
    _close();
}

void dialog::_close_or_continue()
{
    // Stay only when the npc has more quests to give or take (turning one in often unlocks the
    // next), like the quest window in WoW.
    _cursor = 0;
    _build_options();

    for(const option& item : _options)
    {
        if(item.kind == option_kind::QUEST && quest_state(item.quest).status != quest_status::ACTIVE)
        {
            _show_gossip();
            return;
        }
    }

    _close();
}

void dialog::_close()
{
    _state = state::CLOSED;
    _npcs.refresh_markers();
    ui::clear();
}

void dialog::_draw_gossip()
{
    const npc_info& info = get_npc_info(_npc);
    ui::panel(0, gossip_top, ui::columns, gossip_height);

    int row = gossip_top + 1;
    ui::text(2, row, info.name, ui::color::YELLOW, true);
    ++row;

    if(info.subtitle[0])
    {
        bn::string<32> subtitle = "<";
        subtitle += info.subtitle;
        subtitle += ">";
        ui::text(2, row, subtitle, ui::color::GRAY, true);
        ++row;
    }

    int gossip_rows = options_top - 1 - row - (_discovered ? 1 : 0);
    ui::text_wrapped(2, row, 26, gossip_rows, info.gossip, ui::color::WHITE, true);

    if(_discovered)
    {
        ui::text(2, options_top - 2, "New flight path discovered!", ui::color::GREEN, true);
    }

    ui::divider(1, options_top - 1, ui::columns - 2);

    for(int line = 0; line < visible_options; ++line)
    {
        int index = _option_scroll + line;

        if(index >= _options.size())
        {
            break;
        }

        const option& item = _options[index];
        int y = options_top + line;

        if(index == _cursor)
        {
            ui::cursor(2, y);
        }

        if(item.kind == option_kind::QUEST)
        {
            quest_status status = quest_state(item.quest).status;
            const quest_def& def = get_quest(item.quest);
            bool complete = status == quest_status::COMPLETE;
            bool active = status == quest_status::ACTIVE;
            ui::text(4, y, complete || active ? "?" : "!", active ? ui::color::GRAY : ui::color::YELLOW, true);
            ui::text(6, y, def.title, ui::color(level_color(def.level)), true);
        }
        else if(item.kind == option_kind::VENDOR)
        {
            ui::text(6, y, "Browse your goods", ui::color::WHITE, true);
        }
        else if(item.kind == option_kind::TRAINER)
        {
            ui::text(6, y, "Train me", ui::color::WHITE, true);
        }
        else if(item.kind == option_kind::REST)
        {
            ui::text(6, y, "Rest a while", ui::color::WHITE, true);
        }
        else if(item.kind == option_kind::HOME)
        {
            ui::text(6, y, "Make this inn your home", ui::color::WHITE, true);
        }
        else if(item.kind == option_kind::FLIGHT)
        {
            int cost = flight_cost(master_flight(_npc), item.flight);
            bool afford = character().money >= cost;
            ui::text(6, y, get_flight(item.flight).name, afford ? ui::color::WHITE : ui::color::GRAY, true);
            ui::money_right(27, y, cost);
        }
        else if(item.kind == option_kind::UNLEARN)
        {
            if(_confirm_unlearn)
            {
                ui::text(6, y, "Are you sure? Press A", ui::color::RED, true);
            }
            else
            {
                ui::text(6, y, "Unlearn talents", ui::color::WHITE, true);
                ui::money_right(27, y, unlearn_cost);
            }
        }
        else
        {
            ui::text(6, y, "Goodbye", ui::color::WHITE, true);
        }
    }

    if(_option_scroll > 0)
    {
        ui::scroll_arrow(28, options_top, true);
    }

    if(_option_scroll + visible_options < _options.size())
    {
        ui::scroll_arrow(28, options_top + visible_options - 1, false);
    }
}

void dialog::_draw_quest()
{
    const quest_def& def = get_quest(_quest);
    ui::panel(0, 0, ui::columns, ui::rows);
    ui::text(page_x, 1, def.title, ui::color::YELLOW, true);
    ui::divider(1, 2, ui::columns - 2);
    _page.draw(page_x, page_top, page_rows, _scroll);
    ui::divider(1, hint_row - 1, ui::columns - 2);

    switch(_state)
    {

    case state::QUEST_OFFER:
        ui::text(page_x, hint_row, "A Accept", ui::color::GREEN, true);
        ui::text_right(27, hint_row, "B Decline", ui::color::GRAY, true);
        break;

    case state::QUEST_REWARD:
        ui::text(page_x, hint_row, "A Complete", ui::color::GREEN, true);
        ui::text_right(27, hint_row, "B Back", ui::color::GRAY, true);
        break;

    default:
        ui::text(page_x, hint_row, "A Continue", ui::color::WHITE, true);
        ui::text_right(27, hint_row, "B Back", ui::color::GRAY, true);
        break;
    }
}

}
