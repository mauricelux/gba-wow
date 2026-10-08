#include "gw_input.h"

namespace gw::input
{

namespace
{
    constexpr int repeat_delay = 18;
    constexpr int repeat_interval = 5;

    constexpr bn::keypad::key_type keys[] = {
        bn::keypad::key_type::UP, bn::keypad::key_type::DOWN, bn::keypad::key_type::LEFT,
        bn::keypad::key_type::RIGHT
    };

    int held_frames[4] = {};
}

void update()
{
    for(int index = 0; index < 4; ++index)
    {
        held_frames[index] = bn::keypad::held(keys[index]) ? held_frames[index] + 1 : 0;
    }
}

bool repeated(bn::keypad::key_type key)
{
    for(int index = 0; index < 4; ++index)
    {
        if(keys[index] == key)
        {
            int frames = held_frames[index];
            return frames == 1 || (frames > repeat_delay && (frames - repeat_delay) % repeat_interval == 0);
        }
    }

    return bn::keypad::pressed(key);
}

}
