#include "gw_combat.h"

#include "bn_keypad.h"
#include "bn_math.h"
#include "bn_string.h"

#include "gw_enemies.h"
#include "gw_floating_text.h"
#include "gw_hud.h"
#include "gw_player.h"
#include "gw_talents.h"
#include "gw_world.h"

namespace gw
{

namespace
{
    constexpr int seconds = 60;
    constexpr int target_range = 120;
    constexpr int gore_interval = 9 * seconds;
    constexpr int gore_windup = 50;
    constexpr int lose_target_range = 220;
    constexpr int combat_timeout = 5 * seconds;
    constexpr int potion_cooldown_frames = 60 * seconds;
    constexpr int regen_interval = 2 * seconds;
    constexpr int dot_interval = 3 * seconds;
    constexpr int ranged_min_range = melee_range + 2;
    constexpr int auto_shot_range = 140;

    // Rage conversion from classic: how much damage is worth one point of rage at a level.
    [[nodiscard]] int rage_conversion(int level)
    {
        return (91 * level * level / 10000) + (3226 * level / 1000) + 4;
    }
}

combat::combat(player& player_ref, enemies& enemies_ref, floating_texts& texts, effects& fx, hud& hud_ref) :
    _player(player_ref),
    _enemies(enemies_ref),
    _texts(texts),
    _effects(fx),
    _hud(hud_ref)
{
    refresh_stats();
}

void combat::refresh_stats()
{
    stat_bonus bonus;

    if(_buffs[int(buff_id::BATTLE_SHOUT)])
    {
        bonus.attack_power += _buff_values[int(buff_id::BATTLE_SHOUT)];
    }

    if(_buffs[int(buff_id::FROST_ARMOR)])
    {
        bonus.armor += _buff_values[int(buff_id::FROST_ARMOR)];
    }

    if(_buffs[int(buff_id::LAST_STAND)])
    {
        bonus.health_percent += _buff_values[int(buff_id::LAST_STAND)];
    }

    if(_buffs[int(buff_id::ARCANE_POWER)])
    {
        bonus.damage_percent += _buff_values[int(buff_id::ARCANE_POWER)];
    }

    if(_buffs[int(buff_id::ASPECT_OF_THE_HAWK)])
    {
        bonus.ranged_attack_power += _buff_values[int(buff_id::ASPECT_OF_THE_HAWK)];
    }

    if(_buffs[int(buff_id::BESTIAL_WRATH)])
    {
        bonus.haste_percent += _buff_values[int(buff_id::BESTIAL_WRATH)];
    }

    _stats = compute_stats(bonus);
    character_data& data = character();
    data.health = bn::min(int(data.health), _stats.max_health);
    data.power = bn::min(int(data.power), _stats.max_power);
}

bool combat::in_combat() const
{
    return _combat_frames < combat_timeout || _enemies.any_in_combat();
}

int combat::_power() const
{
    return character().power;
}

void combat::_spend(int cost)
{
    character_data& data = character();
    data.power = bn::max(0, int(data.power) - cost);

    if(uses_mana() && cost > 0)
    {
        _since_cast = 0;
    }
}

int combat::cooldown(ability_id ability) const
{
    int own = _cooldowns[int(ability)];

    if(ability == ability_id::HEROIC_STRIKE)
    {
        return own;
    }

    return bn::max(own, _gcd);
}

bool combat::usable(ability_id ability) const
{
    if(! knows_ability(ability) || _dead)
    {
        return false;
    }

    const ability_def& def = get_ability(ability);

    if(_power() < def.cost || cooldown(ability) > 0)
    {
        return false;
    }

    if(ability == ability_id::EXECUTE)
    {
        if(! _target_valid())
        {
            return false;
        }

        const enemy& target = _enemies.at(_target);
        return target.health * 5 < target.max_health;
    }

    if(ability == ability_id::CHARGE)
    {
        return ! _enemies.any_in_combat();
    }

    return true;
}

int combat::cast_progress() const
{
    if(_cast_ability == ability_id::NONE || _cast_total == 0)
    {
        return 0;
    }

    return (_cast_total - _cast_frames) * 100 / _cast_total;
}

bool combat::_target_valid() const
{
    return _target >= 0 && _target < _enemies.count() && _enemies.at(_target).alive();
}

int combat::_target_distance() const
{
    return distance(_player.position(), _enemies.at(_target).position);
}

bn::fixed_point combat::_head(const bn::fixed_point& feet, int height) const
{
    return bn::fixed_point(feet.x(), feet.y() - height - 4);
}

void combat::clear_target()
{
    _target = -1;
    _auto_attack = false;
    _queued = ability_id::NONE;
    _effects.set_target(nullptr, true);
}

void combat::on_map_change()
{
    clear_target();
    _cast_ability = ability_id::NONE;
    _player.sprite().set_casting(false);
    _charging = false;
    _effects.clear();
    _texts.clear();
}

void combat::engage()
{
    if(_dead)
    {
        return;
    }

    if(! _target_valid())
    {
        _target = _enemies.nearest(_player.position(), target_range);
    }

    if(_target_valid())
    {
        _auto_attack = true;
        _player.face(_enemies.at(_target).position);
    }
    else
    {
        clear_target();
    }
}

void combat::_cycle_target()
{
    int current_distance = _target_valid() ? distance_squared(_player.position(), _enemies.at(_target).position) : -1;
    int best = -1;
    int best_distance = 0;
    int nearest = -1;
    int nearest_distance = 0;

    // The next enemy further away than the current target, wrapping around to the closest one.
    for(int index = 0, limit = _enemies.count(); index < limit; ++index)
    {
        const enemy& item = _enemies.at(index);

        if(index == _target || ! item.alive() || item.state == enemy_state::EVADE)
        {
            continue;
        }

        int d = distance_squared(_player.position(), item.position);

        if(d > target_range * target_range)
        {
            continue;
        }

        if(nearest < 0 || d < nearest_distance)
        {
            nearest = index;
            nearest_distance = d;
        }

        if(d >= current_distance && (best < 0 || d < best_distance))
        {
            best = index;
            best_distance = d;
        }
    }

    int chosen = best >= 0 ? best : nearest;

    if(chosen >= 0)
    {
        bool was_attacking = _auto_attack;
        clear_target();
        _target = chosen;
        _auto_attack = was_attacking;
    }
}

void combat::update(bool input_enabled)
{
    character_data& data = character();

    if(_dead)
    {
        return;
    }

    ++_combat_frames;
    ++_since_cast;

    if(_potion_cooldown > 0)
    {
        --_potion_cooldown;
    }

    if(_gcd > 0)
    {
        --_gcd;
    }

    for(int& cooldown_frames : _cooldowns)
    {
        if(cooldown_frames > 0)
        {
            --cooldown_frames;
        }
    }

    if(_target >= 0 && (_target >= _enemies.count() || ! _enemies.at(_target).alive() ||
                        _enemies.at(_target).state == enemy_state::EVADE ||
                        distance_squared(_player.position(), _enemies.at(_target).position) >
                        lose_target_range * lose_target_range))
    {
        clear_target();
    }

    if(input_enabled)
    {
        _read_input();
    }

    // Moving interrupts casting.
    if(_cast_ability != ability_id::NONE)
    {
        if(_player.moving())
        {
            _cast_ability = ability_id::NONE;
            _player.sprite().set_casting(false);
            _hud.message("Interrupted", ui::color::RED);
        }
        else
        {
            // Arcane Missiles fires one missile per second while channeling.
            if(_cast_ability == ability_id::ARCANE_MISSILES && _cast_frames % 60 == 0 && _target_valid())
            {
                _apply_ability(ability_id::ARCANE_MISSILES, _target);
            }

            if(--_cast_frames <= 0)
            {
                _finish_cast();
            }
        }
    }

    if(_charging && ! _player.dashing())
    {
        _charging = false;

        if(_charge_target >= 0 && _charge_target < _enemies.count() && _enemies.at(_charge_target).alive())
        {
            enemy& target = _enemies.at(_charge_target);
            target.stun_frames = get_ability(ability_id::CHARGE).duration;
            _enemies.aggro(_charge_target);
            data.power = bn::min(100, int(data.power) + ability_value(ability_id::CHARGE, data.level));
            _effects.spark(_head(target.position, 10));
        }
    }

    _update_auto_attack();
    _update_projectiles();
    _update_buffs();
    _update_regen();

    if(_target_valid())
    {
        const enemy& target = _enemies.at(_target);
        _effects.set_target(&target.position, true);
    }

    data.play_frames += 1;
}

void combat::_read_input()
{
    if(bn::keypad::r_held())
    {
        if(bn::keypad::a_pressed())
        {
            _use_slot(0);
        }
        else if(bn::keypad::b_pressed())
        {
            _use_slot(1);
        }
        else if(bn::keypad::l_pressed())
        {
            _use_slot(2);
        }
        else if(bn::keypad::up_pressed())
        {
            _use_slot(3);
        }
        else if(bn::keypad::right_pressed())
        {
            _use_slot(4);
        }
        else if(bn::keypad::down_pressed())
        {
            _use_slot(5);
        }
        else if(bn::keypad::left_pressed())
        {
            _use_slot(6);
        }
    }
    else if(bn::keypad::l_pressed())
    {
        _cycle_target();
    }
}

void combat::_use_slot(int slot)
{
    ability_id ability = character().action_bar[slot];

    if(ability != ability_id::NONE)
    {
        _use_ability(ability);
    }
}

bool combat::_use_ability(ability_id ability)
{
    const ability_def& def = get_ability(ability);
    character_data& data = character();

    if(_cast_ability != ability_id::NONE)
    {
        _hud.message("Already casting", ui::color::RED);
        return false;
    }

    if(cooldown(ability) > 0)
    {
        _hud.message(_gcd > 0 && _cooldowns[int(ability)] == 0 ? "Not ready yet" : "Ability not ready",
                     ui::color::RED);
        return false;
    }

    if(_power() < def.cost)
    {
        _hud.message(uses_mana() ? "Not enough mana" : "Not enough rage", ui::color::RED);
        return false;
    }

    int target = -1;

    if(def.target == ability_target::ENEMY)
    {
        if(! _target_valid())
        {
            _target = _enemies.nearest(_player.position(), def.range ? def.range : target_range);
        }

        if(! _target_valid())
        {
            _hud.message("No target", ui::color::RED);
            return false;
        }

        target = _target;
        int d = _target_distance();
        int range = def.range ? def.range : melee_range;

        if(ability == ability_id::CHARGE)
        {
            if(_enemies.any_in_combat())
            {
                _hud.message("Can't charge in combat", ui::color::RED);
                return false;
            }

            if(d < 32)
            {
                _hud.message("Too close", ui::color::RED);
                return false;
            }
        }

        if(d > range)
        {
            _hud.message("Out of range", ui::color::RED);
            return false;
        }

        const enemy& target_enemy = _enemies.at(target);

        if(def.range && ! world::line_clear(_player.position().x().integer(), _player.position().y().integer() - 8,
                                            target_enemy.position.x().integer(),
                                            target_enemy.position.y().integer() - 8))
        {
            _hud.message("Target not in line of sight", ui::color::RED);
            return false;
        }

        if(ability == ability_id::EXECUTE && target_enemy.health * 5 >= target_enemy.max_health)
        {
            _hud.message("Target health too high", ui::color::RED);
            return false;
        }

        _player.face(target_enemy.position);

        // Using an attack on an enemy also starts auto-attacking it.
        _auto_attack = true;
    }

    if(ability == ability_id::HEROIC_STRIKE)
    {
        _queued = _queued == ability ? ability_id::NONE : ability;
        return true;
    }

    if(def.cast_time)
    {
        if(_player.moving())
        {
            _hud.message("Can't do that while moving", ui::color::RED);
            return false;
        }

        _cast_ability = ability;
        _cast_frames = def.cast_time;
        _cast_total = def.cast_time;
        _gcd = global_cooldown;
        _player.sprite().set_casting(true);

        // Channeled spells pay up front; cast spells pay when they finish.
        if(ability == ability_id::ARCANE_MISSILES)
        {
            _spend(def.cost);
        }

        return true;
    }

    _spend(def.cost);
    _gcd = global_cooldown;
    _cooldowns[int(ability)] = def.cooldown;
    _apply_ability(ability, target);
    (void) data;
    return true;
}

void combat::_finish_cast()
{
    ability_id ability = _cast_ability;
    const ability_def& def = get_ability(ability);
    _cast_ability = ability_id::NONE;
    _player.sprite().set_casting(false);

    if(ability == ability_id::ARCANE_MISSILES)
    {
        return;
    }

    if(! _target_valid())
    {
        _hud.message("No target", ui::color::RED);
        return;
    }

    if(_power() < def.cost)
    {
        _hud.message("Not enough mana", ui::color::RED);
        return;
    }

    _spend(def.cost);
    _cooldowns[int(ability)] = def.cooldown;
    _apply_ability(ability, _target);
}

int combat::_roll_weapon(bool& crit) const
{
    int damage = random_range(_stats.melee_min, _stats.melee_max);
    crit = random_chance(_stats.crit);
    return damage;
}

int combat::_roll_spell(ability_id ability, bool& crit) const
{
    int level = character().level;
    int base = ability_value(ability, level);
    int damage = random_range(base * 9 / 10, base * 11 / 10) + _stats.spell_power * base / 60;
    crit = random_chance(_stats.spell_crit);
    return damage;
}

bool combat::_roll_miss(int index) const
{
    int difference = int(_enemies.at(index).level) - int(character().level);
    int chance = 5 + bn::max(0, difference) * 2;
    return random_chance(chance);
}

bool combat::damage_enemy(int index, int amount, bool crit, bool periodic)
{
    enemy& target = _enemies.at(index);

    if(target.state == enemy_state::EVADE)
    {
        _texts.show(_head(target.position, 24), "Evade", floating_texts::style::INFO);
        return false;
    }

    if(! target.alive())
    {
        return false;
    }

    if(target.marked_frames > 0)
    {
        amount += amount / 10;
    }

    amount = bn::max(1, amount * _stats.damage_percent / 100);

    if(crit && ! periodic)
    {
        amount = amount * _stats.crit_percent / 100;
    }

    _combat_frames = 0;
    int height = target.sprite ? target.sprite->height() : 24;
    _texts.show_number(_head(target.position, height), amount,
                       crit ? floating_texts::style::CRIT : floating_texts::style::DAMAGE_DEALT);

    if(_target < 0)
    {
        _target = index;
    }

    return _enemies.damage(index, amount);
}

void combat::_melee_swing(int index)
{
    enemy& target = _enemies.at(index);
    _player.sprite().play_attack();
    _swing_timer = _stats.melee_speed;
    _combat_frames = 0;

    bool heroic = _queued == ability_id::HEROIC_STRIKE && _power() >= get_ability(ability_id::HEROIC_STRIKE).cost;
    _queued = ability_id::NONE;

    if(_roll_miss(index))
    {
        _texts.show(_head(target.position, 24), "Miss", floating_texts::style::INFO);
        _enemies.aggro(index);
        return;
    }

    bool crit;
    int damage = _roll_weapon(crit);

    if(heroic)
    {
        _spend(get_ability(ability_id::HEROIC_STRIKE).cost);
        damage += ability_value(ability_id::HEROIC_STRIKE, character().level);
    }

    _effects.spark(_head(target.position, 8));

    if(! heroic)
    {
        _gain_rage(crit ? damage * 2 : damage, true);
    }

    damage_enemy(index, damage, crit);
}

void combat::_ranged_shot(int index)
{
    _ranged_timer = _stats.ranged_speed;
    _player.sprite().play_attack();
    _combat_frames = 0;

    bool crit = random_chance(_stats.crit);
    int damage = random_range(_stats.ranged_min, _stats.ranged_max);

    if(_roll_miss(index))
    {
        damage = 0;
    }

    _effects.launch(_head(_player.position(), 6), projectile_kind::ARROW,
                    projectile_hit{ index, damage, crit, ability_id::NONE });
}

void combat::_update_auto_attack()
{
    if(_swing_timer > 0)
    {
        --_swing_timer;
    }

    if(_ranged_timer > 0)
    {
        --_ranged_timer;
    }

    if(! _auto_attack || ! _target_valid() || _cast_ability != ability_id::NONE || _player.dashing())
    {
        return;
    }

    int d = _target_distance();

    if(d <= melee_range)
    {
        if(_swing_timer <= 0)
        {
            _player.face(_enemies.at(_target).position);
            _melee_swing(_target);
        }
    }
    else if(_stats.has_ranged && character().player_class == class_id::HUNTER && d >= ranged_min_range &&
            d <= auto_shot_range && ! _player.moving() && _ranged_timer <= 0)
    {
        const enemy& target = _enemies.at(_target);

        if(world::line_clear(_player.position().x().integer(), _player.position().y().integer() - 8,
                             target.position.x().integer(), target.position.y().integer() - 8))
        {
            _player.face(target.position);
            _ranged_shot(_target);
        }
    }
}

void combat::_update_projectiles()
{
    _arrived.clear();
    _effects.update([this](int index) -> const bn::fixed_point*
    {
        if(index < 0 || index >= _enemies.count() || ! _enemies.at(index).alive())
        {
            return nullptr;
        }

        return &_enemies.at(index).position;
    }, _arrived);

    for(const projectile_hit& hit : _arrived)
    {
        _apply_hit(hit);
    }
}

void combat::_apply_hit(const projectile_hit& hit)
{
    if(hit.target < 0 || hit.target >= _enemies.count() || ! _enemies.at(hit.target).alive())
    {
        return;
    }

    enemy& target = _enemies.at(hit.target);

    if(hit.damage <= 0)
    {
        _texts.show(_head(target.position, 24), "Miss", floating_texts::style::INFO);
        _enemies.aggro(hit.target);
        return;
    }

    int level = character().level;

    switch(hit.ability)
    {

    case ability_id::FIREBALL:
    case ability_id::PYROBLAST:
    {
        const ability_def& def = get_ability(hit.ability);
        int ticks = def.duration / dot_interval;
        int total = hit.ability == ability_id::PYROBLAST ? hit.damage / 2 : 2 + level / 2;

        if(ticks > 0)
        {
            target.dot_damage = bn::max(1, total / ticks);
            target.dot_ticks = ticks;
            target.dot_timer = dot_interval;
        }
        break;
    }

    case ability_id::FROSTBOLT:
        target.slow_frames = get_ability(hit.ability).duration;
        target.slow_percent = 40;
        break;

    case ability_id::SERPENT_STING:
    {
        const ability_def& def = get_ability(hit.ability);
        int ticks = def.duration / dot_interval;
        target.dot_damage = bn::max(1, ability_value(hit.ability, level) / ticks);
        target.dot_ticks = ticks;
        target.dot_timer = dot_interval;
        _enemies.aggro(hit.target);
        return;
    }

    case ability_id::CONCUSSIVE_SHOT:
        target.slow_frames = get_ability(hit.ability).duration;
        target.slow_percent = 50;
        break;

    default:
        break;
    }

    damage_enemy(hit.target, hit.damage, hit.crit);
}

void combat::_area(ability_id ability, int radius)
{
    const ability_def& def = get_ability(ability);

    for(int index = 0, limit = _enemies.count(); index < limit; ++index)
    {
        enemy& item = _enemies.at(index);

        if(! item.alive() || item.state == enemy_state::EVADE ||
           distance_squared(item.position, _player.position()) > radius * radius)
        {
            continue;
        }

        bool crit;
        int damage = ability == ability_id::THUNDER_CLAP ? ability_value(ability, character().level) :
                _roll_spell(ability, crit);
        crit = ability != ability_id::THUNDER_CLAP && crit;

        if(ability == ability_id::THUNDER_CLAP)
        {
            item.slow_frames = def.duration;
            item.slow_percent = 30;
        }
        else if(ability == ability_id::FROST_NOVA)
        {
            item.root_frames = def.duration;
        }

        damage_enemy(index, damage, crit);
    }

    if(ability == ability_id::FROST_NOVA)
    {
        _effects.circle(_player.position(), radius, 30, true);
    }
    else
    {
        _effects.burst(_head(_player.position(), 4), ability == ability_id::ARCANE_EXPLOSION ?
                           projectile_kind::ARCANE : projectile_kind::NONE);
    }
}

void combat::_apply_ability(ability_id ability, int target_index)
{
    const ability_def& def = get_ability(ability);
    character_data& data = character();
    int level = data.level;
    bn::fixed_point from = _head(_player.position(), 8);

    switch(ability)
    {

    case ability_id::BATTLE_SHOUT:
        _set_buff(buff_id::BATTLE_SHOUT, def.duration, ability_value(ability, level));
        _texts.show(_head(_player.position(), 26), "Battle Shout", floating_texts::style::INFO);
        break;

    case ability_id::CHARGE:
        _charging = true;
        _charge_target = target_index;
        _player.dash_to(_enemies.at(target_index).position, melee_range - 6);
        break;

    case ability_id::REND:
    {
        enemy& target = _enemies.at(target_index);
        int ticks = def.duration / dot_interval;
        target.dot_damage = bn::max(1, ability_value(ability, level) / ticks);
        target.dot_ticks = ticks;
        target.dot_timer = dot_interval;
        _enemies.aggro(target_index);
        _player.sprite().play_attack();
        _texts.show(_head(target.position, 24), "Rend", floating_texts::style::INFO);
        break;
    }

    case ability_id::THUNDER_CLAP:
        _player.sprite().play_attack();
        _area(ability, def.range);
        break;

    case ability_id::HAMSTRING:
    {
        enemy& target = _enemies.at(target_index);
        target.slow_frames = def.duration;
        target.slow_percent = 50;
        _player.sprite().play_attack();
        damage_enemy(target_index, ability_value(ability, level), false);
        break;
    }

    case ability_id::EXECUTE:
    {
        // Extra rage is converted into extra damage.
        int extra = _power();
        _spend(extra);
        _player.sprite().play_attack();
        _effects.spark(_head(_enemies.at(target_index).position, 8));
        damage_enemy(target_index, ability_value(ability, level) + extra * 3, random_chance(_stats.crit));
        break;
    }

    case ability_id::MORTAL_STRIKE:
    case ability_id::RAPTOR_STRIKE:
    case ability_id::COUNTERATTACK:
    {
        bool crit;
        int damage = _roll_weapon(crit) + ability_value(ability, level);
        _player.sprite().play_attack();
        _effects.spark(_head(_enemies.at(target_index).position, 8));

        if(ability == ability_id::COUNTERATTACK)
        {
            _enemies.at(target_index).root_frames = def.duration;
        }

        damage_enemy(target_index, damage, crit);
        break;
    }

    case ability_id::BLOODTHIRST:
    {
        int damage = _stats.attack_power * ability_value(ability, level) / 100;
        bool crit = random_chance(_stats.crit);
        _player.sprite().play_attack();
        _effects.spark(_head(_enemies.at(target_index).position, 8));
        damage_enemy(target_index, damage, crit);
        heal_player(_stats.max_health / 20);
        break;
    }

    case ability_id::LAST_STAND:
    {
        int before = _stats.max_health;
        _set_buff(buff_id::LAST_STAND, def.duration, ability_value(ability, level));
        heal_player(_stats.max_health - before);
        break;
    }

    case ability_id::FIREBALL:
    case ability_id::FROSTBOLT:
    case ability_id::PYROBLAST:
    case ability_id::ARCANE_MISSILES:
    case ability_id::ARCANE_SHOT:
    {
        bool crit;
        int damage = _roll_spell(ability, crit);

        if(_roll_miss(target_index))
        {
            damage = 0;
        }

        _effects.launch(from, def.projectile, projectile_hit{ target_index, damage, crit, ability });
        _player.sprite().play_attack();
        break;
    }

    case ability_id::FIRE_BLAST:
    {
        bool crit;
        int damage = _roll_spell(ability, crit);
        _effects.burst(_head(_enemies.at(target_index).position, 10), projectile_kind::FIRE);
        damage_enemy(target_index, damage, crit);
        break;
    }

    case ability_id::FROST_ARMOR:
        _set_buff(buff_id::FROST_ARMOR, def.duration, ability_value(ability, level));
        _texts.show(_head(_player.position(), 26), "Frost Armor", floating_texts::style::INFO);
        break;

    case ability_id::FROST_NOVA:
    case ability_id::ARCANE_EXPLOSION:
        _area(ability, def.range);
        break;

    case ability_id::ICE_BARRIER:
        _set_buff(buff_id::ICE_BARRIER, def.duration, ability_value(ability, level));
        _texts.show(_head(_player.position(), 26), "Ice Barrier", floating_texts::style::INFO);
        break;

    case ability_id::ARCANE_POWER:
        _set_buff(buff_id::ARCANE_POWER, def.duration, ability_value(ability, level));
        break;

    case ability_id::SERPENT_STING:
    case ability_id::CONCUSSIVE_SHOT:
    case ability_id::AIMED_SHOT:
    {
        bool crit = random_chance(_stats.crit);
        int damage = ability == ability_id::SERPENT_STING ? 1 :
                random_range(_stats.ranged_min, _stats.ranged_max) + ability_value(ability, level);

        if(_roll_miss(target_index))
        {
            damage = 0;
        }

        _effects.launch(from, projectile_kind::ARROW, projectile_hit{ target_index, damage, crit, ability });
        _player.sprite().play_attack();
        break;
    }

    case ability_id::HUNTERS_MARK:
        _enemies.at(target_index).marked_frames = def.duration;
        _enemies.aggro(target_index);
        _texts.show(_head(_enemies.at(target_index).position, 24), "Marked", floating_texts::style::INFO);
        break;

    case ability_id::ASPECT_OF_THE_HAWK:
        _set_buff(buff_id::ASPECT_OF_THE_HAWK, 0x7FFF, ability_value(ability, level));
        _texts.show(_head(_player.position(), 26), "Aspect of the Hawk", floating_texts::style::INFO);
        break;

    case ability_id::MULTI_SHOT:
    {
        // The target plus up to two enemies close to it.
        const bn::fixed_point& center = _enemies.at(target_index).position;
        int hits = 0;

        for(int index = 0, limit = _enemies.count(); index < limit && hits < 3; ++index)
        {
            const enemy& item = _enemies.at(index);

            if(item.alive() && item.state != enemy_state::EVADE &&
               (index == target_index || distance_squared(item.position, center) < 48 * 48))
            {
                int damage = random_range(_stats.ranged_min, _stats.ranged_max) + ability_value(ability, level);
                _effects.launch(from, projectile_kind::ARROW,
                                projectile_hit{ index, damage, random_chance(_stats.crit), ability });
                ++hits;
            }
        }

        _player.sprite().play_attack();
        break;
    }

    case ability_id::BESTIAL_WRATH:
        _set_buff(buff_id::BESTIAL_WRATH, def.duration, ability_value(ability, level));
        break;

    default:
        break;
    }
}

void combat::_set_buff(buff_id buff, int frames, int value)
{
    _buffs[int(buff)] = frames;
    _buff_values[int(buff)] = value;
    refresh_stats();
}

void combat::_update_buffs()
{
    bool changed = false;

    for(int index = 0; index < int(buff_id::COUNT); ++index)
    {
        int& frames = _buffs[index];

        // Aspects last until death or the next map; everything else counts down.
        if(frames > 0 && index != int(buff_id::ASPECT_OF_THE_HAWK))
        {
            if(--frames == 0)
            {
                changed = true;
            }
        }
    }

    if(changed)
    {
        refresh_stats();
    }

    // Eating and drinking stop when moving or fighting.
    if(_buffs[int(buff_id::WELL_FED)])
    {
        if(_player.moving() || _combat_frames < 60)
        {
            _buffs[int(buff_id::WELL_FED)] = 0;
        }
        else if(_buffs[int(buff_id::WELL_FED)] % 60 == 0)
        {
            character_data& data = character();
            data.health = bn::min(_stats.max_health, int(data.health) + _eat_health / 18);

            if(uses_mana())
            {
                data.power = bn::min(_stats.max_power, int(data.power) + _eat_mana / 18);
            }
        }
    }
}

void combat::start_eating(int health, int mana)
{
    // Eating while drinking (or the other way round) keeps both going.
    bool eating = _buffs[int(buff_id::WELL_FED)] > 0;
    _eat_health = health || ! eating ? health : _eat_health;
    _eat_mana = mana || ! eating ? mana : _eat_mana;
    _buffs[int(buff_id::WELL_FED)] = 18 * seconds;
}

bool combat::use_item(item_id item, const char** error)
{
    auto fail = [this, error](const char* reason)
    {
        if(error)
        {
            *error = reason;
        }
        else
        {
            _hud.message(reason, ui::color::RED);
        }

        return false;
    };

    const item_def& def = get_item(item);
    character_data& data = character();

    if(_dead || item_count(item) == 0)
    {
        return false;
    }

    if(data.level < def.level)
    {
        return fail("Your level is too low");
    }

    switch(def.type)
    {

    case item_type::POTION:
        if(_potion_cooldown > 0)
        {
            return fail("Potion not ready yet");
        }

        if(data.health >= _stats.max_health)
        {
            return fail("You are at full health");
        }

        heal_player(random_range(def.min_damage, def.max_damage));
        _potion_cooldown = potion_cooldown_frames;
        break;

    case item_type::FOOD:
    case item_type::DRINK:
        if(in_combat())
        {
            return fail("You can't eat in combat");
        }

        if(def.type == item_type::FOOD ? data.health >= _stats.max_health :
           ! uses_mana() || data.power >= _stats.max_power)
        {
            return fail(def.type == item_type::FOOD ? "You are at full health" : "You are at full mana");
        }

        if(def.type == item_type::FOOD)
        {
            start_eating(def.min_damage, 0);
        }
        else
        {
            start_eating(0, def.min_damage);
        }

        _hud.message(def.type == item_type::FOOD ? "Eating..." : "Drinking...", ui::color::GREEN);
        break;

    default:
        return false;
    }

    remove_item(item, 1);
    return true;
}

bool combat::quick_use()
{
    const character_data& data = character();
    item_type wanted;

    if(in_combat())
    {
        wanted = item_type::POTION;
    }
    else if(data.health < _stats.max_health)
    {
        wanted = item_type::FOOD;
    }
    else if(uses_mana() && data.power < _stats.max_power)
    {
        wanted = item_type::DRINK;
    }
    else
    {
        _hud.message("You are at full health", ui::color::WHITE);
        return false;
    }

    // The best one the player can use.
    item_id best = item_id::NONE;

    for(const item_stack& slot : data.bags)
    {
        const item_def& def = get_item(slot.item);

        if(slot.item != item_id::NONE && def.type == wanted && def.level <= data.level &&
           (best == item_id::NONE || def.min_damage > get_item(best).min_damage))
        {
            best = slot.item;
        }
    }

    if(best == item_id::NONE)
    {
        _hud.message(wanted == item_type::POTION ? "No healing potions" :
                     wanted == item_type::FOOD ? "No food" : "No drinks", ui::color::RED);
        return false;
    }

    return use_item(best);
}

void combat::_update_regen()
{
    if(++_regen_timer < regen_interval)
    {
        return;
    }

    _regen_timer = 0;
    character_data& data = character();
    bool fighting = in_combat();

    if(! fighting && data.health < _stats.max_health)
    {
        data.health = bn::min(_stats.max_health, int(data.health) + _stats.health_regen);
    }

    if(uses_mana())
    {
        // The five second rule: mana only comes back after not spending any for a while.
        if(_since_cast >= combat_timeout && data.power < _stats.max_power)
        {
            data.power = bn::min(_stats.max_power, int(data.power) + _stats.power_regen);
        }
    }
    else if(! fighting && data.power > 0)
    {
        data.power = bn::max(0, int(data.power) - _stats.power_regen * 2);
    }
}

void combat::_gain_rage(int damage, bool dealt)
{
    if(uses_mana())
    {
        return;
    }

    character_data& data = character();
    int conversion = rage_conversion(data.level);
    int rage = (dealt ? damage * 12 / conversion : damage * 4 / conversion) * _stats.rage_percent / 100;
    data.power = bn::min(100, int(data.power) + bn::max(1, rage));
}

void combat::enemy_attacks(int index, int percent)
{
    enemy& attacker = _enemies.at(index);

    if(_dead)
    {
        return;
    }

    _combat_frames = 0;

    // Getting hit while not targeting anything targets the attacker.
    if(! _target_valid())
    {
        _target = index;
    }

    bn::fixed_point head = _head(_player.position(), 26);

    if(random_chance(_stats.dodge))
    {
        _texts.show(head, "Dodge", floating_texts::style::INFO);
        return;
    }

    if(random_chance(5))
    {
        _texts.show(head, "Miss", floating_texts::style::INFO);
        return;
    }

    int damage = random_range(attacker.damage * 3 / 4, attacker.damage * 5 / 4) * percent / 100;

    if(random_chance(5))
    {
        damage = damage * 3 / 2;
    }

    // Armor mitigation, from classic.
    int armor = _stats.armor;
    int reduction = armor * 100 / (armor + 400 + 85 * attacker.level);
    reduction = bn::min(reduction, 75);
    damage = bn::max(1, damage * (100 - reduction) / 100);

    if(_buffs[int(buff_id::FROST_ARMOR)])
    {
        attacker.slow_frames = 5 * seconds;
        attacker.slow_percent = 30;
    }

    damage_player(damage, attacker.position);
}

void combat::damage_player(int amount, const bn::fixed_point& from)
{
    if(_dead)
    {
        return;
    }

    (void) from;
    _combat_frames = 0;
    bn::fixed_point head = _head(_player.position(), 26);

    // Ice Barrier soaks damage first.
    int& barrier = _buff_values[int(buff_id::ICE_BARRIER)];

    if(_buffs[int(buff_id::ICE_BARRIER)] && barrier > 0)
    {
        int absorbed = bn::min(barrier, amount);
        barrier -= absorbed;
        amount -= absorbed;

        if(barrier <= 0)
        {
            _buffs[int(buff_id::ICE_BARRIER)] = 0;
        }

        if(amount <= 0)
        {
            _texts.show(head, "Absorb", floating_texts::style::INFO);
            return;
        }
    }

    _gain_rage(amount, false);
    _player.sprite().flash();
    _texts.show_number(head, amount, floating_texts::style::DAMAGE_TAKEN);

    character_data& data = character();
    data.health -= amount;

    // Getting hit pushes back casting a little, like in WoW.
    if(_cast_ability != ability_id::NONE && _cast_ability != ability_id::ARCANE_MISSILES)
    {
        _cast_frames = bn::min(_cast_total, _cast_frames + 15);
    }

    if(data.health <= 0)
    {
        data.health = 0;
        _die();
    }
}

void combat::heal_player(int amount)
{
    character_data& data = character();
    int healed = bn::min(amount, _stats.max_health - int(data.health));

    if(healed > 0)
    {
        data.health += healed;
        _texts.show_number(_head(_player.position(), 26), healed, floating_texts::style::HEAL);
    }
}

void combat::_die()
{
    _dead = true;
    _cast_ability = ability_id::NONE;
    _player.sprite().set_casting(false);
    _player.sprite().set_dead(true);
    clear_target();

    for(int& frames : _buffs)
    {
        frames = 0;
    }

    refresh_stats();
    _enemies.reset_combat();
}

void combat::revive()
{
    _dead = false;
    _player.sprite().set_dead(false);
    _combat_frames = combat_timeout;
    _since_cast = combat_timeout;
    refresh_stats();

    character_data& data = character();
    data.health = _stats.max_health / 2;
    data.power = uses_mana() ? _stats.max_power / 2 : 0;
}

void combat::enemy_killed(int index)
{
    enemy& item = _enemies.at(index);

    if(index == _target)
    {
        clear_target();
    }

    if(! item.tapped)
    {
        return;
    }

    int xp = kill_xp(item.level, item.elite());

    if(item.boss())
    {
        xp *= 3;
    }

    if(xp > 0)
    {
        bn::string<16> text = "+";
        text += bn::to_string<8>(xp);
        text += " XP";
        _texts.show(_head(item.position, 34), text, floating_texts::style::XP);
        gain_xp(xp);
    }

    if(on_kill)
    {
        on_kill(callback_context, index);
    }
}

void combat::gain_xp(int amount)
{
    character_data& data = character();

    if(data.level >= max_level)
    {
        return;
    }

    data.xp += amount;

    while(data.level < max_level && data.xp >= xp_for_level(data.level))
    {
        data.xp -= xp_for_level(data.level);
        ++data.level;
        refresh_stats();
        data.health = _stats.max_health;

        if(uses_mana())
        {
            data.power = _stats.max_power;
        }

        bn::string<24> text = "Level ";
        text += bn::to_string<4>(data.level);
        text += "!";
        _texts.show(_head(_player.position(), 34), text, floating_texts::style::CRIT);

        for(int index = 1; index < ability_count; ++index)
        {
            const ability_def& def = get_ability(ability_id(index));

            if(def.player_class == data.player_class && def.level == data.level)
            {
                _hud.message("New skills at your trainer", ui::color::YELLOW);
                break;
            }
        }

        if(data.level >= first_talent_level)
        {
            _hud.message("New talent point to spend", ui::color::YELLOW);
        }

        if(on_level_up)
        {
            on_level_up(callback_context);
        }
    }

    if(data.level >= max_level)
    {
        data.xp = 0;
    }
}

bool combat::boss_update(int index)
{
    enemy& boss = _enemies.at(index);
    int health_percent = boss.health * 100 / boss.max_health;
    bool in_melee = distance_squared(boss.position, _player.position()) <= melee_range * melee_range;

    if(boss.special_timer > 0)
    {
        --boss.special_timer;
    }

    switch(boss.id)
    {

    case enemy_id::PRINCESS:
        // Gore: lowers her head for a moment, then a hit for more than double.
        if(boss.phase == 0)
        {
            boss.phase = 1;
            boss.special_timer = gore_interval;
        }
        else if(boss.phase == 1 && boss.special_timer == 0 && in_melee)
        {
            boss.phase = 2;
            boss.special_timer = gore_windup;
            _texts.show(_head(boss.position, 40), "Gore", floating_texts::style::DAMAGE_TAKEN);
        }
        else if(boss.phase == 2)
        {
            if(boss.special_timer > 0)
            {
                boss.moving = false;
                return true;
            }

            if(in_melee)
            {
                if(boss.sprite)
                {
                    boss.sprite->play_attack();
                }

                _hud.message("Princess gores you!", ui::color::RED);
                enemy_attacks(index, 250);
                boss.attack_timer = boss.def->attack_speed * 6;
            }

            boss.phase = 1;
            boss.special_timer = gore_interval;
        }
        break;

    case enemy_id::HOGGER:
        if(boss.phase == 0 && health_percent <= 35)
        {
            boss.phase = 1;
            _hud.message("Hogger goes into a frenzy!", ui::color::RED);
            _texts.show(_head(boss.position, 44), "Frenzy", floating_texts::style::DAMAGE_TAKEN);
        }

        // Frenzied: swings half again as often.
        if(boss.phase == 1 && boss.attack_timer > 0 && random_chance(50))
        {
            --boss.attack_timer;
        }
        break;

    default:
        break;
    }

    return false;
}

}
