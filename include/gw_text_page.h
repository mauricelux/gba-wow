#ifndef GW_TEXT_PAGE_H
#define GW_TEXT_PAGE_H

#include "bn_string.h"
#include "bn_string_view.h"
#include "bn_vector.h"

#include "gw_ui.h"

namespace gw
{

// A page of wrapped, colored lines drawn on a panel and scrolled a line at a time: quest texts,
// item details, talent descriptions.
class text_page
{

public:
    static constexpr int max_lines = 64;

    explicit text_page(int width) :
        _width(width)
    {
    }

    void clear();

    // Adds text wrapped to the page width. The text must outlive the page (string literals).
    void add(const bn::string_view& text, ui::color color = ui::color::WHITE, int indent = 0);

    // Adds text that is copied into the page, wrapped like add().
    void add_copy(const bn::string_view& text, ui::color color = ui::color::WHITE, int indent = 0);

    // A label with an amount of money right-aligned on the same line.
    void add_money(const bn::string_view& label, int copper);

    void add_blank();

    [[nodiscard]] int size() const
    {
        return _lines.size();
    }

    // The largest scroll that still fills the visible rows.
    [[nodiscard]] int max_scroll(int rows) const;

    // Draws rows lines starting at line scroll, with scroll arrows at column x + width when the
    // page does not fit.
    void draw(int x, int y, int rows, int scroll) const;

private:
    struct line
    {
        bn::string_view text;
        ui::color color;
        int8_t indent;
        int money;      // shown right-aligned when positive
    };

    int _width;
    bn::vector<line, max_lines> _lines;
    bn::vector<bn::string<64>, 16> _copies;
};

}

#endif
