#ifndef GW_TITLE_SCREEN_H
#define GW_TITLE_SCREEN_H

#include "bn_camera_ptr.h"
#include "bn_fixed.h"
#include "bn_regular_bg_ptr.h"
#include "bn_sprite_ptr.h"
#include "bn_vector.h"

namespace gw
{

// The first screen: the logo over Elwynn Forest, with Continue (when there is a save) and New Game.
class title_screen
{

public:
    enum class result : uint8_t
    {
        WAITING,
        CONTINUE,   // the save is loaded
        NEW_GAME
    };

    title_screen();

    result update();

private:
    enum class option : uint8_t
    {
        CONTINUE,
        NEW_GAME
    };

    bn::camera_ptr _camera;
    bn::regular_bg_ptr _ground;
    bn::regular_bg_ptr _overhead;
    bn::vector<bn::sprite_ptr, 6> _logo;
    bn::fixed _pan;
    int _pan_direction = 1;
    int _frame = 0;
    int _fade_frames = 0;
    bool _has_save = false;
    bool _confirming = false;
    bool _dirty = true;
    option _option = option::NEW_GAME;
    result _chosen = result::WAITING;

    void _choose(result chosen);
    void _draw();
};

}

#endif
