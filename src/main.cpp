#include "bn_core.h"
#include "bn_keypad.h"
#include "bn_unique_ptr.h"

#include "gw_character.h"
#include "gw_character_creation.h"
#include "gw_fade.h"
#include "gw_game.h"
#include "gw_save.h"
#include "gw_title_screen.h"

namespace
{
    // Each scene is gone before the next one takes the screen.
    template<typename Scene>
    [[nodiscard]] typename Scene::result run_scene(typename Scene::result waiting)
    {
        bn::unique_ptr<Scene> scene(new Scene());
        typename Scene::result result = waiting;

        while((result = scene->update()) == waiting)
        {
            bn::core::update();
        }

        return result;
    }
}

int main()
{
    bn::core::init();

    while(true)
    {
        if(run_scene<gw::title_screen>(gw::title_screen::result::WAITING) == gw::title_screen::result::CONTINUE)
        {
            break;
        }

        gw::set_fade(0);

        if(run_scene<gw::character_creation>(gw::character_creation::result::CHOOSING) ==
           gw::character_creation::result::CREATED)
        {
            break;
        }

        // Backed out: the title screen loads the save again.
    }

    gw::set_fade(0);

    // The press that began the game shouldn't also talk to whoever stands at the start.
    while(bn::keypad::a_held())
    {
        bn::core::update();
    }

    bn::unique_ptr<gw::game> game(new gw::game());

    while(true)
    {
        game->update();
        bn::core::update();
    }
}
