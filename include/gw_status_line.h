#ifndef GW_STATUS_LINE_H
#define GW_STATUS_LINE_H

#include "bn_algorithm.h"
#include "bn_string.h"
#include "bn_string_view.h"

#include "gw_ui.h"

namespace gw
{

// A short message shown inside a full-screen panel for a moment ("Not enough money"), since the
// hud is hidden there.
struct status_line
{
    bn::string<28> text;
    ui::color color = ui::color::WHITE;
    int frames = 0;

    void show(const bn::string_view& message, ui::color message_color)
    {
        text = message.substr(0, bn::min(message.size(), 28));
        color = message_color;
        frames = 120;
    }

    // Returns true when the message just went away (redraw needed).
    bool update()
    {
        return frames > 0 && --frames == 0;
    }

    // Draws the message centered on the row if it is showing; returns whether it did.
    bool draw(int row) const
    {
        if(frames <= 0)
        {
            return false;
        }

        ui::fill_panel(1, row, ui::columns - 2, 1);
        ui::text_center(row, text, color, true);
        return true;
    }
};

}

#endif
