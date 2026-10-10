#include "gw_effects.h"

#include "bn_color.h"
#include "bn_math.h"
#include "bn_sprite_double_size_mode.h"

#include "bn_sprite_items_fx_circle.h"
#include "bn_sprite_items_fx_projectiles.h"
#include "bn_sprite_items_fx_target.h"

#include "gw_sprite_palettes.h"
#include "gw_world.h"

namespace gw
{

namespace
{
    // Frames of fx_projectiles, see tools/gen_effects.py.
    constexpr int fire_frames = 0;
    constexpr int frost_frames = 2;
    constexpr int arcane_frames = 4;
    constexpr int arrow_right = 6;
    constexpr int arrow_diagonal = 7;
    constexpr int arrow_down = 8;
    constexpr int slash_frames = 9;
    constexpr int fire_burst = 12;
    constexpr int frost_burst = 14;
    constexpr int arcane_burst = 16;
    constexpr int shadow_frames = 18;
    constexpr int nature_frames = 20;
    constexpr int holy_frames = 22;
    constexpr int shadow_burst = 24;
    constexpr int nature_burst = 26;
    constexpr int holy_burst = 28;

    constexpr bn::fixed projectile_speed = 3;

    // Spells and sparks are drawn over the overhead layer so tree tops never hide them.
    constexpr int effect_bg_priority = 1;

    // Frames of fx_target and fx_circle: each color is its own frame of one shared palette.
    constexpr int hostile_ring = 0;
    constexpr int friendly_ring = 1;

    [[nodiscard]] int burst_frame(projectile_kind kind)
    {
        switch(kind)
        {

        case projectile_kind::FIRE:
            return fire_burst;

        case projectile_kind::FROST:
            return frost_burst;

        case projectile_kind::ARCANE:
            return arcane_burst;

        case projectile_kind::SHADOW:
            return shadow_burst;

        case projectile_kind::NATURE:
            return nature_burst;

        case projectile_kind::HOLY:
            return holy_burst;

        default:
            return slash_frames;
        }
    }

    // The first of the two flight frames of a spell.
    [[nodiscard]] int flight_frame(projectile_kind kind)
    {
        switch(kind)
        {

        case projectile_kind::FIRE:
            return fire_frames;

        case projectile_kind::FROST:
            return frost_frames;

        case projectile_kind::SHADOW:
            return shadow_frames;

        case projectile_kind::NATURE:
            return nature_frames;

        case projectile_kind::HOLY:
            return holy_frames;

        default:
            return arcane_frames;
        }
    }
}

effects::effects(const bn::camera_ptr& camera) :
    _camera(camera)
{
}

void effects::_place(bn::sprite_ptr& sprite, const bn::fixed_point& world_position)
{
    bn::fixed_point screen = world::to_screen_space(world_position);
    sprite.set_position(screen.x().floor_integer(), screen.y().floor_integer());
}

void effects::set_target(const bn::fixed_point* feet, bool hostile)
{
    if(! feet)
    {
        _target_ring.reset();
        return;
    }

    int frame = hostile ? hostile_ring : friendly_ring;

    if(! _target_ring)
    {
        if(! sprite_palettes::fits(bn::sprite_items::fx_target.palette_item()))
        {
            return;
        }

        _target_ring = bn::sprite_items::fx_target.create_sprite(0, 0, frame);
        _target_ring->set_camera(_camera);
        _target_ring->set_bg_priority(2);
        _target_ring->set_z_order(32000);   // behind every character
    }
    else
    {
        _target_ring->set_tiles(bn::sprite_items::fx_target.tiles_item(), frame);
    }

    _place(*_target_ring, bn::fixed_point(feet->x(), feet->y() - 1));
}

void effects::spark(const bn::fixed_point& world_position)
{
    if(! sprite_palettes::fits(bn::sprite_items::fx_projectiles.palette_item()))
    {
        return;
    }

    if(_sparks.full())
    {
        _sparks.erase(_sparks.begin());
    }

    _sparks.push_back(
        timed_sprite{ bn::sprite_items::fx_projectiles.create_sprite(0, 0, slash_frames), world_position, 0,
                      slash_frames, 3, 12 });
    timed_sprite& spark = _sparks.back();
    spark.sprite.set_camera(_camera);
    spark.sprite.set_bg_priority(effect_bg_priority);
    _place(spark.sprite, world_position);
}

void effects::burst(const bn::fixed_point& world_position, projectile_kind kind)
{
    if(kind == projectile_kind::ARROW || kind == projectile_kind::NONE)
    {
        spark(world_position);
        return;
    }

    if(! sprite_palettes::fits(bn::sprite_items::fx_projectiles.palette_item()))
    {
        return;
    }

    if(_sparks.full())
    {
        _sparks.erase(_sparks.begin());
    }

    int frame = burst_frame(kind);
    _sparks.push_back(
        timed_sprite{ bn::sprite_items::fx_projectiles.create_sprite(0, 0, frame), world_position, 0, frame, 2,
                      14 });
    timed_sprite& item = _sparks.back();
    item.sprite.set_camera(_camera);
    item.sprite.set_bg_priority(effect_bg_priority);
    _place(item.sprite, world_position);
}

int effects::circle(const bn::fixed_point& world_position, int radius, int frames, circle_style style)
{
    if(_circles.full())
    {
        _circles.erase(_circles.begin());
    }

    int id = _next_circle++;

    if(! sprite_palettes::fits(bn::sprite_items::fx_circle.palette_item()))
    {
        return id;
    }

    // The frames follow circle_style.
    _circles.push_back(timed_sprite{ bn::sprite_items::fx_circle.create_sprite(0, 0, int(style)), world_position, 0,
                                     0, 1, frames, id });
    timed_sprite& item = _circles.back();
    item.sprite.set_camera(_camera);
    item.sprite.set_bg_priority(2);
    item.sprite.set_z_order(31000);
    item.sprite.set_double_size_mode(bn::sprite_double_size_mode::ENABLED);
    item.sprite.set_scale(bn::fixed(radius) / 32);
    _place(item.sprite, world_position);
    return id;
}

void effects::remove_circle(int id)
{
    for(auto it = _circles.begin(); it != _circles.end(); ++it)
    {
        if(it->id == id)
        {
            _circles.erase(it);
            return;
        }
    }
}

void effects::launch(const bn::fixed_point& from, projectile_kind kind, const projectile_hit& hit)
{
    if(_projectiles.full())
    {
        return;
    }

    // Without room for its palette the projectile still flies and hits, unseen.
    int frame = kind == projectile_kind::ARROW ? arrow_right : flight_frame(kind);
    projectile& item = _projectiles.emplace_back();
    item.position = from;
    item.kind = kind;
    item.hit = hit;

    if(sprite_palettes::fits(bn::sprite_items::fx_projectiles.palette_item()))
    {
        item.sprite = bn::sprite_items::fx_projectiles.create_sprite(0, 0, frame);
        item.sprite->set_camera(_camera);
        item.sprite->set_bg_priority(effect_bg_priority);
        _place(*item.sprite, from);
    }
}

bool effects::_move_projectile(projectile& item, const bn::fixed_point& target)
{
    bn::fixed_point aim(target.x(), target.y() - 10);
    bn::fixed dx = aim.x() - item.position.x();
    bn::fixed dy = aim.y() - item.position.y();
    bn::fixed length = bn::sqrt(dx * dx + dy * dy);

    if(length <= projectile_speed || item.frames > 180)
    {
        return true;
    }

    item.position += bn::fixed_point(dx * projectile_speed / length, dy * projectile_speed / length);
    ++item.frames;

    if(! item.sprite)
    {
        return false;
    }

    bn::sprite_ptr& sprite = *item.sprite;

    if(item.kind == projectile_kind::ARROW)
    {
        // Pick the arrow frame closest to the flight direction and mirror it as needed.
        bn::fixed ax = bn::abs(dx);
        bn::fixed ay = bn::abs(dy);
        int frame = ax > ay * 2 ? arrow_right : ay > ax * 2 ? arrow_down : arrow_diagonal;
        sprite.set_tiles(bn::sprite_items::fx_projectiles.tiles_item(), frame);
        sprite.set_horizontal_flip(dx < 0);
        sprite.set_vertical_flip(dy < 0 && frame != arrow_right);
    }
    else
    {
        sprite.set_tiles(bn::sprite_items::fx_projectiles.tiles_item(),
                         flight_frame(item.kind) + ((item.frames / 4) & 1));
    }

    _place(sprite, item.position);
    return false;
}

void effects::_update_effects()
{
    for(auto it = _sparks.begin(); it != _sparks.end();)
    {
        ++it->frames;

        if(it->frames >= it->lifetime)
        {
            it = _sparks.erase(it);
            continue;
        }

        int frame = it->first_frame + (it->frames * it->frame_count / it->lifetime);
        it->sprite.set_tiles(bn::sprite_items::fx_projectiles.tiles_item(), frame);
        ++it;
    }

    for(auto it = _circles.begin(); it != _circles.end();)
    {
        ++it->frames;

        if(it->frames >= it->lifetime)
        {
            it = _circles.erase(it);
            continue;
        }

        // Blink near the end so the player sees it is about to go off (or wear off).
        if(it->lifetime - it->frames <= bn::min(it->lifetime / 4, 60))
        {
            it->sprite.set_visible((it->frames & 4) == 0);
        }

        ++it;
    }
}

void effects::clear()
{
    _target_ring.reset();
    _projectiles.clear();
    _sparks.clear();
    _circles.clear();
}

}
