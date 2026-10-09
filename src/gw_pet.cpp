#include "gw_pet.h"

#include "bn_math.h"
#include "bn_string.h"

#include "gw_character.h"
#include "gw_combat.h"
#include "gw_enemies.h"
#include "gw_floating_text.h"
#include "gw_hud.h"
#include "gw_player.h"
#include "gw_world.h"

namespace gw
{

namespace
{
    constexpr int seconds = 60;
    constexpr int follow_distance = 18;     // behind the player
    constexpr int follow_slack = 10;        // how far from that spot it lets the player get
    constexpr int teleport_distance = 220;  // farther than this, it catches up at once
    constexpr int assist_range = 120;       // it helps against enemies this close to the player
    constexpr int bite_reach = melee_range - 8;
    constexpr int cast_range = 150;
    constexpr int cast_frames = 150;        // the Water Elemental's Frostbolt
    constexpr int freeze_radius = 40;
    constexpr int freeze_cooldown = 20 * seconds;
    constexpr int corpse_frames = 3 * seconds;
    constexpr int command_frames = 5 * seconds;
    constexpr int mend_interval = 3 * seconds;
    constexpr int miss_chance = 5;
    constexpr int crit_chance = 5;

    [[nodiscard]] pet_data& data()
    {
        return character().pet;
    }

    // What the pet fights, besides the player's target: not enemies held by crowd control.
    [[nodiscard]] bool fightable(const enemy& item)
    {
        return item.alive() && item.state == enemy_state::CHASE && ! item.controlled();
    }
}

pet::pet(const bn::camera_ptr& camera, player& player_ref, enemies& enemies_ref, floating_texts& texts,
         hud& hud_ref) :
    _camera(camera),
    _player(player_ref),
    _enemies(enemies_ref),
    _texts(texts),
    _hud(hud_ref)
{
}

int pet::_level() const
{
    return character().level;
}

bool pet::tamed() const
{
    return data().species != enemy_id::NONE;
}

bool pet::dead() const
{
    return ! elemental() && tamed() && data().health <= 0;
}

bool pet::active() const
{
    if(! _visible || ! _placed)
    {
        return false;
    }

    return elemental() || (tamed() && ! dead() && ! (data().flags & pet_flag::DISMISSED));
}

int pet::max_health() const
{
    if(elemental())
    {
        return enemy_base_health(_level()) * 70 / 100;
    }

    if(! tamed())
    {
        return 1;
    }

    // A little tougher than its wild kin, so it can hold what it bites.
    return enemy_base_health(_level()) * get_enemy_def(data().species).health_percent * 12 / 1000;
}

int pet::health() const
{
    return elemental() ? _elemental_health : bn::max(0, int(data().health));
}

void pet::_set_health(int health)
{
    health = bn::clamp(health, 0, max_health());

    if(elemental())
    {
        _elemental_health = health;
    }
    else
    {
        data().health = int16_t(health);
    }
}

int pet::height() const
{
    return _sprite ? _sprite->height() : 20;
}

void pet::place()
{
    const bn::fixed_point& feet = _player.position();
    constexpr int offsets[][2] = { { -follow_distance, 0 }, { follow_distance, 0 }, { 0, follow_distance },
                                   { 0, -follow_distance } };
    _position = feet;

    for(const auto& offset : offsets)
    {
        bn::fixed_point spot(feet.x() + offset[0], feet.y() + offset[1]);

        if(enemies::fits(spot.x(), spot.y()))
        {
            _position = spot;
            break;
        }
    }

    _placed = true;
    _target = -1;
    _moving = false;
    _command = ability_id::NONE;
}

void pet::tame(enemy_id species)
{
    _elemental_frames = 0;
    data().species = species;
    data().flags = 0;
    data().health = int16_t(max_health());
    _sprite.reset();
    place();

    bn::string<40> text = get_enemy_def(species).name;
    text += " is your pet now";
    _hud.message(text, ui::color::GREEN);
}

bool pet::call(const char** reason)
{
    if(! tamed())
    {
        *reason = "You have no pet";
        return false;
    }

    if(dead())
    {
        *reason = "Your pet is dead";
        return false;
    }

    if(data().flags & pet_flag::DISMISSED)
    {
        data().flags &= ~pet_flag::DISMISSED;
        place();
    }
    else
    {
        data().flags |= pet_flag::DISMISSED;
        _sprite.reset();
    }

    return true;
}

void pet::revive(int percent)
{
    data().flags &= ~pet_flag::DISMISSED;
    data().health = int16_t(bn::max(1, max_health() * percent / 100));
    _corpse_frames = 0;
    place();
}

bool pet::toggle_passive()
{
    data().flags ^= pet_flag::PASSIVE;
    return data().flags & pet_flag::PASSIVE;
}

void pet::mend(int total)
{
    _mend_ticks = 5;
    _mend_total = total;
    _mend_timer = mend_interval;
}

void pet::command(ability_id ability, int target, int value)
{
    _command = ability;
    _command_target = target;
    _command_value = value;
    _command_frames = command_frames;
    _target = target;
}

void pet::summon_elemental(int frames)
{
    _elemental_frames = frames;
    _elemental_health = max_health();
    _freeze_cooldown = 0;
    _attack_timer = seconds;
    _sprite.reset();
    place();
}

void pet::end_elemental()
{
    if(_elemental_frames > 0)
    {
        _elemental_frames = 0;
        _sprite.reset();
        _target = -1;
    }
}

bool pet::damage(int amount)
{
    if(! active())
    {
        return false;
    }

    _set_health(health() - amount);

    if(_sprite)
    {
        _sprite->flash();
    }

    if(health() > 0)
    {
        return false;
    }

    if(elemental())
    {
        end_elemental();
        _hud.message("Your Water Elemental is gone", ui::color::RED);
    }
    else
    {
        _corpse_frames = corpse_frames;
        _mend_ticks = 0;
        _hud.message("Your pet has died", ui::color::RED);
    }

    _target = -1;
    _command = ability_id::NONE;
    return true;
}

void pet::update(bool player_alive)
{
    _visible = player_alive;

    if(_elemental_frames > 0 && --_elemental_frames == 0)
    {
        _sprite.reset();
        _target = -1;
        _hud.message("Your Water Elemental fades", ui::color::WHITE);
    }

    if(_corpse_frames > 0 && --_corpse_frames == 0)
    {
        _sprite.reset();
    }

    if(! active())
    {
        if(_corpse_frames <= 0 || ! player_alive)
        {
            _sprite.reset();
        }
        else
        {
            _update_sprite();
        }

        return;
    }

    if(_attack_timer > 0)
    {
        --_attack_timer;
    }

    if(_freeze_cooldown > 0)
    {
        --_freeze_cooldown;
    }

    if(_command != ability_id::NONE && --_command_frames <= 0)
    {
        _command = ability_id::NONE;
    }

    // Mend Pet ticks, and out of a fight it heals by itself.
    if(_mend_ticks > 0 && --_mend_timer <= 0)
    {
        int amount = _mend_total / _mend_ticks;
        _mend_total -= amount;
        --_mend_ticks;
        _mend_timer = mend_interval;
        _set_health(health() + amount);
        _texts.show_number(bn::fixed_point(_position.x(), _position.y() - height() - 4), amount,
                           floating_texts::style::HEAL);
    }

    bool fighting = _combat->in_combat();

    if(! fighting && ++_regen_timer >= seconds)
    {
        _regen_timer = 0;
        _set_health(health() + bn::max(1, max_health() / 50));
    }

    // Too far behind (a dash, a fall back after a fight): it catches up at once.
    if(distance_squared(_position, _player.position()) > teleport_distance * teleport_distance)
    {
        place();
    }

    _target = _pick_target();

    if(_target < 0)
    {
        _follow();
    }
    else if(elemental())
    {
        // The Water Elemental stays by the player and casts from there.
        _follow();
        enemy& target = _enemies.at(_target);

        if(_attack_timer <= 0 && distance_squared(_position, target.position) <= cast_range * cast_range &&
           world::line_clear(_position.x().integer(), _position.y().integer() - 4, target.position.x().integer(),
                             target.position.y().integer() - 4))
        {
            _attack_timer = cast_frames;
            _direction = facing_towards(_position, target.position);
            _cast(_target);
        }
    }
    else
    {
        enemy& target = _enemies.at(_target);

        if(_move_towards(target.position, _speed(), bite_reach))
        {
            _direction = facing_towards(_position, target.position);
        }

        bool in_reach = distance_squared(_position, target.position) <= melee_range * melee_range;
        bool commanded = _command != ability_id::NONE && _command_target == _target;

        if(in_reach && (_attack_timer <= 0 || commanded))
        {
            _bite(_target);
        }
    }

    // Freeze: the Water Elemental holds what reaches the player.
    if(elemental() && _freeze_cooldown <= 0)
    {
        int near = _enemies.nearest(_player.position(), melee_range, -1, true);

        if(near >= 0)
        {
            _freeze_cooldown = freeze_cooldown;

            if(_sprite)
            {
                _sprite->play_attack();
            }

            _combat->pet_freeze(_player.position(), freeze_radius);
        }
    }

    _update_sprite();
}

int pet::_pick_target() const
{
    if(! elemental() && (data().flags & pet_flag::PASSIVE))
    {
        return -1;
    }

    int count = _enemies.count();

    // Kill Command and Intimidation pick the target; then what the player fights; then what fights
    // them.
    if(_command != ability_id::NONE && _command_target >= 0 && _command_target < count &&
       _enemies.at(_command_target).alive())
    {
        return _command_target;
    }

    int target = _combat->target();

    if(target >= 0 && target < count && fightable(_enemies.at(target)))
    {
        return target;
    }

    if(_target >= 0 && _target < count && fightable(_enemies.at(_target)))
    {
        return _target;
    }

    int best = -1;
    int best_distance = assist_range * assist_range + 1;

    for(int index = 0; index < count; ++index)
    {
        const enemy& item = _enemies.at(index);

        if(fightable(item))
        {
            int d = distance_squared(item.position, _player.position());

            if(d < best_distance)
            {
                best = index;
                best_distance = d;
            }
        }
    }

    return best;
}

void pet::_follow()
{
    // Its spot is behind the player, a little to the side.
    bn::fixed_point spot = _player.position();

    switch(_player.direction())
    {

    case facing::DOWN:
        spot += bn::fixed_point(follow_distance / 2, -follow_distance);
        break;

    case facing::UP:
        spot += bn::fixed_point(-follow_distance / 2, follow_distance);
        break;

    case facing::LEFT:
        spot += bn::fixed_point(follow_distance, 4);
        break;

    default:
        spot += bn::fixed_point(-follow_distance, 4);
        break;
    }

    if(! _move_towards(spot, _speed(), follow_slack) && ! _moving)
    {
        // Blocked: it waits where it is, facing the player.
        _direction = facing_towards(_position, _player.position());
    }
}

bn::fixed pet::_speed() const
{
    // A bit faster than the player, mounted or not, so it keeps up.
    return bn::fixed(2.2) * bn::max(100, _combat->speed_percent()) / 100;
}

bool pet::_move_towards(const bn::fixed_point& target, bn::fixed speed, int stop_distance)
{
    bn::fixed dx = target.x() - _position.x();
    bn::fixed dy = target.y() - _position.y();

    if(bn::abs(dx) <= stop_distance && bn::abs(dy) <= stop_distance)
    {
        _moving = false;
        return true;
    }

    bn::fixed step_x = bn::abs(dx) > 1 ? (dx > 0 ? speed : -speed) : bn::fixed(0);
    bn::fixed step_y = bn::abs(dy) > 1 ? (dy > 0 ? speed : -speed) : bn::fixed(0);

    if(step_x != 0 && step_y != 0)
    {
        step_x *= bn::fixed(0.7071);
        step_y *= bn::fixed(0.7071);
    }

    // Never past the spot.
    if(bn::abs(step_x) > bn::abs(dx))
    {
        step_x = dx;
    }

    if(bn::abs(step_y) > bn::abs(dy))
    {
        step_y = dy;
    }

    bool moved = false;

    if(step_x != 0 && enemies::fits(_position.x() + step_x, _position.y()))
    {
        _position.set_x(_position.x() + step_x);
        moved = true;
    }

    if(step_y != 0 && enemies::fits(_position.x(), _position.y() + step_y))
    {
        _position.set_y(_position.y() + step_y);
        moved = true;
    }

    _moving = moved;

    if(moved)
    {
        ++_walk_counter;
        _direction = facing_towards(bn::fixed_point(0, 0), bn::fixed_point(dx, dy));
    }

    return false;
}

void pet::_bite(int index)
{
    enemy& target = _enemies.at(index);
    const enemy_def& def = get_enemy_def(data().species);
    int base = enemy_base_damage(_level()) * def.damage_percent / 100;
    _attack_timer = def.attack_speed * 6;
    _direction = facing_towards(_position, target.position);

    // Bestial Wrath sends the pet into the same frenzy as the hunter.
    if(int haste = _combat->buff_value(buff_id::BESTIAL_WRATH))
    {
        _attack_timer = _attack_timer * 100 / (100 + haste);
    }

    if(_sprite)
    {
        _sprite->play_attack();
    }

    bool commanded = _command != ability_id::NONE && _command_target == index;
    int damage = random_chance(miss_chance) && ! commanded ? 0 : random_range(base * 9 / 10, base * 11 / 10);
    bool crit = damage > 0 && random_chance(crit_chance);

    if(commanded)
    {
        bn::fixed_point head(target.position.x(), target.position.y() - 34);

        if(_command == ability_id::KILL_COMMAND)
        {
            damage += _command_value;
            _texts.show(head, "Kill Command", floating_texts::style::INFO);
        }
        else if(_command == ability_id::INTIMIDATION && ! target.boss())
        {
            target.stun_frames = get_ability(ability_id::INTIMIDATION).duration;
            _texts.show(head, "Stunned", floating_texts::style::INFO);
        }

        _command = ability_id::NONE;
    }

    _combat->pet_hits(index, damage, crit);
}

void pet::_cast(int index)
{
    int base = ability_value(ability_id::WATER_ELEMENTAL);
    int damage = random_range(base * 9 / 10, base * 11 / 10);

    if(_sprite)
    {
        _sprite->play_attack();
    }

    _combat->pet_casts(index, damage, random_chance(crit_chance));
}

void pet::_update_sprite()
{
    look_id look = elemental() ? look_id::WATER_ELEMENTAL : get_enemy_def(data().species).look;

    if(_sprite && look != _look)
    {
        _sprite.reset();
    }

    if(! _sprite)
    {
        if(! actor_sprite::can_create(look))
        {
            return;
        }

        int scale = elemental() ? 100 : get_enemy_def(data().species).scale_percent;
        _sprite.emplace(look, _camera, bn::fixed(scale) / 100);
        _look = look;
    }

    _sprite->set_dead(dead());
    _sprite->update(_position, _direction, _moving, _walk_counter);
}

}
