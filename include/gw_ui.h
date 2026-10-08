#ifndef GW_UI_H
#define GW_UI_H

#include "bn_string_view.h"

namespace gw::ui
{

// Text colors. Each is a palette bank of graphics/ui_palette (tools/gen_ui.py).
enum class color : uint8_t
{
    WHITE,
    YELLOW,
    GRAY,
    GREEN,
    RED,
    BLUE,
    PURPLE
};

// Bar fill colors, sharing the banks above.
enum class bar_color : uint8_t
{
    HEALTH,     // green
    RAGE,       // red
    MANA,       // blue
    XP,         // purple
    CAST,       // orange
    NEUTRAL,    // gray
    GOLD
};

constexpr int columns = 30;   // visible cells
constexpr int rows = 20;

// Creates the UI background. Call before creating any other background so the UI owns the first
// palette banks and tile block.
void init();

void set_visible(bool visible);

// Uploads the cells changed this frame. Call once per frame.
void commit();

void clear();

void clear_rect(int x, int y, int width, int height);

// Writes text at a cell and returns the number of cells used. On a panel the glyphs are drawn over
// the panel fill instead of the map. Text is clipped at the right edge of the screen.
int text(int x, int y, const bn::string_view& text, color text_color = color::WHITE, bool on_panel = false);

// Right-aligned: the last character lands on cell right_x.
void text_right(int right_x, int y, const bn::string_view& text, color text_color = color::WHITE,
                bool on_panel = false);

// Centered on the screen width.
void text_center(int y, const bn::string_view& text, color text_color = color::WHITE, bool on_panel = false);

// Writes text wrapped at word boundaries within width cells. Returns the number of lines used.
int text_wrapped(int x, int y, int width, int max_lines, const bn::string_view& text,
                 color text_color = color::WHITE, bool on_panel = false);

// A bordered panel, filled.
void panel(int x, int y, int width, int height);

// A horizontal divider line inside a panel.
void divider(int x, int y, int width);

// A bar of the given width in cells (including its two end caps), filled value / max.
void bar(int x, int y, int width, int value, int max, bar_color fill);

// Menu cursor arrow.
void cursor(int x, int y, bool on_panel = true);

// Scroll arrows on a panel.
void scroll_arrow(int x, int y, bool up);

// A coin icon on a panel: 0 gold, 1 silver, 2 copper.
void coin(int x, int y, int kind);

// Writes "12g 34s 56c" style money on a panel, right-aligned at right_x.
void money_right(int right_x, int y, int copper, bool on_panel = true);

}

#endif
