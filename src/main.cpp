#include "bn_camera_ptr.h"
#include "bn_core.h"
#include "bn_display.h"
#include "bn_math.h"
#include "bn_regular_bg_ptr.h"
#include "bn_sprite_text_generator.h"

#include "bn_regular_bg_items_northshire_ground.h"
#include "bn_regular_bg_items_northshire_overhead.h"

#include "common_variable_8x16_sprite_font.h"

#include "gw_player.h"
#include "gw_world.h"
#include "gw_zone_banner.h"

namespace
{
    // Background priorities: lower numbers are drawn on top. Characters use priority 2.
    constexpr int ground_priority = 3;
    constexpr int overhead_priority = 1;

    // In front of the abbey doors.
    constexpr bn::fixed_point player_start(512, 262);

    void follow(bn::camera_ptr& camera, const gw::player& player)
    {
        constexpr int max_x = (gw::world::width - bn::display::width()) / 2;
        constexpr int max_y = (gw::world::height - bn::display::height()) / 2;

        bn::fixed_point target = gw::world::to_screen_space(player.position());
        int x = bn::clamp(target.x().floor_integer(), -max_x, max_x);
        int y = bn::clamp(target.y().floor_integer(), -max_y, max_y);
        camera.set_position(x, y);
    }
}

int main()
{
    bn::core::init();

    bn::camera_ptr camera = bn::camera_ptr::create(0, 0);

    bn::regular_bg_ptr ground = bn::regular_bg_items::northshire_ground.create_bg(0, 0);
    ground.set_priority(ground_priority);
    ground.set_camera(camera);

    bn::regular_bg_ptr overhead = bn::regular_bg_items::northshire_overhead.create_bg(0, 0);
    overhead.set_priority(overhead_priority);
    overhead.set_camera(camera);

    gw::player player(player_start, camera);
    follow(camera, player);

    bn::sprite_text_generator text_generator(common::variable_8x16_sprite_font);
    gw::zone_banner banner(text_generator);
    banner.show("Northshire Abbey");

    while(true)
    {
        player.update();
        follow(camera, player);
        banner.update();
        bn::core::update();
    }
}
