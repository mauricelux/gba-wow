#ifndef GW_TRAINER_SCREEN_H
#define GW_TRAINER_SCREEN_H

#include "bn_vector.h"

#include "gw_abilities.h"
#include "gw_ids.h"
#include "gw_list_cursor.h"
#include "gw_status_line.h"
#include "gw_text_page.h"

namespace gw
{

// A class trainer's list of abilities: learn new ones as you level up, for a fee.
class trainer_screen
{

public:
    trainer_screen();

    void open(npc_id npc);

    // Returns false once the player leaves (B).
    bool update();

private:
    npc_id _npc = npc_id::NONE;
    bn::vector<ability_id, 16> _abilities;
    list_cursor _cursor;
    text_page _details;
    status_line _status;
    bool _dirty = true;

    void _learn();
    void _draw();
};

}

#endif
