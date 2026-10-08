#ifndef GW_MENU_H
#define GW_MENU_H

#include "gw_quest_ids.h"
#include "gw_text_page.h"

namespace gw
{

class combat;
class hud;
class npcs;

// The pause menu opened with Start. L and R switch between its pages; B closes it.
class menu
{

public:
    menu(combat& combat_ref, hud& hud_ref, npcs& npcs_ref);

    void open();

    [[nodiscard]] bool is_open() const
    {
        return _open;
    }

    // Reads input and redraws. Returns false once the menu closed.
    bool update();

private:
    enum class tab : uint8_t
    {
        QUESTS,
        COUNT
    };

    combat& _combat;
    hud& _hud;
    npcs& _npcs;
    bool _open = false;
    bool _dirty = true;
    tab _tab = tab::QUESTS;

    // Quest log
    int _quest_cursor = 0;
    int _quest_scroll = 0;
    quest_id _quest = quest_id::NONE;   // the quest whose details are shown, or NONE for the list
    bool _confirm_abandon = false;
    text_page _page;
    int _page_scroll = 0;

    void _switch_tab(int direction);
    void _draw_frame();
    void _update_quests();
    void _draw_quests();
    void _show_quest_details(quest_id quest);
};

}

#endif
