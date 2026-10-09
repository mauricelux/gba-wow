#include "gw_menu.h"

#include "bn_keypad.h"
#include "bn_math.h"
#include "bn_string.h"

#include "bn_sprite_items_fx_map_marks.h"

#include "gw_character.h"
#include "gw_continents.h"
#include "gw_map_marks.h"
#include "gw_menu_layout.h"
#include "gw_minimaps.h"
#include "gw_quests.h"
#include "gw_travel.h"
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

    constexpr int zone_count = int(zone_id::COUNT);

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

    // The zone a minimap belongs to: its own, or for a dungeon the one its door leads to.
    [[nodiscard]] int minimap_zone(int minimap)
    {
        map_id map = minimaps[minimap].map;

        for(int pass = 0; pass < 2; ++pass)
        {
            for(int index = 0; index < zone_count; ++index)
            {
                if(zones[index].map == map)
                {
                    return index;
                }
            }

            const map_info& info = get_map(map);

            if(info.warps.empty())
            {
                break;
            }

            map = info.warps[0].target;
        }

        return int(zone_id::ELWYNN_FOREST);
    }

    // The minimap of a zone in the game, -1 for one that arrives later.
    [[nodiscard]] int zone_minimap(int zone)
    {
        for(int index = 0; index < minimap_count; ++index)
        {
            if(zones[zone].map != map_id::NONE && minimaps[index].map == zones[zone].map)
            {
                return index;
            }
        }

        return -1;
    }

    // The nearest zone of the same continent in a direction, preferring the ones in line with it.
    [[nodiscard]] int next_zone(int from, int dx, int dy)
    {
        const zone_def& here = zones[from];
        int result = from;
        int best = 0;

        for(int index = 0; index < zone_count; ++index)
        {
            const zone_def& zone = zones[index];
            int x = zone.x - here.x;
            int y = zone.y - here.y;
            int along = x * dx + y * dy;
            int across = bn::abs(x * dy) + bn::abs(y * dx);

            if(index != from && zone.continent == here.continent && along > 0)
            {
                int score = along + across * 2;

                if(result == from || score < best)
                {
                    result = index;
                    best = score;
                }
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

    if(_map_continent < 0)
    {
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
        else if(bn::keypad::b_pressed())
        {
            _map_pick = minimap_zone(_map_zone);
            _map_continent = zones[_map_pick].continent;
            _dirty = true;
        }
    }
    else
    {
        int pick = _map_pick;

        if(bn::keypad::left_pressed())
        {
            pick = next_zone(pick, -1, 0);
        }
        else if(bn::keypad::right_pressed())
        {
            pick = next_zone(pick, 1, 0);
        }
        else if(bn::keypad::up_pressed())
        {
            pick = next_zone(pick, 0, -1);
        }
        else if(bn::keypad::down_pressed())
        {
            pick = next_zone(pick, 0, 1);
        }
        else if(bn::keypad::select_pressed())
        {
            // The other continent, on its first zone in the game or else its first zone.
            _map_continent = (_map_continent + 1) % continent_count;
            pick = -1;

            for(int index = 0; index < zone_count; ++index)
            {
                if(zones[index].continent == _map_continent &&
                   (pick < 0 || (zone_minimap(pick) < 0 && zone_minimap(index) >= 0)))
                {
                    pick = index;
                }
            }

            _dirty = true;
        }
        else if(bn::keypad::a_pressed() && zone_minimap(_map_pick) >= 0)
        {
            _map_zone = zone_minimap(_map_pick);
            _map_continent = -1;
            _dirty = true;
        }

        if(pick != _map_pick)
        {
            _map_pick = pick;
            _dirty = true;
        }
    }

    ++_map_frame;

    if((_map_frame & 15) == 0)
    {
        bool blink = _map_frame & 16;

        const bn::sprite_tiles_item& tiles = bn::sprite_items::fx_map_marks.tiles_item();

        if(_map_player)
        {
            _map_player->set_tiles(tiles, blink ? MARK_PLAYER_BLINK : MARK_PLAYER);
        }

        if(_map_cursor)
        {
            _map_cursor->set_tiles(tiles, blink ? MARK_CURSOR_BLINK : MARK_CURSOR);
        }
    }
}

void menu::_clear_map()
{
    _map_player.reset();
    _map_cursor.reset();
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

    if(_map_continent >= 0)
    {
        _draw_continent();
        return;
    }

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
            _map_sprites.push_back(world_mark(minimap, chest.x, chest.y - 4, MARK_CHEST));
        }
    }

    // Gryphon masters whose flight path the player knows.
    for(const npc_def& npc : map.npcs)
    {
        flight_id flight = master_flight(npc.npc);

        if(flight != flight_id::COUNT && flight_known(flight) && ! _map_sprites.full())
        {
            _map_sprites.push_back(world_mark(minimap, npc.x, npc.y - 4, MARK_FLIGHT));
        }
    }

    // Quest givers and quests to turn in.
    for(const npc_def& npc : map.npcs)
    {
        quest_marker marker = npc_quest_marker(npc.npc);

        if((marker == quest_marker::AVAILABLE || marker == quest_marker::COMPLETE) && ! _map_sprites.full())
        {
            _map_sprites.push_back(world_mark(minimap, npc.x, npc.y - 4,
                                              marker == quest_marker::COMPLETE ? MARK_TURN_IN : MARK_QUEST));
        }
    }

    if(_map_zone == here)
    {
        _map_player = world_mark(minimap, player_x, player_y - 4, MARK_PLAYER);
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
    constexpr const char* legend[] = { "You", "Quest", "Turn in", "Chest", "Flight" };
    constexpr int legend_frames[] = { MARK_PLAYER, MARK_QUEST, MARK_TURN_IN, MARK_CHEST, MARK_FLIGHT };

    for(int index = 0; index < 5; ++index)
    {
        int row = legend_y + index;
        _map_sprites.push_back(mark(side_x * 8 + 4, row * 8 + 4, legend_frames[index]));
        ui::text(side_x + 2, row, legend[index], ui::color::WHITE, true);
    }

    bn::string<16> chests = bn::to_string<4>(opened_chest_count());
    chests += "/";
    chests += bn::to_string<4>(total_chests());
    ui::text(side_x, 15, "Treasure", ui::color::YELLOW, true);
    ui::text(side_x, 16, chests, ui::color::WHITE, true);
    ui::text(side_x + chests.size() + 1, 16, "found", ui::color::GRAY, true);

    ui::text(page_x, hint_row, "<> Other maps", ui::color::WHITE, true);
    ui::text_right(27, hint_row, "B World", ui::color::GRAY, true);
}

void menu::_draw_continent()
{
    int player_x = 0;
    int player_y = 0;
    int here = minimap_zone(player_zone(player_x, player_y));
    const continent_def& continent = continents[_map_continent];

    for(int index = 0; index < 4; ++index)
    {
        int x = picture_x + (index % 2) * 64 + 32 - 120;
        int y = picture_y + (index / 2) * 64 + 32 - 80;
        bn::sprite_ptr sprite = continent.item.create_sprite(x, y, index);
        sprite.set_bg_priority(marker_bg_priority);
        sprite.set_z_order(1);
        _map_sprites.push_back(bn::move(sprite));
    }

    // Every zone of the route: gold once it is in the game, gray until then.
    for(int index = 0; index < zone_count; ++index)
    {
        const zone_def& zone = zones[index];

        if(zone.continent == _map_continent && ! _map_sprites.full())
        {
            int frame = zone_minimap(index) >= 0 ? MARK_ZONE : MARK_ZONE_LATER;
            _map_sprites.push_back(mark(picture_x + zone.x, picture_y + zone.y, frame));
        }
    }

    // The flight paths the player knows, beside their zones.
    for(int index = 0; index < int(flight_id::COUNT); ++index)
    {
        const flight_def& def = get_flight(flight_id(index));

        if(def.continent == _map_continent && flight_known(flight_id(index)) && ! _map_sprites.full())
        {
            _map_sprites.push_back(mark(picture_x + def.x, picture_y + def.y, MARK_FLIGHT));
        }
    }

    const zone_def& pick = zones[_map_pick];

    if(zones[here].continent == _map_continent)
    {
        _map_player = mark(picture_x + zones[here].x, picture_y + zones[here].y, MARK_PLAYER);
        _map_player->set_z_order(-1);
    }

    _map_cursor = mark(picture_x + pick.x, picture_y + pick.y, MARK_CURSOR);
    _map_cursor->set_z_order(-2);

    // The side column: the continent, the zone under the cursor, its levels and whether it's here yet.
    int lines = ui::text_wrapped(side_x, content_top, side_width, 2, continent.name, ui::color::YELLOW, true);
    int y = content_top + lines + 1;
    y += ui::text_wrapped(side_x, y, side_width, 2, pick.name, ui::color::WHITE, true);

    bn::string<16> levels;

    if(pick.min_level == 1 && pick.max_level == max_level)
    {
        levels = "Capital";
    }
    else
    {
        levels = "Level ";
        levels += bn::to_string<4>(pick.min_level);
        levels += "-";
        levels += bn::to_string<4>(pick.max_level);
    }

    ui::text(side_x, y, levels, ui::color::GRAY, true);

    if(_map_pick == here)
    {
        ui::text(side_x, y + 1, "You are here", ui::color::GREEN, true);
    }
    else if(zone_minimap(_map_pick) < 0)
    {
        ui::text(side_x, y + 1, "Coming later", ui::color::GRAY, true);
    }

    constexpr const char* legend[] = { "You", "Zone", "Later", "Flight" };
    constexpr int legend_frames[] = { MARK_PLAYER, MARK_ZONE, MARK_ZONE_LATER, MARK_FLIGHT };

    for(int index = 0; index < 4; ++index)
    {
        int row = 13 + index;
        _map_sprites.push_back(mark(side_x * 8 + 4, row * 8 + 4, legend_frames[index]));
        ui::text(side_x + 2, row, legend[index], ui::color::WHITE, true);
    }

    if(zone_minimap(_map_pick) >= 0)
    {
        ui::text(page_x, hint_row, "A Open", ui::color::WHITE, true);
    }

    bn::string<24> other = "SEL ";
    other += continents[(_map_continent + 1) % continent_count].name;
    ui::text_right(27, hint_row, other, ui::color::WHITE, true);
}

}
