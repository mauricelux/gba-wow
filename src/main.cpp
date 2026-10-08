#include "bn_core.h"
#include "bn_unique_ptr.h"

#include "gw_character.h"
#include "gw_game.h"
#include "gw_save.h"

int main()
{
    bn::core::init();

    if(! gw::load_game())
    {
        gw::new_character(gw::race_id::HUMAN, gw::class_id::WARRIOR);
    }

    bn::unique_ptr<gw::game> game(new gw::game());

    while(true)
    {
        game->update();
        bn::core::update();
    }
}
