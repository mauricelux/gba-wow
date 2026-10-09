#ifndef GW_VENDOR_SCREEN_H
#define GW_VENDOR_SCREEN_H

#include "gw_bag_view.h"
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
    list_cursor _cursor;    // the vendor's goods
    bag_view _bags;         // the player's, to sell
    text_page _details;
    status_line _status;
    bool _dirty = true;

    [[nodiscard]] int _count() const;
    void _buy();
    void _sell();
    void _sell_junk();
    void _draw();
};

}

#endif
