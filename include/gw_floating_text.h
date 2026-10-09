#ifndef GW_FLOATING_TEXT_H
#define GW_FLOATING_TEXT_H

#include "bn_camera_ptr.h"
#include "bn_fixed_point.h"
#include "bn_sprite_ptr.h"
#include "bn_sprite_text_generator.h"
#include "bn_string_view.h"
#include "bn_vector.h"

namespace gw
{

// Combat numbers and short notes that float up from a character and fade away.
class floating_texts
{

public:
    enum class style : uint8_t
    {
        DAMAGE_DEALT,   // white
        CRIT,           // yellow
        DAMAGE_TAKEN,   // red
        HEAL,           // green
        INFO,           // light blue: "Miss", "Dodge", "Immune"
        XP              // purple
    };

    explicit floating_texts(const bn::camera_ptr& camera);

    // world_position is in world pixels; the text is centered on it.
    void show(const bn::fixed_point& world_position, const bn::string_view& text, style text_style);

    void show_number(const bn::fixed_point& world_position, int value, style text_style);

    void update();

    void clear();

private:
    // Four characters per sprite: texts are cut to max_length characters.
    static constexpr int max_sprites = 6;
    static constexpr int max_length = max_sprites * 4;

    struct entry
    {
        bn::vector<bn::sprite_ptr, max_sprites> sprites;
        bn::fixed_point position;
        int frames = 0;
    };

    bn::camera_ptr _camera;
    bn::sprite_text_generator _generator;
    bn::vector<entry, 8> _entries;
};

}

#endif
