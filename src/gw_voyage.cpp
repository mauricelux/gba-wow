#include "gw_voyage.h"

#include "bn_math.h"

#include "bn_sprite_items_fx_scene.h"
#include "bn_sprite_items_fx_vehicles.h"

#include "gw_fade.h"
#include "gw_ui.h"

namespace gw
{

namespace
{
    constexpr int fade_frames = 16;
    constexpr int ride_frames = 300;
    constexpr int strip_width = 64;
    constexpr int label_y = -64;

    // The frames of fx_vehicles and fx_scene (tools/gen_travel.py).
    enum vehicle_frame
    {
        BOAT,
        TRAM,
        MOON
    };

    enum scene_frame
    {
        SEA_TOP,
        SEA_DEEP,
        TUNNEL_TOP,
        TUNNEL_BOTTOM
    };

    enum z_order
    {
        Z_LABEL = -1,
        Z_FRONT,        // the waves the boat sits in
        Z_VEHICLE,
        Z_STRIPS,
        Z_MOON
    };

    // A band of strips across the screen, scrolling left at speed / 4 pixels a frame.
    struct row_def
    {
        scene_frame frame;
        int y;
        int speed;
    };

    // The boat sits on the crests, the deeper water slides by faster, nearer to the eye.
    constexpr row_def sea_rows[] = { { SEA_TOP, 8, 4 }, { SEA_DEEP, 40, 6 }, { SEA_DEEP, 72, 8 } };

    // The tram runs fast between the vault, lamps passing, and the rail.
    constexpr row_def tunnel_rows[] = { { TUNNEL_TOP, -40, 16 }, { TUNNEL_BOTTOM, 40, 16 } };

    [[nodiscard]] bn::span<const row_def> rows(vehicle ride)
    {
        return ride == vehicle::BOAT ? bn::span<const row_def>(sea_rows) : bn::span<const row_def>(tunnel_rows);
    }
}

voyage::voyage(bn::sprite_text_generator& text_generator) :
    _text_generator(text_generator)
{
}

void voyage::open(vehicle ride, const char* destination)
{
    _ride = ride;
    _destination = destination;
    _frame = 0;
}

bool voyage::update()
{
    ++_frame;

    int ride_start = fade_frames * 2;
    int ride_end = ride_start + ride_frames;

    if(_frame <= fade_frames)
    {
        // The world fades out, then the scene takes the screen.
        set_fade(bn::fixed(_frame) / fade_frames);

        if(_frame == fade_frames)
        {
            _show();
        }
    }
    else if(_frame <= ride_start)
    {
        set_fade(bn::fixed(ride_start - _frame) / fade_frames);
    }
    else if(_frame > ride_end)
    {
        set_fade(bn::fixed(_frame - ride_end) / fade_frames);

        if(_frame == ride_end + fade_frames)
        {
            _clear();
            _frame = -1;
            return false;
        }
    }

    if(_vehicle)
    {
        _place();
    }

    return true;
}

void voyage::_show()
{
    ui::clear();
    ui::panel(0, 0, ui::columns, ui::rows);

    for(const row_def& row : rows(_ride))
    {
        for(int index = 0; index < strips_per_row; ++index)
        {
            bn::sprite_ptr strip = bn::sprite_items::fx_scene.create_sprite(0, row.y, int(row.frame));
            strip.set_bg_priority(0);
            strip.set_z_order(row.frame == SEA_TOP ? Z_FRONT : Z_STRIPS);
            _strips.push_back(bn::move(strip));
        }
    }

    if(_ride == vehicle::BOAT)
    {
        _moon = bn::sprite_items::fx_vehicles.create_sprite(-80, -48, MOON);
        _moon->set_bg_priority(0);
        _moon->set_z_order(Z_MOON);
    }

    // The vehicle is drawn small and zoomed like the gryphon.
    _zoom = bn::sprite_affine_mat_ptr::create();
    _zoom->set_scale(2);
    _vehicle = bn::sprite_items::fx_vehicles.create_sprite(0, 0, _ride == vehicle::BOAT ? BOAT : TRAM);
    _vehicle->set_affine_mat(*_zoom);
    _vehicle->set_bg_priority(0);
    _vehicle->set_z_order(Z_VEHICLE);

    int bg_priority = _text_generator.bg_priority();
    int z_order = _text_generator.z_order();
    _text_generator.set_bg_priority(0);
    _text_generator.set_z_order(Z_LABEL);
    _text_generator.set_center_alignment();
    _text_generator.generate(0, label_y, _destination, _label);
    _text_generator.set_bg_priority(bg_priority);
    _text_generator.set_z_order(z_order);
}

void voyage::_place()
{
    int index = 0;

    for(const row_def& row : rows(_ride))
    {
        int offset = (_frame * row.speed / 4) % strip_width;

        for(int column = 0; column < strips_per_row; ++column)
        {
            _strips[index].set_x(-strip_width * 3 / 2 + column * strip_width - offset);
            ++index;
        }
    }

    // The boat rides the swell; the tram car rattles on the rail.
    if(_ride == vehicle::BOAT)
    {
        _vehicle->set_y(-16 + bn::degrees_lut_sin((_frame * 3) % 360) * 2);
    }
    else
    {
        _vehicle->set_y((_frame / 4) % 4 == 0 ? -1 : 0);
    }
}

void voyage::_clear()
{
    _label.clear();
    _vehicle.reset();
    _zoom.reset();
    _moon.reset();
    _strips.clear();
    ui::clear();
}

}
