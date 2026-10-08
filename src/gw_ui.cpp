#include "gw_ui.h"

#include "bn_bg_tiles.h"
#include "bn_memory.h"
#include "bn_optional.h"
#include "bn_regular_bg_item.h"
#include "bn_regular_bg_map_item.h"
#include "bn_regular_bg_map_ptr.h"
#include "bn_regular_bg_ptr.h"
#include "bn_string.h"
#include "bn_unique_ptr.h"

#include "bn_bg_palette_items_ui_palette.h"
#include "bn_regular_bg_tiles_items_ui_tiles.h"

namespace gw::ui
{

namespace
{
    // Tile layout of graphics/ui_tiles.bmp, see tools/gen_ui.py.
    constexpr int glyph_tiles = 0;
    constexpr int panel_glyph_tiles = 95;
    constexpr int panel_fill = 95;
    constexpr int border_tiles = 190;
    constexpr int bar_left = 198;
    constexpr int bar_segments = 199;
    constexpr int bar_right = 208;
    constexpr int cursor_panel = 209;
    constexpr int cursor_map = 210;
    constexpr int arrow_up = 211;
    constexpr int arrow_down = 212;
    constexpr int coin_tiles = 213;
    constexpr int divider_tile = 216;

    constexpr int map_columns = 32;
    constexpr int map_rows = 32;

    struct layer
    {
        alignas(int) bn::regular_bg_map_cell cells[map_columns * map_rows];
        bn::regular_bg_map_item map_item;
        bn::optional<bn::regular_bg_ptr> bg;
        bn::optional<bn::regular_bg_map_ptr> map;
        bool dirty = false;

        layer() :
            map_item(cells[0], bn::size(map_columns, map_rows))
        {
            bn::memory::clear(cells);
        }
    };

    bn::unique_ptr<layer> data;

    [[nodiscard]] constexpr bn::regular_bg_map_cell make_cell(int tile, int palette)
    {
        return bn::regular_bg_map_cell(tile | (palette << 12));
    }

    void put(int x, int y, int tile, int palette)
    {
        if(x < 0 || y < 0 || x >= columns || y >= rows)
        {
            return;
        }

        data->cells[y * map_columns + x] = make_cell(tile, palette);
        data->dirty = true;
    }

    [[nodiscard]] int glyph_tile(char c, bool on_panel)
    {
        int code = (unsigned char) c;

        if(code < 32 || code > 126)
        {
            code = '?';
        }

        return (on_panel ? panel_glyph_tiles : glyph_tiles) + code - 32;
    }
}

void init()
{
    data.reset(new layer());

    // The map references the cells directly, so tiles can't be moved by an offset.
    bn::bg_tiles::set_allow_offset(false);
    bn::regular_bg_item item(bn::regular_bg_tiles_items::ui_tiles, bn::bg_palette_items::ui_palette,
                             data->map_item);
    data->bg = item.create_bg(8, 48);   // the top-left 30x20 cells cover the screen
    data->bg->set_priority(0);
    data->map = data->bg->map();
    bn::bg_tiles::set_allow_offset(true);
}

void set_visible(bool visible)
{
    data->bg->set_visible(visible);
}

void commit()
{
    if(data->dirty)
    {
        data->map->reload_cells_ref();
        data->dirty = false;
    }
}

void clear()
{
    bn::memory::clear(data->cells);
    data->dirty = true;
}

void clear_rect(int x, int y, int width, int height)
{
    for(int row = y; row < y + height; ++row)
    {
        for(int column = x; column < x + width; ++column)
        {
            put(column, row, 0, 0);
        }
    }
}

int text(int x, int y, const bn::string_view& string, color text_color, bool on_panel)
{
    int palette = int(text_color);
    int count = 0;

    for(char c : string)
    {
        put(x + count, y, glyph_tile(c, on_panel), palette);
        ++count;
    }

    return count;
}

void text_right(int right_x, int y, const bn::string_view& string, color text_color, bool on_panel)
{
    text(right_x - string.size() + 1, y, string, text_color, on_panel);
}

void text_center(int y, const bn::string_view& string, color text_color, bool on_panel)
{
    text((columns - string.size()) / 2, y, string, text_color, on_panel);
}

int text_wrapped(int x, int y, int width, int max_lines, const bn::string_view& string, color text_color,
                 bool on_panel)
{
    int line = 0;
    int start = 0;
    int size = string.size();

    while(start < size && line < max_lines)
    {
        // Explicit line breaks win; otherwise break at the last space that fits.
        int end = start;
        int last_space = -1;

        while(end < size && end - start < width && string[end] != '\n')
        {
            if(string[end] == ' ')
            {
                last_space = end;
            }

            ++end;
        }

        int next = end;

        if(end < size && string[end] != '\n' && last_space > start)
        {
            end = last_space;
            next = last_space + 1;
        }
        else if(end < size && string[end] == '\n')
        {
            next = end + 1;
        }

        text(x, y + line, string.substr(start, end - start), text_color, on_panel);
        start = next;
        ++line;
    }

    return line;
}

void panel(int x, int y, int width, int height)
{
    int right = x + width - 1;
    int bottom = y + height - 1;

    for(int row = y; row <= bottom; ++row)
    {
        for(int column = x; column <= right; ++column)
        {
            int tile = panel_fill;

            if(row == y)
            {
                tile = column == x ? border_tiles : column == right ? border_tiles + 2 : border_tiles + 1;
            }
            else if(row == bottom)
            {
                tile = column == x ? border_tiles + 5 : column == right ? border_tiles + 7 : border_tiles + 6;
            }
            else if(column == x)
            {
                tile = border_tiles + 3;
            }
            else if(column == right)
            {
                tile = border_tiles + 4;
            }

            put(column, row, tile, 0);
        }
    }
}

void divider(int x, int y, int width)
{
    for(int column = x; column < x + width; ++column)
    {
        put(column, y, divider_tile, 0);
    }
}

void bar(int x, int y, int width, int value, int max, bar_color fill)
{
    int palette = int(fill);
    int segments = width - 2;
    int pixels = segments * 8;
    int filled = max > 0 ? (value * pixels) / max : 0;

    if(value > 0 && filled == 0)
    {
        filled = 1;
    }

    if(filled > pixels)
    {
        filled = pixels;
    }

    put(x, y, bar_left, palette);

    for(int index = 0; index < segments; ++index)
    {
        int segment = filled - index * 8;
        segment = segment < 0 ? 0 : segment > 8 ? 8 : segment;
        put(x + 1 + index, y, bar_segments + segment, palette);
    }

    put(x + width - 1, y, bar_right, palette);
}

void cursor(int x, int y, bool on_panel)
{
    put(x, y, on_panel ? cursor_panel : cursor_map, 0);
}

void scroll_arrow(int x, int y, bool up)
{
    put(x, y, up ? arrow_up : arrow_down, 0);
}

void coin(int x, int y, int kind)
{
    put(x, y, coin_tiles + kind, 0);
}

void money_right(int right_x, int y, int copper, bool on_panel)
{
    int gold = copper / 10000;
    int silver = (copper / 100) % 100;
    int rest = copper % 100;
    int x = right_x;

    // Copper always shows; silver and gold only when there are any (or a higher unit shows).
    bn::string<8> text_value = bn::to_string<8>(rest);
    coin(x, y, 2);
    x -= text_value.size();
    text(x, y, text_value, color::WHITE, on_panel);
    --x;

    if(silver || gold)
    {
        text_value = bn::to_string<8>(silver);
        coin(x, y, 1);
        x -= text_value.size();
        text(x, y, text_value, color::WHITE, on_panel);
        --x;
    }

    if(gold)
    {
        text_value = bn::to_string<8>(gold);
        coin(x, y, 0);
        x -= text_value.size();
        text(x, y, text_value, color::WHITE, on_panel);
    }
}

}
