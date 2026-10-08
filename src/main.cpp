#include "bn_core.h"
#include "bn_unique_ptr.h"

#include "gw_character.h"
#include "gw_game.h"

int main()
{
    bn::core::init();

    gw::new_character(gw::race_id::HUMAN, gw::class_id::WARRIOR);
    bn::unique_ptr<gw::game> game(new gw::game());

    while(true)
    {
        game->update();
        bn::core::update();
    }
}
