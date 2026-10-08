#include "bn_core.h"

#include "gw_game.h"
#include "gw_map_elwynn.h"

int main()
{
    bn::core::init();

    constexpr gw::point_def start = gw::map_data::elwynn::start;
    gw::game game(gw::map_id::ELWYNN, bn::fixed_point(start.x, start.y));

    while(true)
    {
        game.update();
        bn::core::update();
    }
}
