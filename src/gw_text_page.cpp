#include "gw_text_page.h"

#include "bn_math.h"

namespace gw
{

void text_page::clear()
{
    _lines.clear();
    _copies.clear();
}

void text_page::add(const bn::string_view& text, ui::color color, int indent)
{
    int start = 0;
    int size = text.size();

    if(size == 0)
    {
        add_blank();
        return;
    }

    while(start < size && ! _lines.full())
    {
        int next;
        int end = ui::wrap_line(text, start, _width - indent, next);
        _lines.push_back(line{ text.substr(start, end - start), color, int8_t(indent), 0 });
        start = next;
    }
}

void text_page::add_copy(const bn::string_view& text, ui::color color, int indent)
{
    if(_copies.full())
    {
        return;
    }

    _copies.push_back(bn::string<64>(text.substr(0, bn::min(text.size(), 64))));
    add(_copies.back(), color, indent);
}

void text_page::add_money(const bn::string_view& label, int copper)
{
    if(! _lines.full())
    {
        _lines.push_back(line{ label, ui::color::WHITE, 0, copper });
    }
}

void text_page::add_blank()
{
    if(! _lines.full())
    {
        _lines.push_back(line{ bn::string_view(), ui::color::WHITE, 0, 0 });
    }
}

int text_page::max_scroll(int rows) const
{
    return bn::max(0, _lines.size() - rows);
}

void text_page::draw(int x, int y, int rows, int scroll) const
{
    ui::fill_panel(x, y, _width + 1, rows);

    for(int row = 0; row < rows; ++row)
    {
        int index = scroll + row;

        if(index >= _lines.size())
        {
            break;
        }

        const line& item = _lines[index];
        ui::text(x + item.indent, y + row, item.text, item.color, true);

        if(item.money > 0)
        {
            ui::money_right(x + _width - 1, y + row, item.money);
        }
    }

    if(scroll > 0)
    {
        ui::scroll_arrow(x + _width, y, true);
    }

    if(scroll < max_scroll(rows))
    {
        ui::scroll_arrow(x + _width, y + rows - 1, false);
    }
}

}
