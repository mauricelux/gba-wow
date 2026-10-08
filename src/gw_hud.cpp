#include "gw_hud.h"

#include "bn_keypad.h"
#include "bn_affine_mat_attributes.h"

#include "bn_sprite_items_fx_icons.h"

#include "gw_combat.h"
#include "gw_enemies.h"
#include "gw_icons.h"

namespace gw
{

namespace
{
    constexpr int message_frames = 120;
    constexpr int message_row = 5;
    constexpr int cast_row = 13;
    constexpr int action_label_row = 18;
    constexpr int xp_row = 19;

    constexpr char slot_labels[7] = { 'A', 'B', 'L', '^', '>', 'v', '<' };

    constexpr icon_id buff_icons[] = {
        icon_id::BATTLE_SHOUT, icon_id::FROST_ARMOR, icon_id::SHIELD_BLOCK, icon_id::ICE_BARRIER,
        icon_id::ARCANE_INTELLECT, icon_id::ASPECT_HAWK, icon_id::ASPECT_HAWK, icon_id::FOOD
    };

    [[nodiscard]] int slot_x(int slot)
    {
        return 5 + slot * 3;
    }
}

hud::hud() = default;

void hud::message(const bn::string_view& text, ui::color color)
{
    _message = text.substr(0, bn::min(text.size(), 28));
    _message_color = color;
    _message_frames = message_frames;
    _message_dirty = true;
}

void hud::invalidate()
{
    _dirty = true;
}

void hud::set_visible(bool visible)
{
    if(visible == _visible)
    {
        return;
    }

    _visible = visible;

    if(visible)
    {
        _dirty = true;
    }
    else
    {
        _icons.clear();
        _buff_icons.clear();
        _action_bar_shown = false;
    }
}

void hud::update(const combat& combat_ref, const enemies& enemies_ref)
{
    if(! _visible)
    {
        return;
    }

    if(_dirty)
    {
        ui::clear();
        _health = -1;
        _power = -1;
        _target = -2;
        _xp = -1;
        _cast = -1;
        _buff_mask = -1;
        _message_dirty = true;
        _action_bar_shown = false;
        _icons.clear();
        _dirty = false;
    }

    const stats& s = combat_ref.player_stats();
    const character_data& data = character();

    if(data.health != _health || s.max_health != _max_health || data.power != _power ||
       s.max_power != _max_power || data.level != _level)
    {
        _health = data.health;
        _max_health = s.max_health;
        _power = data.power;
        _max_power = s.max_power;
        _draw_player();
    }

    _draw_target(combat_ref, enemies_ref);

    if(data.xp != _xp || data.level != _level)
    {
        _xp = data.xp;
        _level = data.level;
        _draw_xp();
        _draw_player();
    }

    _draw_cast(combat_ref);

    if(_message_frames > 0 && --_message_frames == 0)
    {
        _message.clear();
        _message_dirty = true;
    }

    if(_message_dirty)
    {
        _draw_message();
        _message_dirty = false;
    }

    _update_action_bar(combat_ref);
    _update_buffs(combat_ref);
}

void hud::_draw_player()
{
    ui::clear_rect(0, 0, 14, 2);
    ui::bar(0, 0, 10, _health, _max_health, ui::bar_color::HEALTH);
    ui::bar(0, 1, 10, _power, _max_power, uses_mana() ? ui::bar_color::MANA : ui::bar_color::RAGE);

    bn::string<12> text = bn::to_string<6>(_health);
    ui::text(10, 0, text, ui::color::WHITE);
    text = bn::to_string<6>(_power);
    ui::text(10, 1, text, uses_mana() ? ui::color::BLUE : ui::color::RED);
}

void hud::_draw_target(const combat& combat_ref, const enemies& enemies_ref)
{
    int target = combat_ref.target();
    int health = target >= 0 ? enemies_ref.at(target).health : -1;

    if(target == _target && health == _target_health)
    {
        return;
    }

    bool redraw_name = target != _target;
    _target = target;
    _target_health = health;

    if(target < 0)
    {
        ui::clear_rect(14, 0, 16, 2);
        return;
    }

    const enemy& item = enemies_ref.at(target);

    if(redraw_name)
    {
        ui::clear_rect(14, 0, 16, 2);

        int difference = int(item.level) - int(character().level);
        ui::color name_color = is_gray(item.level) ? ui::color::GRAY : difference >= 3 ? ui::color::RED :
                difference >= -2 ? ui::color::YELLOW : ui::color::GREEN;
        ui::text_right(29, 0, item.def->name, name_color);

        bn::string<8> level_text = item.boss() ? bn::string<8>("??") : bn::to_string<8>(int(item.level));

        if(item.elite())
        {
            level_text += "+";
        }

        ui::text_right(19, 1, level_text, name_color);
    }

    ui::bar(20, 1, 10, item.health, item.max_health, ui::bar_color::RAGE);
}

void hud::_draw_xp()
{
    if(character().level >= max_level)
    {
        ui::clear_rect(0, xp_row, ui::columns, 1);
        return;
    }

    ui::bar(0, xp_row, ui::columns, character().xp, xp_for_level(character().level), ui::bar_color::XP);
}

void hud::_draw_cast(const combat& combat_ref)
{
    ability_id casting = combat_ref.casting();
    int progress = casting == ability_id::NONE ? -1 : combat_ref.cast_progress();

    if(progress == _cast)
    {
        return;
    }

    bool started = _cast < 0 && progress >= 0;
    _cast = progress;

    if(progress < 0)
    {
        ui::clear_rect(8, cast_row, 14, 2);
        return;
    }

    if(started)
    {
        ui::clear_rect(8, cast_row, 14, 2);
        ui::text_center(cast_row, get_ability(casting).name, ui::color::YELLOW);
    }

    ui::bar(9, cast_row + 1, 12, progress, 100, ui::bar_color::CAST);
}

void hud::_draw_message()
{
    ui::clear_rect(0, message_row, ui::columns, 1);

    if(! _message.empty())
    {
        ui::text_center(message_row, _message, _message_color);
    }
}

void hud::_update_action_bar(const combat& combat_ref)
{
    bool show = bn::keypad::r_held() && ! combat_ref.dead();
    const character_data& data = character();

    if(! show)
    {
        if(_action_bar_shown)
        {
            _icons.clear();
            ui::clear_rect(0, action_label_row, ui::columns, 1);
            _action_bar_shown = false;
        }

        return;
    }

    if(! _action_bar_shown)
    {
        _icons.clear();

        for(int slot = 0; slot < action_slots; ++slot)
        {
            ability_id ability = data.action_bar[slot];
            int frame = ability == ability_id::NONE ? -1 : int(get_ability(ability).icon);

            if(frame >= 0)
            {
                bn::sprite_ptr icon = bn::sprite_items::fx_icons.create_sprite(
                            slot_x(slot) * 8 + 8 - 120, 136 - 80, frame);
                icon.set_bg_priority(0);
                _icons.push_back(bn::move(icon));
            }

            _action_bar_state[slot] = -1;
        }

        _action_bar_shown = true;
    }

    int icon_index = 0;

    for(int slot = 0; slot < action_slots; ++slot)
    {
        ability_id ability = data.action_bar[slot];

        if(ability == ability_id::NONE)
        {
            continue;
        }

        int cooldown = combat_ref.cooldown(ability);
        int seconds_left = (cooldown + 59) / 60;
        bool usable = combat_ref.usable(ability);
        int state = (usable ? 1000 : 0) + (cooldown > global_cooldown ? seconds_left : 0);

        if(state != _action_bar_state[slot])
        {
            _action_bar_state[slot] = state;
            bn::sprite_ptr& icon = _icons[icon_index];
            icon.set_palette(usable ? bn::sprite_items::fx_icons.palette_item() : palettes::icons_gray);

            ui::clear_rect(slot_x(slot), action_label_row, 2, 1);

            if(cooldown > global_cooldown)
            {
                ui::text(slot_x(slot), action_label_row, bn::to_string<4>(seconds_left), ui::color::RED);
            }
            else
            {
                char label[2] = { slot_labels[slot], 0 };
                ui::text(slot_x(slot), action_label_row, label, ui::color::YELLOW);
            }
        }

        ++icon_index;
    }
}

void hud::_update_buffs(const combat& combat_ref)
{
    int mask = 0;

    for(int index = 0; index < int(buff_id::COUNT); ++index)
    {
        if(combat_ref.buff_frames(buff_id(index)) > 0)
        {
            mask |= 1 << index;
        }
    }

    if(mask == _buff_mask)
    {
        return;
    }

    _buff_mask = mask;
    _buff_icons.clear();

    if(! _small)
    {
        bn::affine_mat_attributes attributes;
        attributes.set_scale(0.5);
        _small = bn::sprite_affine_mat_ptr::create(attributes);
    }

    int x = 0;

    for(int index = 0; index < int(buff_id::COUNT) && ! _buff_icons.full(); ++index)
    {
        if(mask & (1 << index))
        {
            bn::sprite_ptr icon = bn::sprite_items::fx_icons.create_sprite(-120 + 4 + x * 10, -80 + 20,
                                                                           int(buff_icons[index]));
            icon.set_bg_priority(0);
            icon.set_affine_mat(*_small);
            _buff_icons.push_back(bn::move(icon));
            ++x;
        }
    }
}

}
