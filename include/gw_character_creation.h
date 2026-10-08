#ifndef GW_CHARACTER_CREATION_H
#define GW_CHARACTER_CREATION_H

#include "bn_camera_ptr.h"
#include "bn_optional.h"

#include "gw_actor_sprite.h"
#include "gw_ids.h"

namespace gw
{

// Choosing a race and a class before a new game, with the character turning on a pedestal.
class character_creation
{

public:
    enum class result : uint8_t
    {
        CHOOSING,
        CREATED,    // new_character() was called with the choice
        BACK        // the player backed out of the first step
    };

    character_creation();

    result update();

private:
    enum class step : uint8_t
    {
        RACE,
        CLASS,
        CONFIRM
    };

    bn::camera_ptr _camera;
    bn::optional<actor_sprite> _preview;
    step _step = step::RACE;
    race_id _race = race_id::HUMAN;
    class_id _class = class_id::WARRIOR;
    int _frame = 0;
    bool _dirty = true;

    void _choose_class(int direction);
    void _refresh();
    void _draw();
};

}

#endif
