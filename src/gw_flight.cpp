#include "gw_flight.h"

#include "bn_math.h"

#include "bn_sprite_items_fx_map_marks.h"
#include "bn_sprite_items_fx_travel.h"

#include "gw_continents.h"
#include "gw_fade.h"
#include "gw_map_marks.h"
#include "gw_ui.h"

namespace gw
{

namespace
{
    constexpr int fade_frames = 16;
    constexpr int zoom = 2;
    constexpr int picture_size = 128;
    constexpr int margin = 8;                  // the picture's see-through border
    constexpr int half_height = 80 / zoom;     // the screen's half, in pixels of the picture
    constexpr int dot_spacing = 3;             // pixels of the picture between the route's dots
    constexpr int flap_frames = 8;
    constexpr int label_y = -64;

    // The frames of fx_travel (tools/gen_travel.py).
    enum travel_frame
    {
        GRYPHON,
        GRYPHON_FLAP
    };

    enum z_order
    {
        Z_LABEL = -1,
        Z_GRYPHON,
        Z_FLIGHTS,
        Z_DOTS,
        Z_PICTURE
    };

    [[nodiscard]] bn::fixed_point spot(flight_id flight)
    {
        const flight_def& def = get_flight(flight);
        return bn::fixed_point(def.x, def.y);
    }

    [[nodiscard]] bn::fixed_point along(const bn::fixed_point& from, const bn::fixed_point& to, bn::fixed progress)
    {
        return bn::fixed_point(from.x() + (to.x() - from.x()) * progress, from.y() + (to.y() - from.y()) * progress);
    }
}

flight::flight(bn::sprite_text_generator& text_generator) :
    _text_generator(text_generator)
{
}

void flight::open(flight_id from, flight_id to)
{
    _from = from;
    _to = to;
    _frame = 0;
    _air_frames = flight_frames(from, to);
}

bool flight::update()
{
    ++_frame;

    int air_start = fade_frames * 2;
    int air_end = air_start + _air_frames;

    if(_frame <= fade_frames)
    {
        // The world fades out, then the continent takes the screen.
        set_fade(bn::fixed(_frame) / fade_frames);

        if(_frame == fade_frames)
        {
            _show();
        }
    }
    else if(_frame <= air_start)
    {
        set_fade(bn::fixed(air_start - _frame) / fade_frames);
    }
    else if(_frame > air_end)
    {
        set_fade(bn::fixed(_frame - air_end) / fade_frames);

        if(_frame == air_end + fade_frames)
        {
            _clear();
            _frame = -1;
            return false;
        }
    }

    if(_gryphon)
    {
        _place(bn::clamp(_frame - air_start, 0, _air_frames));
    }

    return true;
}

void flight::_show()
{
    ui::clear();
    ui::panel(0, 0, ui::columns, ui::rows);

    bn::fixed_point from = spot(_from);
    bn::fixed_point to = spot(_to);
    const continent_def& continent = continents[get_flight(_from).continent];
    _zoom = bn::sprite_affine_mat_ptr::create();
    _zoom->set_scale(zoom);

    for(int index = 0; index < 4; ++index)
    {
        bn::sprite_ptr sprite = continent.item.create_sprite(0, 0, index);
        sprite.set_affine_mat(*_zoom);
        sprite.set_bg_priority(0);
        sprite.set_z_order(Z_PICTURE);
        _picture.push_back(bn::move(sprite));
    }

    bn::fixed dx = to.x() - from.x();
    bn::fixed dy = to.y() - from.y();
    int length = bn::sqrt(dx * dx + dy * dy).integer();
    _dot_count = bn::clamp(length / dot_spacing - 1, 0, max_dots);

    for(int index = 0; index < _dot_count + 2; ++index)
    {
        bool dot = index < _dot_count;
        bn::sprite_ptr sprite = bn::sprite_items::fx_map_marks.create_sprite(0, 0, dot ? MARK_ROUTE : MARK_FLIGHT);
        sprite.set_bg_priority(0);
        sprite.set_z_order(dot ? Z_DOTS : Z_FLIGHTS);
        _route.push_back(bn::move(sprite));
    }

    // The gryphon is drawn facing left, and zoomed like the picture.
    _gryphon_zoom = bn::sprite_affine_mat_ptr::create();
    _gryphon_zoom->set_scale(zoom);
    _gryphon_zoom->set_horizontal_flip(to.x() > from.x());
    _gryphon = bn::sprite_items::fx_travel.create_sprite(0, 0, GRYPHON);
    _gryphon->set_affine_mat(*_gryphon_zoom);
    _gryphon->set_bg_priority(0);
    _gryphon->set_z_order(Z_GRYPHON);

    int bg_priority = _text_generator.bg_priority();
    int z_order = _text_generator.z_order();
    _text_generator.set_bg_priority(0);
    _text_generator.set_z_order(Z_LABEL);
    _text_generator.set_center_alignment();
    _text_generator.generate(0, label_y, get_flight(_to).name, _label);
    _text_generator.set_bg_priority(bg_priority);
    _text_generator.set_z_order(z_order);
}

void flight::_place(int air_frame)
{
    bn::fixed_point from = spot(_from);
    bn::fixed_point to = spot(_to);
    bn::fixed progress = bn::fixed(air_frame) / _air_frames;
    bn::fixed_point position = along(from, to, progress);

    // Zoomed, the picture is a little narrower than the screen and stays centred between the panel's
    // sides; the camera follows the gryphon up and down, never past the picture's edges.
    bn::fixed_point camera(picture_size / 2, bn::clamp(position.y(), bn::fixed(margin + half_height),
                                                       bn::fixed(picture_size - margin - half_height)));

    auto screen = [&camera](const bn::fixed_point& point)
    {
        return (point - camera) * zoom;
    };

    for(int index = 0; index < _picture.size(); ++index)
    {
        _picture[index].set_position(screen(bn::fixed_point((index % 2) * 64 + 32, (index / 2) * 64 + 32)));
    }

    // The dots ahead of the gryphon, and both flight paths.
    for(int index = 0; index < _dot_count; ++index)
    {
        bn::fixed dot_progress = bn::fixed(index + 1) / (_dot_count + 1);
        _route[index].set_position(screen(along(from, to, dot_progress)));
        _route[index].set_visible(dot_progress > progress);
    }

    _route[_dot_count].set_position(screen(from));
    _route[_dot_count + 1].set_position(screen(to));

    bool flap = (_frame / flap_frames) & 1;
    _gryphon->set_tiles(bn::sprite_items::fx_travel.tiles_item(), flap ? GRYPHON_FLAP : GRYPHON);
    _gryphon->set_position(screen(position) + bn::fixed_point(0, flap ? -zoom : 0));
}

void flight::_clear()
{
    _label.clear();
    _gryphon.reset();
    _route.clear();
    _picture.clear();
    _zoom.reset();
    _gryphon_zoom.reset();
    ui::clear();
}

}
