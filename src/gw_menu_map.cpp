#include "gw_menu.h"

#include "bn_keypad.h"
#include "bn_string.h"

#include "bn_sprite_items_fx_map_marks.h"

#include "gw_character.h"
#include "gw_menu_layout.h"
#include "gw_minimaps.h"
#include "gw_quests.h"
#include "gw_ui.h"
#include "gw_world.h"

namespace gw
{

using namespace menu_layout;

namespace
{
    // The 128x128 picture's top-left corner on screen, so its 112x112 content fills rows 3 to 16 and
    // columns 2 to 15.
    constexpr int picture_x = 8;
    constexpr int picture_y = 16;
    constexpr int side_x = 17;          // the column right of the picture
    constexpr int side_width = 12;
    constexpr int marker_bg_priority = 0;   // above the menu panel

    enum mark_frame
    {
        PLAYER,
        PLAYER_BLINK,
        QUEST,
        TURN_IN,
        CHEST
    };

    // The picture of the map the player is on. Interiors show the map their door leads to, with the
    // player at the door.
    [[nodiscard]] int player_zone(int& x, int& y)
    {
        const map_info& map = world::map();
        x = character().x;
        y = character().y;

        for(int index = 0; index < minimap_count; ++index)
        {
            if(minimaps[index].map == map.id)
            {
                return index;
            }
        }

        if(! map.warps.empty())
        {
            const warp_def& door = map.warps[0];
            x = door.target_x;
            y = door.target_y;

            for(int index = 0; index < minimap_count; ++index)
            {
                if(minimaps[index].map == door.target)
                {
                    return index;
                }
            }
        }

        return 0;
    }

    // The name of the area covering the most of the map: the zone's name.
    [[nodiscard]] const char* zone_name(const map_info& map)
    {
        const char* result = "";
        int best = 0;

        for(const area_def& area : map.areas)
        {
            if(area.width * area.height > best)
            {
                best = area.width * area.height;
                result = area.name;
            }
        }

        return result;
    }

    [[nodiscard]] bn::sprite_ptr mark(int screen_x, int screen_y, int frame)
    {
        bn::sprite_ptr sprite = bn::sprite_items::fx_map_marks.create_sprite(
                    screen_x - 120, screen_y - 80, frame);
        sprite.set_bg_priority(marker_bg_priority);
        return sprite;
    }

    [[nodiscard]] bn::sprite_ptr world_mark(const minimap_def& minimap, int x, int y, int frame)
    {
        int px = picture_x + minimap.left + x * minimap_content / minimap.size;
        int py = picture_y + minimap.top + y * minimap_content / minimap.size;
        return mark(px, py, frame);
    }
}

void menu::_update_map()
{
    if(_map_zone < 0)
    {
        return;
    }

    if(bn::keypad::left_pressed())
    {
        _map_zone = (_map_zone + minimap_count - 1) % minimap_count;
        _dirty = true;
    }
    else if(bn::keypad::right_pressed())
    {
        _map_zone = (_map_zone + 1) % minimap_count;
        _dirty = true;
    }

    ++_map_frame;

    if(_map_player && (_map_frame & 15) == 0)
    {
        int frame = (_map_frame & 16) ? PLAYER_BLINK : PLAYER;
        _map_player->set_tiles(bn::sprite_items::fx_map_marks.tiles_item(), frame);
    }
}

void menu::_clear_map()
{
    _map_player.reset();
    _map_sprites.clear();
}

void menu::_draw_map()
{
    int player_x = 0;
    int player_y = 0;
    int here = player_zone(player_x, player_y);

    if(_map_zone < 0)
    {
        _map_zone = here;
    }

    _clear_map();

    const minimap_def& minimap = minimaps[_map_zone];
    const map_info& map = get_map(minimap.map);

    // The picture: four 64x64 frames.
    for(int index = 0; index < 4; ++index)
    {
        int x = picture_x + (index % 2) * 64 + 32 - 120;
        int y = picture_y + (index / 2) * 64 + 32 - 80;
        bn::sprite_ptr sprite = minimap.item.create_sprite(x, y, index);
        sprite.set_bg_priority(marker_bg_priority);
        sprite.set_z_order(1);
        _map_sprites.push_back(bn::move(sprite));
    }

    // Opened chests only: the others stay hidden.
    for(const chest_def& chest : map.chests)
    {
        if(chest_opened(chest.id) && ! _map_sprites.full())
        {
            _map_sprites.push_back(world_mark(minimap, chest.x, chest.y - 4, CHEST));
        }
    }

    // Quest givers and quests to turn in.
    for(const npc_def& npc : map.npcs)
    {
        quest_marker marker = npc_quest_marker(npc.npc);

        if((marker == quest_marker::AVAILABLE || marker == quest_marker::COMPLETE) && ! _map_sprites.full())
        {
            _map_sprites.push_back(world_mark(minimap, npc.x, npc.y - 4,
                                              marker == quest_marker::COMPLETE ? TURN_IN : QUEST));
        }
    }

    if(_map_zone == here)
    {
        _map_player = world_mark(minimap, player_x, player_y - 4, PLAYER);
    }

    // The side column: zone, where the player is, the legend and the chest count.
    int lines = ui::text_wrapped(side_x, content_top, side_width, 2, zone_name(map), ui::color::YELLOW, true);
    int y = content_top + lines + 1;

    if(_map_zone == here)
    {
        const area_def* area = area_at(world::map(), character().x, character().y);

        if(area && bn::string_view(area->name) != bn::string_view(zone_name(map)))
        {
            ui::text_wrapped(side_x, y, side_width, 2, area->name, ui::color::WHITE, true);
        }
    }
    else
    {
        ui::text(side_x, y, "(elsewhere)", ui::color::GRAY, true);
    }

    int legend_y = 9;
    constexpr const char* legend[] = { "You", "Quest", "Turn in", "Chest" };
    constexpr int legend_frames[] = { PLAYER, QUEST, TURN_IN, CHEST };

    for(int index = 0; index < 4; ++index)
    {
        int row = legend_y + index;
        _map_sprites.push_back(mark(side_x * 8 + 4, row * 8 + 4, legend_frames[index]));
        ui::text(side_x + 2, row, legend[index], ui::color::WHITE, true);
    }

    bn::string<16> chests = bn::to_string<4>(opened_chest_count());
    chests += "/";
    chests += bn::to_string<4>(total_chests());
    ui::text(side_x, 14, "Treasure", ui::color::YELLOW, true);
    ui::text(side_x, 15, chests, ui::color::WHITE, true);
    ui::text(side_x + chests.size() + 1, 15, "found", ui::color::GRAY, true);

    ui::text(page_x, hint_row, "<> Other maps", ui::color::WHITE, true);
}

}
