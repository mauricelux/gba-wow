#include "bn_core.h"
#include "bn_keypad.h"
#include "bn_unique_ptr.h"

#include "gw_character.h"
#include "gw_character_creation.h"
#include "gw_game.h"
#include "gw_save.h"

int main()
{
    bn::core::init();

    if(! gw::load_game())
    {
        // The creation scene is gone before the game takes the screen.
        bn::unique_ptr<gw::character_creation> creation(new gw::character_creation());

        while(creation->update() != gw::character_creation::result::CREATED)
        {
            bn::core::update();
        }

        // The press that began the game shouldn't also talk to whoever stands at the start.
        while(bn::keypad::a_held())
        {
            bn::core::update();
        }
    }

    bn::unique_ptr<gw::game> game(new gw::game());

    while(true)
    {
        game->update();
        bn::core::update();
    }
}
