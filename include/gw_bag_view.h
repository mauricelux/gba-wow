#ifndef GW_BAG_VIEW_H
#define GW_BAG_VIEW_H

#include "bn_vector.h"

#include "gw_character.h"

namespace gw
{

// The bags as the Bags page and vendors list them: in the player's sort order (character().sort),
// with a heading over each group. The cursor stays on items, skipping the headings.
class bag_view
{

public:
    static constexpr int max_groups = 16;

    // Builds the list again after the bags or the sort order changed. The cursor stays at about the
    // same place in the list.
    void rebuild();

    // Back to the first item (after a new sort order).
    void reset();

    // Up and down move the cursor (with key repeat), left and right jump to the previous or next
    // group. Returns true if it moved.
    bool update(int visible);

    // The bag row under the cursor, -1 with empty bags.
    [[nodiscard]] int selected_row() const;

    // Draws visible lines from row top: headings in yellow, items with the cursor at column 2.
    void draw(int top, int visible) const;

    [[nodiscard]] bool empty() const
    {
        return _entries.empty();
    }

    // The name of the sort order, for hints.
    [[nodiscard]] static const char* sort_name(bag_sort sort);

private:
    // A bag row, or a heading as -1 - group.
    bn::vector<int16_t, bag_rows + max_groups> _entries;
    int _index = 0;     // entry under the cursor
    int _scroll = 0;    // first entry shown
    int _visible = 9;

    [[nodiscard]] bool _is_item(int index) const
    {
        return index >= 0 && index < _entries.size() && _entries[index] >= 0;
    }

    void _settle(int direction);
    void _scroll_to_cursor();
};

}

#endif
