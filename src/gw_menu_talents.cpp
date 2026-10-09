#include "gw_menu.h"

#include "bn_keypad.h"
#include "bn_string.h"

#include "gw_character.h"
#include "gw_combat.h"
#include "gw_menu_layout.h"
#include "gw_talents.h"
#include "gw_ui.h"

namespace gw
{

using namespace menu_layout;

namespace
{
    constexpr int tree_row = content_top;
    constexpr int talents_top = content_top + 1;
    constexpr int talent_rows = details_top - 1 - talents_top;
}

void menu::_update_talents()
{
    if(_cursor.update(talents_per_tree, talent_rows))
    {
        _dirty = true;
    }

    if(! bn::keypad::a_pressed())
    {
        return;
    }

    _dirty = true;

    switch(can_learn_talent(_cursor.index))
    {

    case talent_check::OK:
    {
        learn_talent(_cursor.index);
        _combat.refresh_stats();
        const talent_def& def = get_talent(character().subclass, _cursor.index);
        bn::string<32> text = def.effect == talent_effect::ABILITY ? "Learned " : "";
        text += def.name;

        if(def.effect != talent_effect::ABILITY)
        {
            text += " ";
            text += bn::to_string<4>(talent_rank(_cursor.index));
            text += "/";
            text += bn::to_string<4>(def.ranks);
        }

        _status.show(text, ui::color::GREEN);
        break;
    }

    case talent_check::NO_POINTS:
        _status.show(character().level < first_talent_level ? "Talents open at level 10" : "No talent points left",
                     ui::color::RED);
        break;

    case talent_check::MAXED:
        _status.show("Already at the highest rank", ui::color::GRAY);
        break;

    case talent_check::TIER_LOCKED:
        _status.show("That tier is not open yet", ui::color::RED);
        break;

    case talent_check::LEVEL_TOO_LOW:
        _status.show("Your level is too low", ui::color::RED);
        break;

    case talent_check::COMING:
        _status.show("Not in the game yet", ui::color::GRAY);
        break;

    default:
        break;
    }
}

void menu::_draw_talents()
{
    subclass_id subclass = character().subclass;
    int tree_points = talent_points_total();

    bn::string<32> header = subclass_name(subclass);
    header += " talents (";
    header += bn::to_string<4>(tree_points);
    header += ")";
    ui::text_center(tree_row, header, ui::color::WHITE, true);

    for(int row = 0; row < talent_rows; ++row)
    {
        int index = _cursor.scroll + row;

        if(index >= talents_per_tree)
        {
            break;
        }

        const talent_def& def = get_talent(subclass, index);
        int rank = talent_rank(index);
        bool coming = def.effect == talent_effect::COMING;
        bool open = ! coming && tree_points >= def.tier * points_per_tier;
        int y = talents_top + row;
        ui::color color = rank >= def.ranks ? ui::color::GREEN : rank ? ui::color::YELLOW :
                          open ? ui::color::WHITE : ui::color::GRAY;

        if(index == _cursor.index)
        {
            ui::cursor(2, y);
        }

        ui::text(4, y, def.name, color, true);

        bn::string<8> ranks;

        if(coming)
        {
            ranks = "--";
        }
        else
        {
            ranks = bn::to_string<4>(rank);
            ranks += "/";
            ranks += bn::to_string<4>(def.ranks);
        }

        ui::text_right(27, y, ranks, color, true);
    }

    if(_cursor.scroll > 0)
    {
        ui::scroll_arrow(28, talents_top, true);
    }

    if(_cursor.scroll + talent_rows < talents_per_tree)
    {
        ui::scroll_arrow(28, talents_top + talent_rows - 1, false);
    }

    ui::divider(1, details_top - 1, ui::columns - 2);
    _page.clear();

    const talent_def& selected = get_talent(subclass, _cursor.index);
    int needed = selected.tier * points_per_tier;

    if(selected.effect == talent_effect::COMING)
    {
        _page.add("Coming in a later update", ui::color::GRAY);
    }
    else if(tree_points < needed)
    {
        bn::string<32> text = "Needs ";
        text += bn::to_string<4>(needed);
        text += " points in ";
        text += subclass_name(subclass);
        _page.add_copy(text, ui::color::RED);
    }
    else if(selected.effect == talent_effect::ABILITY && ! talent_rank(_cursor.index) &&
            character().level < ability_level(selected.ability))
    {
        bn::string<32> text = "Requires level ";
        text += bn::to_string<4>(ability_level(selected.ability));
        _page.add_copy(text, ui::color::RED);
    }

    _page.add(selected.description);
    _page.draw(page_x, details_top, details_rows, 0);

    int points = talent_points_available();
    bn::string<16> available = "Points ";
    available += bn::to_string<4>(points);
    ui::text(page_x, hint_row, "A Learn", ui::color::WHITE, true);
    ui::text_right(27, hint_row, available, points ? ui::color::GREEN : ui::color::GRAY, true);
}

}
