#ifndef GW_VENDOR_SCREEN_H
#define GW_VENDOR_SCREEN_H

#include "gw_ids.h"
#include "gw_list_cursor.h"
#include "gw_status_line.h"
#include "gw_text_page.h"

namespace gw
{

// Buying from and selling to a vendor. L and R switch between buying and selling.
class vendor_screen
{

public:
    vendor_screen();

    void open(npc_id npc);

    // Returns false once the player leaves (B).
    bool update();

private:
    npc_id _npc = npc_id::NONE;
    bool _selling = false;
    list_cursor _cursor;
    text_page _details;
    status_line _status;
    bool _dirty = true;

    [[nodiscard]] int _count() const;
    [[nodiscard]] int _bag_slot(int row) const;
    void _buy();
    void _sell();
    void _sell_junk();
    void _draw();
};

}

#endif
