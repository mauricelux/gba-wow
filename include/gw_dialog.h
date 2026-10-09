#ifndef GW_DIALOG_H
#define GW_DIALOG_H

#include "bn_vector.h"

#include "gw_ids.h"
#include "gw_quest_ids.h"
#include "gw_text_page.h"
#include "gw_travel.h"
#include "gw_trainer_screen.h"
#include "gw_vendor_screen.h"

namespace gw
{

class combat;
class hud;
class npcs;

// Talking to an npc: their greeting with a menu of quests (and later goods and training), quest
// offers, progress and rewards. The world is paused while it is open.
class dialog
{

public:
    dialog(combat& combat_ref, hud& hud_ref, npcs& npcs_ref);

    void open(npc_id npc);

    [[nodiscard]] bool is_open() const
    {
        return _state != state::CLOSED;
    }

    // Reads input and redraws. Returns false once the dialog closed.
    bool update();

    // Set when the last quest was turned in; the game shows the ending once the dialog closes.
    bool ending_requested = false;

    // Set when the player chose to rest at an inn; the game fades out and in once the dialog closes.
    bool rest_requested = false;

    // Set when the player paid for a flight; the game flies them once the dialog closes.
    flight_id flight_from = flight_id::COUNT;
    flight_id flight_requested = flight_id::COUNT;

private:
    enum class state : uint8_t
    {
        CLOSED,
        GOSSIP,
        QUEST_OFFER,
        QUEST_PROGRESS,
        QUEST_REWARD,
        VENDOR,
        TRAINER
    };

    enum class option_kind : uint8_t
    {
        QUEST,
        VENDOR,
        TRAINER,
        UNLEARN,
        REST,
        HOME,
        FLIGHT,
        GOODBYE
    };

    struct option
    {
        option_kind kind;
        quest_id quest;
        flight_id flight = flight_id::COUNT;
    };

    // Up to seven quests, then rest, home, vendor, trainer, unlearn, flights and goodbye.
    static constexpr int max_quest_options = 7;
    static constexpr int max_options = max_quest_options + 6 + int(flight_id::COUNT);

    combat& _combat;
    hud& _hud;
    npcs& _npcs;
    state _state = state::CLOSED;
    npc_id _npc = npc_id::NONE;
    bn::vector<option, max_options> _options;
    int _cursor = 0;
    int _option_scroll = 0;
    bool _confirm_unlearn = false;
    bool _discovered = false;           // talking to a gryphon master found a new flight path
    quest_id _quest = quest_id::NONE;
    text_page _page;
    int _scroll = 0;
    int _reward = 0;
    int _reward_count = 0;
    bool _dirty = true;
    vendor_screen _vendor;
    trainer_screen _trainer;

    void _build_options();
    void _show_gossip();
    void _show_quest(quest_id quest);
    void _build_page();
    void _update_gossip();
    void _update_quest();
    void _draw_gossip();
    void _draw_quest();
    void _accept();
    void _complete();
    void _unlearn();
    void _close_or_continue();
    void _close();
};

}

#endif
