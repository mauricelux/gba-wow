#ifndef GW_MENU_H
#define GW_MENU_H

#include "bn_optional.h"
#include "bn_sprite_ptr.h"
#include "bn_vector.h"

#include "gw_bag_view.h"
#include "gw_ids.h"
#include "gw_list_cursor.h"
#include "gw_quest_ids.h"
#include "gw_status_line.h"
#include "gw_text_page.h"

namespace gw
{

class combat;
class hud;
class npcs;

// A place the debug page can send the player.
struct teleport_request
{
    map_id map = map_id::NONE;
    int x = 0;
    int y = 0;
};

// The pause menu opened with Start: character, bags, spellbook, talents, quest log, world map and
// system pages.
// L and R switch pages; B closes it.
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

    // Set by the debug page; the game moves the player there once the menu closed.
    teleport_request teleport;

private:
    enum class tab : uint8_t
    {
        CHARACTER,
        BAGS,
        SPELLS,
        TALENTS,
        QUESTS,
        MAP,
        SYSTEM,
        COUNT
    };

    combat& _combat;
    hud& _hud;
    npcs& _npcs;
    bool _open = false;
    bool _dirty = true;
    tab _tab = tab::CHARACTER;
    list_cursor _cursor;
    status_line _status;
    text_page _page;
    int _page_scroll = 0;

    // Sub-modes (L, R and B belong to the page while one is active)
    quest_id _quest = quest_id::NONE;   // quest whose details are shown
    bool _confirm = false;              // abandon quest or drop item
    int _item_action = -1;              // bags: the cursor in an item's action list
    bool _item_bar_pick = false;        // bags: choosing an Items bar slot
    int _assign_slot = -1;              // spellbook: choosing a bar slot
    int _assign_bar = 0;                // spellbook: the bar_id it goes on
    bool _teleport_list = false;        // system: choosing a destination

    // World map page: the picture, its markers and the blinking player dot.
    int _map_zone = -1;                 // index in gw::minimaps, -1 until the page is first drawn
    int _map_frame = 0;
    bn::vector<bn::sprite_ptr, 40> _map_sprites;
    bn::optional<bn::sprite_ptr> _map_player;

    [[nodiscard]] bool _in_submode() const;
    void _switch_tab(int direction);
    void _draw_frame();

    void _update_character();
    void _draw_character();

    bag_view _bags;

    void _update_bags();
    void _draw_bags();
    void _do_item_action(int action);

    void _update_spells();
    void _draw_spells();
    [[nodiscard]] int _known_count() const;
    [[nodiscard]] int _known_at(int row) const;

    void _update_talents();
    void _draw_talents();

    void _update_quests();
    void _draw_quests();
    void _show_quest_details(quest_id quest);

    void _update_map();
    void _draw_map();
    void _clear_map();

    void _update_system();
    void _draw_system();
};

}

#endif
