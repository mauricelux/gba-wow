#ifndef GW_VOYAGE_H
#define GW_VOYAGE_H

#include "bn_optional.h"
#include "bn_sprite_affine_mat_ptr.h"
#include "bn_sprite_ptr.h"
#include "bn_sprite_text_generator.h"
#include "bn_vector.h"

#include "gw_map_types.h"

namespace gw
{

// A crossing by boat or by the Deeprun Tram: the screen fades to a side view of the vehicle while the
// sea or the tunnel scrolls past it, then to black, and the game puts the player at the far end. The
// world waits meanwhile.
class voyage
{

public:
    explicit voyage(bn::sprite_text_generator& text_generator);

    // destination names the far end at the top of the screen.
    void open(vehicle ride, const char* destination);

    [[nodiscard]] bool is_open() const
    {
        return _frame >= 0;
    }

    // Returns false once it ended on a black screen.
    bool update();

private:
    static constexpr int strips_per_row = 5;
    static constexpr int max_rows = 3;

    bn::sprite_text_generator& _text_generator;
    vehicle _ride = vehicle::NONE;
    const char* _destination = "";
    int _frame = -1;
    bn::vector<bn::sprite_ptr, strips_per_row * max_rows> _strips;
    bn::optional<bn::sprite_affine_mat_ptr> _zoom;
    bn::optional<bn::sprite_ptr> _vehicle;
    bn::optional<bn::sprite_ptr> _moon;
    bn::vector<bn::sprite_ptr, 8> _label;

    void _show();
    void _place();
    void _clear();
};

}

#endif
