#ifndef GW_HUD_H
#define GW_HUD_H

#include "bn_optional.h"
#include "bn_sprite_affine_mat_ptr.h"
#include "bn_sprite_ptr.h"
#include "bn_string.h"
#include "bn_string_view.h"
#include "bn_vector.h"

#include "gw_ui.h"

namespace gw
{

class combat;
class enemies;

// The heads-up display on the UI layer: player and target frames, experience bar, cast bar,
// error and loot messages, buffs and the action bar shown while R is held.
class hud
{

public:
    hud();

    // A short message in the middle of the screen, for errors and loot.
    void message(const bn::string_view& text, ui::color color = ui::color::WHITE);

    void update(const combat& combat_ref, const enemies& enemies_ref);

    // Hidden while menus and dialogs are open. Showing again redraws everything.
    void set_visible(bool visible);

    [[nodiscard]] bool visible() const
    {
        return _visible;
    }

    // Forces a full redraw next update (after the UI layer was used by something else).
    void invalidate();

private:
    bool _visible = true;
    bool _dirty = true;
    int _health = -1;
    int _max_health = -1;
    int _power = -1;
    int _max_power = -1;
    int _target = -2;
    int _target_health = -1;
    int _xp = -1;
    int _level = -1;
    int _cast = -1;
    int _buff_mask = -1;
    struct line
    {
        bn::string<30> text;
        ui::color color = ui::color::WHITE;
        int frames = 0;
    };

    static constexpr int message_lines = 3;
    line _messages[message_lines];   // newest last
    bool _message_dirty = false;
    bool _action_bar_shown = false;
    int _action_bar_state[7] = {};
    bn::vector<bn::sprite_ptr, 7> _icons;
    bn::vector<bn::sprite_ptr, 6> _buff_icons;
    bn::optional<bn::sprite_affine_mat_ptr> _small;

    void _draw_player();
    void _draw_target(const combat& combat_ref, const enemies& enemies_ref);
    void _draw_xp();
    void _draw_cast(const combat& combat_ref);
    void _draw_message();
    void _update_action_bar(const combat& combat_ref);
    void _update_buffs(const combat& combat_ref);
};

}

#endif
