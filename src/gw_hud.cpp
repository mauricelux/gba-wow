#include "gw_hud.h"

#include "bn_affine_mat_attributes.h"
#include "bn_math.h"

#include "bn_sprite_items_fx_icons.h"
#include "bn_sprite_items_fx_petbar.h"

#include "gw_combat.h"
#include "gw_enemies.h"
#include "gw_icons.h"
#include "gw_pet.h"
#include "gw_sprite_palettes.h"

namespace gw
{

namespace
{
    constexpr int message_frames = 150;
    constexpr int message_row = 3;      // the first of the message lines
    constexpr int target_cast_row = 2;  // what the target casts, under its frame
    constexpr int cast_row = 13;
    constexpr int xp_row = 19;

    // The pet's health: a 32x8 sprite under the player's bars, 17 fills and a gray one when dead. The
    // buff reminder and the buffs move over to make room for it.
    constexpr int pet_bar_x = -120 + 16;
    constexpr int pet_bar_y = -80 + 20;
    constexpr int pet_row_width = 34;
    constexpr int pet_bar_steps = 16;
    constexpr int pet_bar_dead = pet_bar_steps + 1;

    constexpr char slot_labels[action_slots] = { 'A', 'B', 'L', '^', '>', 'v', '<' };

    // Where each slot's icon sits (its top left cell): A and B to the right like the buttons, L at
    // the top left, and the D-pad slots around a small cross at the bottom left. A label (key or
    // cooldown) goes over the bottom row of the icon.
    struct cell
    {
        int8_t x;
        int8_t y;
    };

    constexpr cell slot_cells[action_slots] = { { 26, 13 }, { 23, 15 }, { 0, 10 }, { 3, 11 }, { 5, 13 },
                                                { 3, 15 }, { 1, 13 } };
    constexpr cell cross_cell = { 3, 13 };
    constexpr int bar_name_row = 17;

    // The parts of the screen the bars use, to clear.
    constexpr int left_area_x = 0;
    constexpr int left_area_y = 10;
    constexpr int left_area_width = 8;
    constexpr int left_area_height = 8;
    constexpr int right_area_x = 22;
    constexpr int right_area_y = 13;
    constexpr int right_area_width = 7;
    constexpr int right_area_height = 4;

    // By held_bar.
    constexpr const char* bar_names[] = { "", "Combat", "Utility", "Buffs", "Items" };

    // The target frame's name row, right of the player's numbers.
    constexpr int target_name_width = 16;

    // A name too long for the target frame loses its title ("Targorr the Dread" becomes "Targorr") or
    // keeps the first word's initial ("Blackrock Shadowcaster" becomes "B. Shadowcaster").
    [[nodiscard]] bn::string<32> frame_name(const bn::string_view& name)
    {
        if(name.size() <= target_name_width)
        {
            return bn::string<32>(name);
        }

        int space = 0;

        while(space < name.size() && name[space] != ' ')
        {
            ++space;
        }

        if(space + 1 >= name.size())
        {
            return bn::string<32>(name.substr(0, target_name_width));
        }

        char next = name[space + 1];

        if(next >= 'a' && next <= 'z')
        {
            return bn::string<32>(name.substr(0, space));
        }

        bn::string<32> result;
        result.push_back(name[0]);
        result.append(". ");
        bn::string_view rest = name.substr(space + 1);
        result.append(rest.substr(0, bn::min(rest.size(), target_name_width - 3)));
        return result;
    }

    // Items bar slots go on the D-pad slots: up, right, down, left.
    constexpr int first_item_slot = 3;

    [[nodiscard]] bn::fixed_point cell_position(cell at)
    {
        return bn::fixed_point(at.x * 8 + 8 - 120, at.y * 8 + 8 - 80);
    }

    [[nodiscard]] icon_id item_icon(item_type type)
    {
        return type == item_type::POTION ? icon_id::POTION : type == item_type::FOOD ? icon_id::FOOD :
               type == item_type::DRINK ? icon_id::DRINK : icon_id::HEARTHSTONE;
    }

    // The type an Items bar slot uses, set or by default.
    [[nodiscard]] item_type item_slot_type(int slot)
    {
        item_id set = character().item_bar[slot];
        return set != item_id::NONE ? get_item(set).type : item_bar_default(slot);
    }

    // By buff_id.
    constexpr icon_id buff_icons[] = {
        icon_id::BATTLE_SHOUT, icon_id::FROST_ARMOR, icon_id::LAST_STAND, icon_id::ICE_BARRIER,
        icon_id::ARCANE_POWER, icon_id::ASPECT_HAWK, icon_id::BESTIAL_WRATH, icon_id::FOOD,
        icon_id::ARCANE_INTELLECT, icon_id::MOLTEN_ARMOR, icon_id::MAGE_ARMOR, icon_id::FIRE_WARD,
        icon_id::MANA_SHIELD, icon_id::ARCANE_BLAST, icon_id::PRESENCE_OF_MIND, icon_id::COMBUSTION,
        icon_id::ICE_BLOCK, icon_id::RETALIATION, icon_id::SWEEPING_STRIKES, icon_id::WHIRLING_BLADES,
        icon_id::BERSERKER_RAGE, icon_id::RECKLESSNESS, icon_id::DEATH_WISH, icon_id::SHIELD_BLOCK,
        icon_id::SHIELD_WALL, icon_id::ASPECT_MONKEY, icon_id::ASPECT_CHEETAH, icon_id::RAPID_FIRE,
        icon_id::DETERRENCE, icon_id::TRUESHOT_AURA, icon_id::CONCUSSIVE_SHOT, icon_id::FROSTBOLT, icon_id::NET,
        icon_id::STUN, icon_id::SLEEP, icon_id::POLYMORPH, icon_id::INTIMIDATING_SHOUT, icon_id::CURSE,
        icon_id::SUNDER_ARMOR, icon_id::MORTAL_STRIKE, icon_id::REND, icon_id::POISON, icon_id::DISEASE,
        icon_id::BURNING, icon_id::MOUNT, icon_id::ASPECT_BEAST, icon_id::WATER_ELEMENTAL
    };

    static_assert(sizeof(buff_icons) / sizeof(buff_icons[0]) == int(buff_id::COUNT), "an icon per buff");
}

hud::hud() :
    _icons_palette(bn::sprite_items::fx_icons.palette_item().create_palette())
{
}

void hud::message(const bn::string_view& text, ui::color color)
{
    // The same message again just stays longer.
    line& last = _messages[message_lines - 1];

    if(last.frames > 0 && last.text == text.substr(0, bn::min(text.size(), 30)))
    {
        last.frames = message_frames;
        return;
    }

    for(int index = 0; index < message_lines - 1; ++index)
    {
        _messages[index] = _messages[index + 1];
    }

    last.text = text.substr(0, bn::min(text.size(), 30));
    last.color = color;
    last.frames = message_frames;
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
        _debuff_icons.clear();
        _reminder.reset();
        _pet_bar.reset();
        _pet_frame = -1;
        _bar_shown = 0;
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
        _target_cast = -1;
        _buff_mask = ~uint64_t(0);
        _message_dirty = true;
        _bar_shown = 0;
        _icons.clear();
        _reminder_ability = -1;
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
    _draw_target_cast(combat_ref, enemies_ref);

    if(data.xp != _xp || data.level != _level || (data.rest_xp > 0) != _rested)
    {
        _xp = data.xp;
        _rested = data.rest_xp > 0;
        _level = data.level;
        _draw_xp();
        _draw_player();
    }

    _draw_cast(combat_ref);

    for(line& message : _messages)
    {
        if(message.frames > 0 && --message.frames == 0)
        {
            message.text.clear();
            _message_dirty = true;
        }
    }

    if(_message_dirty)
    {
        _draw_message();
        _message_dirty = false;
    }

    ++_frame;
    _update_bar(combat_ref);
    _update_pet(combat_ref);
    _update_reminder(combat_ref);
    _update_buffs(combat_ref);
}

void hud::_update_pet(const combat& combat_ref)
{
    const pet* companion = combat_ref.companion();
    int frame = -1;

    if(companion && companion->dead())
    {
        frame = pet_bar_dead;
    }
    else if(companion && companion->active())
    {
        int max = companion->max_health();
        frame = (companion->health() * pet_bar_steps + max - 1) / max;
    }

    // Without a sprite the bar is tried again until there is room for its palette.
    if(frame == _pet_frame && (frame < 0 || _pet_bar))
    {
        return;
    }

    if((frame < 0) != (_pet_frame < 0))
    {
        _reminder_ability = -1;
        _buff_mask = ~uint64_t(0);
    }

    _pet_frame = frame;

    if(frame < 0)
    {
        _pet_bar.reset();
    }
    else if(_pet_bar)
    {
        _pet_bar->set_tiles(bn::sprite_items::fx_petbar.tiles_item(), frame);
    }
    else if(sprite_palettes::fits(bn::sprite_items::fx_petbar.palette_item()))
    {
        _pet_bar = bn::sprite_items::fx_petbar.create_sprite(pet_bar_x, pet_bar_y, frame);
        _pet_bar->set_bg_priority(0);
    }
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
        ui::text_right(29, 0, frame_name(item.def->name), name_color);

        bn::string<8> level_text = item.boss() ? bn::string<8>("??") : item.rare() ? bn::string<8>("Rare") :
                                                                                bn::to_string<8>(int(item.level));

        if(item.elite())
        {
            level_text += "+";
        }

        ui::text_right(19, 1, level_text, name_color);
    }

    ui::bar(20, 1, 10, item.health, item.max_health, ui::bar_color::RAGE);
}

void hud::_draw_target_cast(const combat& combat_ref, const enemies& enemies_ref)
{
    // The name of what the target casts, so there's time to interrupt it.
    int target = combat_ref.target();
    int casting = target >= 0 ? int(enemies_ref.at(target).ai.casting) : 0;

    if(casting == _target_cast)
    {
        return;
    }

    _target_cast = casting;
    ui::clear_rect(14, target_cast_row, 16, 1);

    if(casting)
    {
        const enemy_ability_def& def = get_enemy_ability(enemy_ability_id(casting));
        bool interruptible = def.flags & enemy_ability_flag::INTERRUPTIBLE;
        ui::text_right(29, target_cast_row, def.name, interruptible ? ui::color::YELLOW : ui::color::GRAY);
    }
}

void hud::_draw_xp()
{
    if(character().level >= max_level)
    {
        ui::clear_rect(0, xp_row, ui::columns, 1);
        return;
    }

    // Blue while rested, as in WoW.
    ui::bar(0, xp_row, ui::columns, character().xp, xp_for_level(character().level),
            character().rest_xp > 0 ? ui::bar_color::MANA : ui::bar_color::XP);
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
    ui::clear_rect(0, message_row, ui::columns, message_lines);

    // Messages stack upwards from the bottom line, newest at the bottom.
    int row = message_row + message_lines - 1;

    for(int index = message_lines - 1; index >= 0; --index)
    {
        const line& message = _messages[index];

        if(! message.text.empty())
        {
            ui::text_center(row, message.text, message.color);
            --row;
        }
    }
}

void hud::_update_bar(const combat& combat_ref)
{
    held_bar bar = combat_ref.dead() ? held_bar::NONE : combat_ref.shown_bar();
    const character_data& data = character();
    bool items = bar == held_bar::ITEMS;
    const ability_id* abilities = bar == held_bar::UTILITY ? data.action_bars[int(bar_id::UTILITY)] :
                                  bar == held_bar::BUFFS ? data.action_bars[int(bar_id::BUFFS)] :
                                                           data.action_bars[int(bar_id::COMBAT)];

    if(int(bar) != _bar_shown)
    {
        _icons.clear();
        ui::clear_rect(left_area_x, left_area_y, left_area_width, left_area_height);
        ui::clear_rect(right_area_x, right_area_y, right_area_width, right_area_height);
        _bar_shown = int(bar);

        if(bar == held_bar::NONE)
        {
            return;
        }

        auto add_icon = [this](cell at, icon_id icon)
        {
            bn::sprite_ptr sprite = bn::sprite_items::fx_icons.create_sprite(cell_position(at), int(icon));

            // Under the UI layer, so labels show on top.
            sprite.set_bg_priority(1);
            _icons.push_back(bn::move(sprite));
        };

        add_icon(cross_cell, icon_id::DPAD);
        ui::text(left_area_x, bar_name_row, bar_names[int(bar)], ui::color::YELLOW);

        for(int slot = 0; slot < action_slots; ++slot)
        {
            _slot_icons[slot] = -1;
            _bar_state[slot] = -1;

            if(items)
            {
                if(slot >= first_item_slot)
                {
                    _slot_icons[slot] = _icons.size();
                    add_icon(slot_cells[slot], item_icon(item_slot_type(slot - first_item_slot)));
                }
            }
            else if(abilities[slot] != ability_id::NONE)
            {
                _slot_icons[slot] = _icons.size();
                add_icon(slot_cells[slot], get_ability(abilities[slot]).icon);
            }
        }
    }

    if(bar == held_bar::NONE)
    {
        return;
    }

    for(int slot = 0; slot < action_slots; ++slot)
    {
        int icon_index = _slot_icons[slot];

        if(icon_index < 0)
        {
            continue;
        }

        // What the label says: a cooldown in seconds (minutes from 100), an item count, or the key.
        bool usable;
        int seconds_left = 0;
        int count = -1;

        if(items)
        {
            int item_slot = slot - first_item_slot;
            item_id item = item_bar_item(item_slot);
            item_type type = item_slot_type(item_slot);
            count = item != item_id::NONE ? item_count(item) : 0;

            if(type == item_type::HEARTHSTONE)
            {
                int frames = int(data.hearthstone_ready) - int(data.play_frames);
                seconds_left = frames > 0 ? (frames + 59) / 60 : 0;
                count = -1;
            }
            else if(type == item_type::POTION)
            {
                seconds_left = (combat_ref.potion_cooldown() + 59) / 60;
            }

            usable = item != item_id::NONE && seconds_left == 0;
        }
        else
        {
            ability_id ability = abilities[slot];
            int cooldown = combat_ref.cooldown(ability);
            seconds_left = cooldown > global_cooldown ? (cooldown + 59) / 60 : 0;
            usable = combat_ref.usable(ability);
        }

        int state = (usable ? 1 << 20 : 0) + (seconds_left << 10) + (count + 1);

        if(state == _bar_state[slot])
        {
            continue;
        }

        _bar_state[slot] = state;
        // Without room for the gray palette an icon that can't be used yet keeps its colors; its cooldown
        // still counts down under it.
        if(usable || sprite_palettes::fits(palettes::icons_gray))
        {
            _icons[icon_index].set_palette(usable ? bn::sprite_items::fx_icons.palette_item() : palettes::icons_gray);
        }

        cell at = slot_cells[slot];
        int label_y = at.y + 1;
        ui::clear_rect(bn::max(0, at.x - 1), label_y, at.x == 0 ? 2 : 3, 1);

        if(seconds_left > 0)
        {
            // Two characters: seconds, or minutes for long cooldowns.
            bn::string<4> left = seconds_left < 100 ? bn::to_string<4>(seconds_left) :
                                                      bn::to_string<4>((seconds_left + 59) / 60);

            if(seconds_left >= 100)
            {
                left += "m";
            }

            ui::text_right(at.x + 1, label_y, left, ui::color::RED);
        }
        else if(count >= 0)
        {
            ui::text_right(at.x + 1, label_y, bn::to_string<4>(count), count ? ui::color::WHITE : ui::color::RED);
        }
        else if(slot < first_item_slot)
        {
            char label[2] = { slot_labels[slot], 0 };
            ui::text(at.x + 1, label_y, label, ui::color::YELLOW);
        }
    }
}

void hud::_update_reminder(const combat& combat_ref)
{
    // Blinks the icon of a missing long buff beside the player frame, while no bar is held.
    ability_id missing = combat_ref.shown_bar() == held_bar::NONE ? combat_ref.missing_buff() : ability_id::NONE;

    if(int(missing) != _reminder_ability)
    {
        _reminder_ability = int(missing);
        _reminder.reset();
        _buff_mask = ~uint64_t(0);  // the buffs move over to make room

        if(missing != ability_id::NONE)
        {
            if(! _small)
            {
                bn::affine_mat_attributes attributes;
                attributes.set_scale(0.5);
                _small = bn::sprite_affine_mat_ptr::create(attributes);
            }

            _reminder = bn::sprite_items::fx_icons.create_sprite(-120 + 4 + (_pet_frame >= 0 ? pet_row_width : 0),
                                                                 -80 + 20, int(get_ability(missing).icon));
            _reminder->set_bg_priority(0);
            _reminder->set_affine_mat(*_small);
        }
    }

    if(_reminder)
    {
        _reminder->set_visible((_frame / 30) % 2 == 0);
    }
}

void hud::_update_buffs(const combat& combat_ref)
{
    uint64_t mask = 0;

    for(int index = 0; index < int(buff_id::COUNT); ++index)
    {
        if(combat_ref.buff_frames(buff_id(index)) > 0)
        {
            mask |= uint64_t(1) << index;
        }
    }

    if(mask == _buff_mask)
    {
        return;
    }

    _buff_mask = mask;
    _buff_icons.clear();
    _debuff_icons.clear();

    if(! _small)
    {
        bn::affine_mat_attributes attributes;
        attributes.set_scale(0.5);
        _small = bn::sprite_affine_mat_ptr::create(attributes);
    }

    // Buffs in a row beside the reminder, debuffs in a row under them.
    int buff_x = _reminder ? 1 : 0;
    int debuff_x = 0;
    int row_x = _pet_frame >= 0 ? pet_row_width : 0;

    for(int index = 0; index < int(buff_id::COUNT); ++index)
    {
        if(! (mask & (uint64_t(1) << index)))
        {
            continue;
        }

        bool debuff = is_debuff(buff_id(index));
        bn::ivector<bn::sprite_ptr>& icons = debuff ? static_cast<bn::ivector<bn::sprite_ptr>&>(_debuff_icons) :
                                                      static_cast<bn::ivector<bn::sprite_ptr>&>(_buff_icons);

        if(icons.full())
        {
            continue;
        }

        int& x = debuff ? debuff_x : buff_x;
        int icon_x = -120 + 4 + x * 10 + (debuff ? 0 : row_x);
        bn::sprite_ptr icon = bn::sprite_items::fx_icons.create_sprite(icon_x, debuff ? -52 : -60,
                                                                       int(buff_icons[index]));
        icon.set_bg_priority(0);
        icon.set_affine_mat(*_small);
        icons.push_back(bn::move(icon));
        ++x;
    }
}

}
