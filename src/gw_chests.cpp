#include "gw_chests.h"

#include "bn_math.h"
#include "bn_string.h"

#include "bn_sprite_items_fx_chest.h"
#include "bn_sprite_items_fx_markers.h"

#include "gw_audio.h"
#include "gw_character.h"
#include "gw_hud.h"
#include "gw_items.h"
#include "gw_loot.h"
#include "gw_types.h"
#include "gw_ui.h"
#include "gw_world.h"

namespace gw
{

namespace
{
    constexpr int sprite_margin_x = 160;
    constexpr int sprite_margin_y = 120;
    constexpr int chest_bg_priority = 2;        // like characters: tree tops and roofs hide it
    constexpr int sparkle_bg_priority = 1;      // above them, so hidden chests still give a hint
    constexpr int sparkle_first_frame = 3;      // fx_markers: two twinkle frames
    constexpr int sparkle_period = 128;
    constexpr int sparkle_frames = 24;

    [[nodiscard]] item_id potion_for(int level)
    {
        return level <= 6 ? item_id::MINOR_HEALING_POTION :
               level <= 12 ? item_id::LESSER_HEALING_POTION : item_id::HEALING_POTION;
    }

    void money_message(hud& hud_ref, int money)
    {
        bn::string<28> text = "You find ";

        if(money >= 100)
        {
            text += bn::to_string<6>(money / 100);
            text += "s ";
        }

        if(money % 100)
        {
            text += bn::to_string<4>(money % 100);
            text += "c";
        }

        hud_ref.message(text, ui::color::YELLOW);
    }

    void item_message(hud& hud_ref, item_id item)
    {
        const item_def& def = get_item(item);
        hud_ref.message(def.name, ui::color(quality_color(def.quality)));
    }
}

chests::chests(const bn::camera_ptr& camera) :
    _camera(camera)
{
}

void chests::load(const map_info& map)
{
    _chests.clear();

    for(const chest_def& def : map.chests)
    {
        if(_chests.full())
        {
            break;
        }

        _chests.emplace_back();
        chest& item = _chests.back();
        item.id = def.id;
        item.level = def.level;
        item.position = bn::fixed_point(def.x, def.y);
    }
}

void chests::update(const bn::fixed_point& player_feet)
{
    ++_frame;

    for(chest& item : _chests)
    {
        _update_sprite(item, player_feet);
    }
}

int chests::nearest_closed(const bn::fixed_point& from, int max_distance) const
{
    int best = -1;
    int best_distance = max_distance * max_distance + 1;

    for(int index = 0, limit = _chests.size(); index < limit; ++index)
    {
        const chest& item = _chests[index];

        if(chest_opened(item.id))
        {
            continue;
        }

        // Measure to the middle of the chest, so it opens from any side.
        int distance = distance_squared(item.position - bn::fixed_point(0, 4), from);

        if(distance < best_distance)
        {
            best = index;
            best_distance = distance;
        }
    }

    return best;
}

bool chests::open(int index, hud& hud_ref)
{
    chest& item = _chests[index];

    if(free_bag_slots() == 0)
    {
        hud_ref.message("Inventory is full", ui::color::RED);
        return false;
    }

    int level = item.level;
    set_chest_opened(item.id);
    play_sound(sound_id::COIN);
    hud_ref.message("Treasure!", ui::color::YELLOW);

    int money = random_range(level * level * 2 + level * 15, level * level * 3 + level * 25);
    character().money += money;
    money_message(hud_ref, money);

    item_id gear = roll_world_drop(level);
    add_item(gear);
    item_message(hud_ref, gear);

    if(random_chance(50))
    {
        item_id potion = potion_for(level);

        if(add_item(potion) == 0)
        {
            item_message(hud_ref, potion);
        }
    }

    if(item.sprite)
    {
        item.sprite->set_tiles(bn::sprite_items::fx_chest.tiles_item(), 1);
    }

    item.sparkle.reset();
    return true;
}

void chests::_update_sprite(chest& item, const bn::fixed_point& player_feet)
{
    bool near = bn::abs(item.position.x() - player_feet.x()) < sprite_margin_x &&
                bn::abs(item.position.y() - player_feet.y()) < sprite_margin_y;

    if(! near)
    {
        item.sprite.reset();
        item.sparkle.reset();
        return;
    }

    bool opened = chest_opened(item.id);
    bn::fixed_point screen = world::to_screen_space(item.position);
    int x = screen.x().floor_integer();
    int y = screen.y().floor_integer();

    if(! item.sprite)
    {
        item.sprite = bn::sprite_items::fx_chest.create_sprite(x, y - 8, opened ? 1 : 0);
        item.sprite->set_camera(_camera);
        item.sprite->set_bg_priority(chest_bg_priority);
        item.sprite->set_z_order(-item.position.y().floor_integer());
    }

    // A short twinkle every couple of seconds, offset per chest so they don't blink together.
    int phase = (_frame + item.id * 37) % sparkle_period;

    if(opened || phase >= sparkle_frames)
    {
        item.sparkle.reset();
        return;
    }

    int frame = sparkle_first_frame + (phase >= sparkle_frames / 2 ? 1 : 0);

    if(! item.sparkle)
    {
        item.sparkle = bn::sprite_items::fx_markers.create_sprite(x + 4, y - 14, frame);
        item.sparkle->set_camera(_camera);
        item.sparkle->set_bg_priority(sparkle_bg_priority);
    }
    else
    {
        item.sparkle->set_tiles(bn::sprite_items::fx_markers.tiles_item(), frame);
    }
}

}
