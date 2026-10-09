#include "gw_ending.h"

#include "bn_keypad.h"
#include "bn_string.h"

#include "gw_audio.h"
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
        quest_id after;         // shown only once this quest is turned in, or always for NONE
    };

    constexpr page_def story_pages[] = {
        { "Westfall Is Free",
          "Edwin VanCleef is dead and the Defias Brotherhood is broken. The farmers of Westfall "
          "return to their fields, and the People's Militia raises its banner over Sentinel Hill.",
          quest_id::NONE },
        { "Lakeshire Stands",
          "Gath'Ilzogg's head hangs from the gate of Lakeshire. The Blackrock fall back beyond the "
          "mountains, Foreman Oslow mends the Everstill bridge and Bray wears a new hat.",
          quest_id::WANTED_GATH_ILZOGG },
        { "Quiet in the Stockade",
          "With Bazil Thredd dead, the riot dies with him. The last of VanCleef's plots ends in "
          "Stormwind's own prison, and Warden Thelwater locks the cells once more.",
          quest_id::NONE },
        { "Champion of Stormwind",
          "Highlord Bolvar names you a champion of the Alliance. Children in Goldshire play at being "
          "you, and Marshal Dughan buys the first round at the Lion's Pride Inn.",
          quest_id::NONE },
        { "The Road South",
          "But Bolvar's letters speak of darker roads. In Duskwood the dead walk and the night never "
          "ends, and Darkshire begs for help. Lord Ello Ebonlocke waits for a champion on the south "
          "road.",
          quest_id::NONE },
        { "Darkshire Holds",
          "Stitches lies dead on the road to Darkshire, and Abercrombie's hut stands empty. The Night "
          "Watch hangs its lamps a little farther out each night.",
          quest_id::STITCHES },
        { "Shadowfang Falls",
          "Archmage Arugal is dead and his worgen scatter into the hills of Silverpine. Ranger Valdan "
          "rides for Darkshire with the news, and north of the mountains the Wetlands wait.",
          quest_id::ARUGAL_MUST_DIE },
        { "Menethil Holds",
          "Nek'rosh is dead at the gate of Grim Batol and the Thandol Span still stands. Captain "
          "Stoutfist's men hang a Dragonmaw banner over the harbor tavern, upside down.",
          quest_id::DEFEAT_NEK_ROSH },
        { "Blackfathom Cleansed",
          "Aku'mai is dead and the Twilight's Hammer flees the drowned temple. The night elves of "
          "Auberdine light the Moonshrine again, and Argent Guard Thaelrid walks out into the sun.",
          quest_id::THE_FATHOM_CORE },
        { "Thermaplugg Falls",
          "Mekgineer Thermaplugg lies dead in the heart of Gnomeregan. High Tinker Mekkatorque swears "
          "his people will go home one day, and King Magni raises a toast to you in the Great Forge. "
          "Beyond the Thandol Span, the hills of Hillsbrad wait.",
          quest_id::THE_GRAND_BETRAYAL },
        { "Southshore Holds",
          "Gravis Slipknot is dead and the Syndicate scatter into Alterac. The fishing boats sail "
          "again, and Magistrate Maleb hangs a Syndicate mask on the town hall wall.",
          quest_id::SYNDICATE_LEADER },
        { "The Library Falls",
          "Arcanist Doan is dead among his books, and the Graveyard sleeps at last. The Argent Dawn "
          "watches the monastery doors. Beyond them, the Armory and the Cathedral wait.",
          quest_id::ARCANIST_DOAN },
        { "The Jungle Tamed",
          "King Bangalash's head hangs over Hemet Nesingwary's tent, and the Bloodsail fleet has lost "
          "its master. Baron Revilgaz lifts a glass to you from his balcony over Booty Bay.",
          quest_id::BIG_GAME_HUNTER },
        { "The Crusade Falls",
          "Mograine and Whitemane lie dead before the altar, and the Scarlet Monastery is silent. "
          "Across the sea, the deserts of Tanaris and the Razorfen thorns of Kalimdor wait.",
          quest_id::IN_THE_NAME_OF_THE_LIGHT },
        { "The Razorfen Fall",
          "Amnennar the Coldbringer is dust, and the dead of the Razorfen Downs lie still. Argent Guard "
          "Dalen raises the Dawn's banner over the barrows, and the Mirage Raceway runs a quiet race.",
          quest_id::BRING_THE_LIGHT },
        { "The Sands Settle",
          "Chief Ukorz Sandscalp lies dead on his throne, and the Sandfury turn on each other. "
          "Gadgetzan toasts you with real water. To the west, the forests of Feralas wait.",
          quest_id::CHIEF_UKORZ_SANDSCALP },
    };

    constexpr int story_page_total = sizeof(story_pages) / sizeof(story_pages[0]);

    [[nodiscard]] bool page_shown(const page_def& page)
    {
        return page.after == quest_id::NONE || quest_state(page.after).status == quest_status::TURNED_IN;
    }

    // The index-th story page shown, or nullptr past the last: then comes the numbers page.
    [[nodiscard]] const page_def* story_page(int index)
    {
        for(const page_def& page : story_pages)
        {
            if(page_shown(page) && index-- == 0)
            {
                return &page;
            }
        }

        return nullptr;
    }

    [[nodiscard]] int page_count()
    {
        int result = 1;

        for(int index = 0; index < story_page_total; ++index)
        {
            result += page_shown(story_pages[index]);
        }

        return result;
    }
}

void ending::open()
{
    _open = true;
    _dirty = true;
    _page = 0;
    ui::clear();
    play_music(music_id::TITLE);
}

bool ending::update()
{
    if(bn::keypad::a_pressed())
    {
        if(++_page >= page_count())
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

    if(const page_def* page = story_page(_page))
    {
        ui::text_center(1, page->title, ui::color::YELLOW, true);
        ui::text_wrapped(2, 4, 26, 11, page->text, ui::color::WHITE, true);
        ui::text(2, 18, "A Continue", ui::color::WHITE, true);
        return;
    }

    const character_data& data = character();
    ui::text_center(1, "End of Chapter One", ui::color::YELLOW, true);

    bn::string<32> line = "Level ";
    line += bn::to_string<4>(data.level);
    line += " ";
    line += race_name(data.race);
    line += " ";
    line += class_name(data.player_class);
    ui::text(2, 4, line, ui::color::WHITE, true);

    // Out of the quests this subclass can do.
    int turned_in = 0;
    int total = 0;

    for(int index = 1; index < int(quest_id::COUNT); ++index)
    {
        subclass_id only = get_quest(quest_id(index)).subclass;

        if(only == subclass_id::NONE || only == data.subclass)
        {
            ++total;

            if(quest_state(quest_id(index)).status == quest_status::TURNED_IN)
            {
                ++turned_in;
            }
        }
    }

    line = "Quests done ";
    line += bn::to_string<4>(turned_in);
    line += " of ";
    line += bn::to_string<4>(total);
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
                     "Thanks for playing! The world stays open: finish your quests, hunt for better gear, "
                     "or start over with another race and class.", ui::color::GRAY, true);
    ui::text(2, 18, "A Keep exploring", ui::color::WHITE, true);
}

}
