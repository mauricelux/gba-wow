#include "gw_ending.h"

#include "bn_keypad.h"
#include "bn_string.h"

#include "gw_character.h"
#include "gw_quests.h"
#include "gw_ui.h"

namespace gw
{

namespace
{
    struct page_def
    {
        const char* title;
        const char* text;
    };

    constexpr page_def story_pages[] = {
        { "Westfall Is Free",
          "Edwin VanCleef is dead and the Defias Brotherhood is broken. The farmers of Westfall "
          "return to their fields, and the People's Militia raises its banner over Sentinel Hill." },
        { "A Hero of Elwynn",
          "Word of your deeds reaches Stormwind. Children in Goldshire play at being you, and "
          "Marshal Dughan buys the first round at the Lion's Pride Inn." },
    };

    constexpr int story_page_count = sizeof(story_pages) / sizeof(story_pages[0]);
    constexpr int page_count = story_page_count + 1;

    constexpr const char* race_names[] = { "Human", "Dwarf", "Night Elf" };
    constexpr const char* class_names[] = { "Warrior", "Mage", "Hunter" };
}

void ending::open()
{
    _open = true;
    _dirty = true;
    _page = 0;
    ui::clear();
}

bool ending::update()
{
    if(bn::keypad::a_pressed())
    {
        if(++_page >= page_count)
        {
            _open = false;
            ui::clear();
            return false;
        }

        _dirty = true;
    }

    if(_dirty)
    {
        _draw();
        _dirty = false;
    }

    return true;
}

void ending::_draw()
{
    ui::clear();
    ui::panel(0, 0, ui::columns, ui::rows);
    ui::divider(1, 2, ui::columns - 2);
    ui::divider(1, 16, ui::columns - 2);

    if(_page < story_page_count)
    {
        const page_def& page = story_pages[_page];
        ui::text_center(1, page.title, ui::color::YELLOW, true);
        ui::text_wrapped(2, 4, 26, 11, page.text, ui::color::WHITE, true);
        ui::text(2, 18, "A Continue", ui::color::WHITE, true);
        return;
    }

    const character_data& data = character();
    ui::text_center(1, "Thanks for Playing", ui::color::YELLOW, true);

    bn::string<32> line = "Level ";
    line += bn::to_string<4>(data.level);
    line += " ";
    line += race_names[int(data.race)];
    line += " ";
    line += class_names[int(data.player_class)];
    ui::text(2, 4, line, ui::color::WHITE, true);

    int turned_in = 0;

    for(int index = 1; index < int(quest_id::COUNT); ++index)
    {
        if(quest_state(quest_id(index)).status == quest_status::TURNED_IN)
        {
            ++turned_in;
        }
    }

    line = "Quests done ";
    line += bn::to_string<4>(turned_in);
    line += " of ";
    line += bn::to_string<4>(int(quest_id::COUNT) - 1);
    ui::text(2, 6, line, ui::color::WHITE, true);

    int minutes = int(data.play_frames / 3600);
    line = "Played ";
    line += bn::to_string<6>(minutes / 60);
    line += "h ";
    line += bn::to_string<4>(minutes % 60);
    line += "m";
    ui::text(2, 7, line, ui::color::WHITE, true);

    ui::text(2, 8, "Money", ui::color::WHITE, true);
    ui::money_right(27, 8, data.money);

    ui::text_wrapped(2, 10, 26, 5,
                     "The world stays open: finish your quests, hunt for better gear, or start over with "
                     "another race and class.", ui::color::GRAY, true);
    ui::text(2, 18, "A Keep exploring", ui::color::WHITE, true);
}

}
