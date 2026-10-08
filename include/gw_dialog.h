#ifndef GW_DIALOG_H
#define GW_DIALOG_H

#include "bn_vector.h"

#include "gw_ids.h"
#include "gw_quest_ids.h"
#include "gw_text_page.h"
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
        HOME,
        GOODBYE
    };

    struct option
    {
        option_kind kind;
        quest_id quest;
    };

    static constexpr int max_options = 8;

    combat& _combat;
    hud& _hud;
    npcs& _npcs;
    state _state = state::CLOSED;
    npc_id _npc = npc_id::NONE;
    bn::vector<option, max_options> _options;
    int _cursor = 0;
    int _option_scroll = 0;
    bool _confirm_unlearn = false;
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
