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
}

void menu::_update_talents()
{
    if(bn::keypad::left_pressed())
    {
        _talent_tree = (_talent_tree + talent_trees - 1) % talent_trees;
        _dirty = true;
    }
    else if(bn::keypad::right_pressed())
    {
        _talent_tree = (_talent_tree + 1) % talent_trees;
        _dirty = true;
    }

    if(_cursor.update(talents_per_tree, talents_per_tree))
    {
        _dirty = true;
    }

    if(! bn::keypad::a_pressed())
    {
        return;
    }

    _dirty = true;

    switch(can_learn_talent(_talent_tree, _cursor.index))
    {

    case talent_check::OK:
    {
        learn_talent(_talent_tree, _cursor.index);
        _combat.refresh_stats();
        const talent_def& def = get_talent(character().player_class, _talent_tree, _cursor.index);
        bn::string<32> text = def.effect == talent_effect::ABILITY ? "Learned " : "";
        text += def.name;

        if(def.effect != talent_effect::ABILITY)
        {
            text += " ";
            text += bn::to_string<4>(talent_rank(_talent_tree, _cursor.index));
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

    default:
        break;
    }
}

void menu::_draw_talents()
{
    class_id player_class = character().player_class;
    int tree_points = talent_points_in_tree(_talent_tree);

    bn::string<32> header = talent_tree_name(player_class, _talent_tree);
    header += " (";
    header += bn::to_string<4>(tree_points);
    header += ")";
    ui::text(2, tree_row, "<", ui::color::GRAY, true);
    ui::text_center(tree_row, header, ui::color::WHITE, true);
    ui::text_right(27, tree_row, ">", ui::color::GRAY, true);

    for(int index = 0; index < talents_per_tree; ++index)
    {
        const talent_def& def = get_talent(player_class, _talent_tree, index);
        int rank = talent_rank(_talent_tree, index);
        bool open = tree_points >= def.tier * points_per_tier;
        int y = talents_top + index;
        ui::color color = rank >= def.ranks ? ui::color::GREEN : rank ? ui::color::YELLOW :
                          open ? ui::color::WHITE : ui::color::GRAY;

        if(index == _cursor.index)
        {
            ui::cursor(2, y);
        }

        ui::text(4, y, def.name, color, true);

        bn::string<8> ranks = bn::to_string<4>(rank);
        ranks += "/";
        ranks += bn::to_string<4>(def.ranks);
        ui::text_right(27, y, ranks, color, true);
    }

    ui::divider(1, details_top - 1, ui::columns - 2);
    _page.clear();

    const talent_def& selected = get_talent(player_class, _talent_tree, _cursor.index);
    int needed = selected.tier * points_per_tier;

    if(tree_points < needed)
    {
        bn::string<32> text = "Needs ";
        text += bn::to_string<4>(needed);
        text += " points in ";
        text += talent_tree_name(player_class, _talent_tree);
        _page.add_copy(text, ui::color::RED);
    }

    _page.add(selected.description);
    _page.draw(page_x, details_top, details_rows, 0);

    int points = talent_points_available();
    bn::string<16> available = "Points ";
    available += bn::to_string<4>(points);
    ui::text(page_x, hint_row, "A Learn  < > Tree", ui::color::WHITE, true);
    ui::text_right(27, hint_row, available, points ? ui::color::GREEN : ui::color::GRAY, true);
}

}
