#ifndef GW_TRAINER_SCREEN_H
#define GW_TRAINER_SCREEN_H

#include "bn_string.h"
#include "bn_vector.h"

#include "gw_abilities.h"
#include "gw_ids.h"
#include "gw_list_cursor.h"
#include "gw_status_line.h"
#include "gw_text_page.h"

namespace gw
{

// A class trainer's list of the subclass's abilities: learn new ones and new ranks as you level up,
// for a fee.
class trainer_screen
{

public:
    trainer_screen();

    void open(npc_id npc);

    // Returns false once the player leaves (B).
    bool update();

private:
    npc_id _npc = npc_id::NONE;
    bn::vector<ability_id, 40> _abilities;
    list_cursor _cursor;
    text_page _details;
    status_line _status;
    bn::string<32> _rank_text;
    bool _dirty = true;

    void _learn();
    void _draw();
};

}

#endif
