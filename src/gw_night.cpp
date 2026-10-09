#include "gw_night.h"

#include "bn_array.h"
#include "bn_algorithm.h"
#include "bn_blending.h"
#include "bn_display.h"
#include "bn_math.h"
#include "bn_optional.h"
#include "bn_rect_window.h"
#include "bn_rect_window_boundaries_hbe_ptr.h"
#include "bn_window.h"

namespace gw::night
{

namespace
{
    constexpr int radius = 52;
    constexpr bn::fixed darkness = 0.5;
    constexpr int flicker_ticks = 6;

    constexpr int half_width = bn::display::width() / 2;
    constexpr int half_height = bn::display::height() / 2;

    struct state
    {
        // The left and right edge of the light on each scanline, relative to the middle of the screen.
        bn::array<bn::pair<bn::fixed, bn::fixed>, bn::display::height()> rows;
        bn::optional<bn::rect_window_boundaries_hbe_ptr> hbe;
        int x = 0;
        int y = 0;
        int r = 0;
        int ticks = 0;
        int flicker = 0;
        // Half chord widths per row from the center, for each flicker radius (radius - 1 to radius + 1).
        int16_t chords[3][radius + 2] = {};
        bool chords_ready = false;
    };

    state& data()
    {
        static state result;
        return result;
    }

    // Half the width of the circle at dy rows from its center, without floating point.
    int half_chord(int r, int dy)
    {
        int squared = r * r - dy * dy;

        if(squared < 0)
        {
            return -1;
        }

        int result = 0;

        while((result + 1) * (result + 1) <= squared)
        {
            ++result;
        }

        return result;
    }

    void fill(state& s)
    {
        if(! s.chords_ready)
        {
            for(int index = 0; index < 3; ++index)
            {
                for(int dy = 0; dy < radius + 2; ++dy)
                {
                    s.chords[index][dy] = int16_t(half_chord(radius - 1 + index, dy));
                }
            }

            s.chords_ready = true;
        }

        const int16_t* chords = s.chords[s.r - radius + 1];

        for(int row = 0; row < bn::display::height(); ++row)
        {
            int dy = bn::abs(row - half_height - s.y);
            int half = dy < radius + 2 ? chords[dy] : -1;

            if(half < 0)
            {
                s.rows[row] = bn::pair<bn::fixed, bn::fixed>(-half_width, -half_width);
            }
            else
            {
                // Odd rows reach one pixel less: a dithered rim reads softer than a hard edge.
                if(row & 1)
                {
                    half = bn::max(half - 1, 0);
                }

                int left = bn::clamp(s.x - half, -half_width, half_width);
                int right = bn::clamp(s.x + half + 1, -half_width, half_width);
                s.rows[row] = bn::pair<bn::fixed, bn::fixed>(left, right);
            }
        }
    }
}

void set_enabled(bool enabled)
{
    state& s = data();
    bn::rect_window light = bn::rect_window::internal();
    bn::window outside = bn::window::outside();

    if(enabled == s.hbe.has_value())
    {
        return;
    }

    if(enabled)
    {
        bn::blending::set_black_fade_color();
        bn::blending::set_fade_alpha(darkness);
        // The scanline edges below are offsets from the window's own left and right: the middle.
        light.set_boundaries(-half_height, 0, half_height, 0);
        light.set_show_blending(false);
        outside.set_show_blending(true);
        s.r = radius;
        s.x = 0;
        s.y = 0;
        fill(s);
        s.hbe = bn::rect_window_boundaries_hbe_ptr::create_horizontal(light, s.rows);
    }
    else
    {
        s.hbe.reset();
        light.set_boundaries(0, 0, 0, 0);
        light.set_show_blending(true);
        bn::blending::set_fade_alpha(0);
    }
}

bool enabled()
{
    return data().hbe.has_value();
}

void update(const bn::fixed_point& center)
{
    state& s = data();

    if(! s.hbe)
    {
        return;
    }

    // A slow flicker, like a lantern.
    if(++s.ticks >= flicker_ticks)
    {
        s.ticks = 0;
        s.flicker = (s.flicker + 1) & 7;
    }

    constexpr int flicker_offsets[] = { 0, 1, 1, 0, -1, 0, 1, 0 };
    int x = center.x().floor_integer();
    int y = center.y().floor_integer();
    int r = radius + flicker_offsets[s.flicker];

    if(x != s.x || y != s.y || r != s.r)
    {
        s.x = x;
        s.y = y;
        s.r = r;
        fill(s);
        s.hbe->reload_deltas_ref();
    }
}

}
