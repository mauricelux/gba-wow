#include "gw_list_cursor.h"

#include "bn_math.h"

#include "gw_input.h"

namespace gw
{

bool list_cursor::update(int count, int visible)
{
    int old_index = index;

    if(input::repeated(bn::keypad::key_type::UP) && index > 0)
    {
        --index;
    }
    else if(input::repeated(bn::keypad::key_type::DOWN) && index < count - 1)
    {
        ++index;
    }

    clamp(count, visible);
    return index != old_index;
}

void list_cursor::clamp(int count, int visible)
{
    index = bn::max(0, bn::min(index, count - 1));

    if(index < scroll)
    {
        scroll = index;
    }
    else if(index >= scroll + visible)
    {
        scroll = index - visible + 1;
    }

    scroll = bn::max(0, bn::min(scroll, count - visible));
}

}
