#ifndef GW_FLIGHT_H
#define GW_FLIGHT_H

#include "bn_optional.h"
#include "bn_sprite_affine_mat_ptr.h"
#include "bn_sprite_ptr.h"
#include "bn_sprite_text_generator.h"
#include "bn_vector.h"

#include "gw_travel.h"

namespace gw
{

// A flight on a gryphon: the screen fades to the continent seen up close, and the gryphon flies from
// one flight path to the other along a dotted route while the map scrolls under it. The world waits.
class flight
{

public:
    explicit flight(bn::sprite_text_generator& text_generator);

    void open(flight_id from, flight_id to);

    [[nodiscard]] bool is_open() const
    {
        return _frame >= 0;
    }

    // Returns false once it ended on a black screen: the game then lands the player and fades in.
    bool update();

    [[nodiscard]] flight_id destination() const
    {
        return _to;
    }

private:
    static constexpr int max_dots = 24;

    bn::sprite_text_generator& _text_generator;
    flight_id _from = flight_id::COUNT;
    flight_id _to = flight_id::COUNT;
    int _frame = -1;
    int _air_frames = 0;
    int _dot_count = 0;
    bn::optional<bn::sprite_affine_mat_ptr> _zoom;
    bn::vector<bn::sprite_ptr, 4> _picture;
    bn::vector<bn::sprite_ptr, max_dots + 2> _route;    // the dots, then both flight paths
    bn::vector<bn::sprite_ptr, 8> _label;
    bn::optional<bn::sprite_affine_mat_ptr> _gryphon_zoom;
    bn::optional<bn::sprite_ptr> _gryphon;

    void _show();
    void _place(int air_frame);
    void _clear();
};

}

#endif
