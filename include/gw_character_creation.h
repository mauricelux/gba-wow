#ifndef GW_CHARACTER_CREATION_H
#define GW_CHARACTER_CREATION_H

#include "bn_camera_ptr.h"
#include "bn_optional.h"

#include "gw_actor_sprite.h"
#include "gw_ids.h"

namespace gw
{

// Choosing a race, a class and its subclass before a new game, with the character turning on a
// pedestal.
class character_creation
{

public:
    enum class result : uint8_t
    {
        CHOOSING,
        CREATED,    // new_character() was called with the choice
        BACK        // the player backed out of the first step
    };

    // With subclass_only, the race and class come from the loaded save and only the subclass is
    // chosen, for characters from before subclasses (CREATED: choose_subclass() was called).
    explicit character_creation(bool subclass_only = false);

    result update();

private:
    enum class step : uint8_t
    {
        RACE,
        CLASS,
        SUBCLASS,
        CONFIRM
    };

    bn::camera_ptr _camera;
    bn::optional<actor_sprite> _preview;
    step _step = step::RACE;
    race_id _race = race_id::HUMAN;
    class_id _class = class_id::WARRIOR;
    int _subclass = 0;      // index in the class's subclasses
    int _frame = 0;
    bool _dirty = true;
    bool _subclass_only = false;

    void _choose_class(int direction);
    void _refresh();
    void _draw();
    void _draw_description();
};

// The subclass choice for a loaded save that has none.
class subclass_choice : public character_creation
{

public:
    subclass_choice() :
        character_creation(true)
    {
    }
};

}

#endif
