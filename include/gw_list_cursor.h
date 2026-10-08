#ifndef GW_LIST_CURSOR_H
#define GW_LIST_CURSOR_H

namespace gw
{

// The selected row of a scrolling list moved with up and down (with key repeat).
struct list_cursor
{
    int index = 0;
    int scroll = 0;

    // Moves the cursor within count rows showing visible at a time. Returns true if it moved.
    bool update(int count, int visible);

    // Keeps the cursor inside the list after it shrank.
    void clamp(int count, int visible);
};

}

#endif
