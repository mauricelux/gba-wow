#include "gw_combat.h"

#include "bn_keypad.h"
#include "bn_math.h"
#include "bn_string.h"

#include "gw_audio.h"
#include "gw_enemies.h"
#include "gw_floating_text.h"
#include "gw_homes.h"
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
    constexpr int out_of_reach_switch_frames = 60;
    constexpr int telegraph_interval = 7 * seconds;
    constexpr int telegraph_windup = 90;
    constexpr int saw_radius = 28;
    constexpr int flurry_radius = 40;
    constexpr int smoke_radius = 32;
    constexpr int lose_target_range = 220;
    constexpr int combat_timeout = 5 * seconds;
    constexpr int potion_cooldown_frames = 60 * seconds;
    constexpr int regen_interval = 2 * seconds;
    constexpr int dot_interval = 3 * seconds;
    constexpr int ranged_min_range = melee_range + 2;
    constexpr int auto_shot_range = 140;
    constexpr int reactive_frames = 5 * seconds;     // Overpower, Revenge, Mongoose Bite after a dodge
    constexpr int ground_radius = 32;                // Flamestrike, Blizzard, Volley, exploding traps
    constexpr int trap_radius = 16;
    constexpr int trap_frames = 60 * seconds;
    constexpr int trap_arm_frames = 30;
    constexpr int feign_frames = 2 * seconds;
    constexpr int daze_frames = 4 * seconds;

    // Rage conversion from classic: how much damage is worth one point of rage at a level.
    [[nodiscard]] int rage_conversion(int level)
    {
        return (91 * level * level / 10000) + (3226 * level / 1000) + 4;
    }

    // Spells roll spell critical strikes; everything else (shots included) rolls physical ones.
    [[nodiscard]] bool is_spell(ability_id ability)
    {
        return get_ability(ability).player_class == class_id::MAGE;
    }

    [[nodiscard]] buff_id buff_of(ability_id ability)
    {
        switch(ability)
        {

        case ability_id::BATTLE_SHOUT:
            return buff_id::BATTLE_SHOUT;

        case ability_id::LAST_STAND:
            return buff_id::LAST_STAND;

        case ability_id::ICE_BARRIER:
            return buff_id::ICE_BARRIER;

        case ability_id::ARCANE_POWER:
            return buff_id::ARCANE_POWER;

        case ability_id::BESTIAL_WRATH:
            return buff_id::BESTIAL_WRATH;

        case ability_id::ARCANE_INTELLECT:
            return buff_id::ARCANE_INTELLECT;

        case ability_id::FIRE_WARD:
            return buff_id::FIRE_WARD;

        case ability_id::MANA_SHIELD:
            return buff_id::MANA_SHIELD;

        case ability_id::RETALIATION:
            return buff_id::RETALIATION;

        case ability_id::SWEEPING_STRIKES:
            return buff_id::SWEEPING_STRIKES;

        case ability_id::WHIRLING_BLADES:
            return buff_id::WHIRLING_BLADES;

        case ability_id::BERSERKER_RAGE:
            return buff_id::BERSERKER_RAGE;

        case ability_id::RECKLESSNESS:
            return buff_id::RECKLESSNESS;

        case ability_id::DEATH_WISH:
            return buff_id::DEATH_WISH;

        case ability_id::SHIELD_BLOCK:
            return buff_id::SHIELD_BLOCK;

        case ability_id::SHIELD_WALL:
            return buff_id::SHIELD_WALL;

        case ability_id::RAPID_FIRE:
            return buff_id::RAPID_FIRE;

        case ability_id::DETERRENCE:
            return buff_id::DETERRENCE;

        default:
            return buff_id::COUNT;
        }
    }

    constexpr item_id conjured_water[] = {
        item_id::CONJURED_WATER, item_id::CONJURED_FRESH_WATER, item_id::CONJURED_PURE_WATER,
        item_id::CONJURED_BROOK_WATER, item_id::CONJURED_ICE_WATER, item_id::CONJURED_CLEAR_WATER,
        item_id::CONJURED_SNOW_WATER
    };

    constexpr item_id conjured_food[] = {
        item_id::CONJURED_MUFFIN, item_id::CONJURED_BREAD, item_id::CONJURED_RYE, item_id::CONJURED_SOURDOUGH,
        item_id::CONJURED_SWEET_ROLL, item_id::CONJURED_CROISSANT
    };
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
    auto active = [this](buff_id buff)
    {
        return _buffs[int(buff)] > 0;
    };
    auto value = [this](buff_id buff)
    {
        return _buff_values[int(buff)];
    };

    if(active(buff_id::BATTLE_SHOUT))
    {
        bonus.attack_power += value(buff_id::BATTLE_SHOUT);
    }

    if(active(buff_id::FROST_ARMOR))
    {
        bonus.armor += value(buff_id::FROST_ARMOR);
    }

    if(active(buff_id::LAST_STAND))
    {
        bonus.health_percent += value(buff_id::LAST_STAND);
    }

    if(active(buff_id::ARCANE_POWER))
    {
        bonus.damage_percent += value(buff_id::ARCANE_POWER);
    }

    if(active(buff_id::ASPECT_OF_THE_HAWK))
    {
        bonus.ranged_attack_power += value(buff_id::ASPECT_OF_THE_HAWK);
    }

    if(active(buff_id::BESTIAL_WRATH))
    {
        bonus.haste_percent += value(buff_id::BESTIAL_WRATH);
    }

    if(active(buff_id::ARCANE_INTELLECT))
    {
        bonus.intellect += value(buff_id::ARCANE_INTELLECT);
    }

    if(active(buff_id::MOLTEN_ARMOR))
    {
        bonus.spell_crit += 3;
    }

    if(active(buff_id::DEATH_WISH))
    {
        bonus.damage_percent += value(buff_id::DEATH_WISH);
    }

    if(active(buff_id::RECKLESSNESS))
    {
        bonus.damage_taken_percent += value(buff_id::RECKLESSNESS);
    }

    if(active(buff_id::SHIELD_WALL))
    {
        bonus.damage_taken_percent -= value(buff_id::SHIELD_WALL);
    }

    if(active(buff_id::SHIELD_BLOCK))
    {
        bonus.block += value(buff_id::SHIELD_BLOCK);
    }

    if(active(buff_id::ASPECT_OF_THE_MONKEY))
    {
        bonus.dodge += value(buff_id::ASPECT_OF_THE_MONKEY);
    }

    if(active(buff_id::RAPID_FIRE))
    {
        bonus.ranged_haste_percent += value(buff_id::RAPID_FIRE);
    }

    if(active(buff_id::TRUESHOT_AURA))
    {
        bonus.attack_power += value(buff_id::TRUESHOT_AURA);
        bonus.ranged_attack_power += value(buff_id::TRUESHOT_AURA);
    }

    if(active(buff_id::WEAKENED))
    {
        bonus.damage_percent -= value(buff_id::WEAKENED);
    }

    _stats = compute_stats(bonus);

    if(active(buff_id::SUNDERED))
    {
        _stats.armor = _stats.armor * (100 - value(buff_id::SUNDERED)) / 100;
    }

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

int combat::_cost(ability_id ability) const
{
    int cost = ability_cost(ability);

    // Every Arcane Blast in a row costs half again as much.
    if(ability == ability_id::ARCANE_BLAST && _buffs[int(buff_id::ARCANE_BLAST)])
    {
        cost = cost * (100 + 50 * _buff_values[int(buff_id::ARCANE_BLAST)]) / 100;
    }

    return cost;
}

int combat::_cast_time(ability_id ability) const
{
    const ability_def& def = get_ability(ability);

    if(! def.cast_time)
    {
        return 0;
    }

    if(_buffs[int(buff_id::PRESENCE_OF_MIND)] && is_spell(ability) && ! (def.flags & ability_flag::CHANNELED))
    {
        return 0;
    }

    return bn::max(15, int(def.cast_time) - talent_value(talent_effect::CAST_TIME, ability));
}

int combat::_cooldown_frames(ability_id ability) const
{
    int frames = get_ability(ability).cooldown;
    return bn::max(0, frames - talent_value(talent_effect::COOLDOWN, ability) * seconds);
}

int combat::cooldown(ability_id ability) const
{
    int own = _cooldowns[int(ability)];

    if(ability == ability_id::HEROIC_STRIKE || ability == ability_id::CLEAVE)
    {
        return own;
    }

    return bn::max(own, _gcd);
}

const char* combat::_unusable_reason(ability_id ability) const
{
    const ability_def& def = get_ability(ability);

    if(_buffs[int(buff_id::ICE_BLOCK)] && ability != ability_id::ICE_BLOCK)
    {
        return "You are encased in ice";
    }

    if(const char* reason = _control_reason(ability))
    {
        return reason;
    }

    if((def.flags & ability_flag::SHIELD) && ! has_shield())
    {
        return "Requires a shield";
    }

    if(def.flags & ability_flag::REACTIVE)
    {
        if(ability == ability_id::OVERPOWER)
        {
            if(_overpower_frames <= 0)
            {
                return "Only after the target dodges";
            }
        }
        else if(_dodged_frames <= 0)
        {
            return ability == ability_id::REVENGE ? "Only after you dodge or block" : "Only after you dodge";
        }
    }

    if(ability == ability_id::CHARGE && _enemies.any_in_combat())
    {
        return "Can't charge in combat";
    }

    if(ability == ability_id::TELEPORT_STORMWIND && item_count(item_id::TELEPORTATION_RUNE) == 0)
    {
        return "You need a Teleportation Rune";
    }

    if(ability == ability_id::EXECUTE && _target_valid())
    {
        const enemy& target = _enemies.at(_target);

        if(target.health * 5 >= target.max_health)
        {
            return "Target health too high";
        }
    }

    return nullptr;
}

bool combat::usable(ability_id ability) const
{
    if(! knows_ability(ability) || _dead)
    {
        return false;
    }

    if(_power() < _cost(ability) || cooldown(ability) > 0 || _unusable_reason(ability))
    {
        return false;
    }

    if(ability == ability_id::EXECUTE)
    {
        return _target_valid();
    }

    return true;
}

int combat::speed_percent() const
{
    if(_buffs[int(buff_id::ICE_BLOCK)] || _buffs[int(buff_id::ROOTED)] || _buffs[int(buff_id::STUNNED)] ||
       player_incapacitated())
    {
        return 0;
    }

    int percent = 100;

    if(_buffs[int(buff_id::ASPECT_OF_THE_CHEETAH)])
    {
        percent += _buff_values[int(buff_id::ASPECT_OF_THE_CHEETAH)];
    }

    if(_buffs[int(buff_id::DAZED)])
    {
        percent /= 2;
    }

    if(_buffs[int(buff_id::CHILLED)])
    {
        percent = percent * (100 - _buff_values[int(buff_id::CHILLED)]) / 100;
    }

    return percent;
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
    _zones.clear();
    _polymorph_target = -1;
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

    if(! input_enabled || _dead)
    {
        // Releases while busy don't count as taps.
        _held_bar = held_bar::NONE;
        _l_used = true;
        _select_used = true;
        _chord = false;
    }

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

    if(_dodged_frames > 0)
    {
        --_dodged_frames;
    }

    if(_overpower_frames > 0)
    {
        --_overpower_frames;
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

    // Feign Death: lie still until moving.
    if(_feign_frames > 0 && (--_feign_frames == 0 || _player.moving()))
    {
        _feign_frames = 0;
        _player.sprite().set_dead(false);
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
            _update_channel();
            _hud.message("Interrupted", ui::color::RED);
        }
        else
        {
            const ability_def& def = get_ability(_cast_ability);

            // Channeled spells work once a second: a missile, a breath of mana.
            if((def.flags & ability_flag::CHANNELED) && _cast_frames % 60 == 0 && _cast_frames != _cast_total)
            {
                if(_cast_ability == ability_id::ARCANE_MISSILES && _target_valid())
                {
                    _apply_ability(ability_id::ARCANE_MISSILES, _target);
                }
                else if(_cast_ability == ability_id::EVOCATION)
                {
                    data.power = bn::min(_stats.max_power,
                                         int(data.power) + _stats.max_power * ability_value(ability_id::EVOCATION) / 100);
                }
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
            data.power = bn::min(100, int(data.power) + ability_value(ability_id::CHARGE));
            _effects.spark(_head(target.position, 10));
        }
    }

    _update_auto_attack();
    _update_projectiles();
    _update_zones();
    _update_buffs();
    _update_regen();

    if(_target_valid())
    {
        const enemy& target = _enemies.at(_target);
        _effects.set_target(&target.position, true);
    }
}

bool combat::bar_keys_held()
{
    return bn::keypad::l_held() || bn::keypad::r_held() || bn::keypad::select_held();
}

void combat::_read_input()
{
    bool l = bn::keypad::l_held();
    bool r = bn::keypad::r_held();

    // L within a few frames of R is the two pressed together, not R's L slot. L and Select only
    // count as taps when let go quickly: holding them to look at their bar does nothing.
    constexpr int chord_frames = 8;
    constexpr int tap_frames = 20;
    _r_frames = r ? _r_frames + 1 : 0;
    _l_frames = l ? _l_frames + 1 : _l_frames;
    _select_frames = bn::keypad::select_held() ? _select_frames + 1 : _select_frames;

    if(bn::keypad::l_pressed())
    {
        _l_used = r;
        _l_frames = 1;

        if(r && _r_frames > chord_frames && ! _chord)
        {
            _use_slot(bar_id::COMBAT, 2);
        }
        else if(r)
        {
            _chord = true;
        }
    }

    if(bn::keypad::r_pressed() && l)
    {
        _chord = true;
        _l_used = true;
    }

    if(! l || ! r)
    {
        _chord = false;
    }

    if(bn::keypad::l_released() && ! _l_used && ! r && _l_frames <= tap_frames)
    {
        _cycle_target();
    }

    // Select: an Items bar on its own, a quick use when tapped.
    if(bn::keypad::select_pressed())
    {
        _select_used = l || r;
        _select_frames = 1;
    }

    if(bn::keypad::select_held() && ! l && ! r)
    {
        _held_bar = held_bar::ITEMS;
        int slot = bn::keypad::up_pressed() ? 0 : bn::keypad::right_pressed() ? 1 :
                   bn::keypad::down_pressed() ? 2 : bn::keypad::left_pressed() ? 3 : -1;

        if(slot >= 0)
        {
            _select_used = true;
            use_item_slot(slot);
        }

        return;
    }

    if(bn::keypad::select_released() && ! _select_used && ! l && ! r && _select_frames <= tap_frames)
    {
        quick_use();
    }

    _held_bar = _chord ? held_bar::BUFFS : r ? held_bar::COMBAT : l ? held_bar::UTILITY : held_bar::NONE;

    if(_held_bar == held_bar::NONE)
    {
        return;
    }

    bar_id bar = _held_bar == held_bar::BUFFS ? bar_id::BUFFS :
                 _held_bar == held_bar::UTILITY ? bar_id::UTILITY : bar_id::COMBAT;

    // Slots in action bar order: A, B, (L), up, right, down, left.
    int slot = bn::keypad::a_pressed() ? 0 : bn::keypad::b_pressed() ? 1 : bn::keypad::up_pressed() ? 3 :
               bn::keypad::right_pressed() ? 4 : bn::keypad::down_pressed() ? 5 :
               bn::keypad::left_pressed() ? 6 : -1;

    if(slot >= 0)
    {
        _l_used = _l_used || l;
        _use_slot(bar, slot);
    }
}

void combat::_use_slot(bar_id bar, int slot)
{
    ability_id ability = character().action_bars[int(bar)][slot];

    if(ability != ability_id::NONE)
    {
        _use_ability(ability);
    }
}

bool combat::_use_ability(ability_id ability)
{
    const ability_def& def = get_ability(ability);

    // Ice Block again breaks out of the ice.
    if(ability == ability_id::ICE_BLOCK && _buffs[int(buff_id::ICE_BLOCK)])
    {
        _end_buff(buff_id::ICE_BLOCK);
        return true;
    }

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

    if(const char* reason = _unusable_reason(ability))
    {
        _hud.message(reason, ui::color::RED);
        return false;
    }

    int cost = _cost(ability);

    if(_power() < cost)
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

        if(ability == ability_id::CHARGE && d < 32)
        {
            _hud.message("Too close", ui::color::RED);
            return false;
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

        // Using an attack on an enemy also starts auto-attacking it, unless it is meant to hold it.
        if(ability != ability_id::POLYMORPH && ability != ability_id::SCATTER_SHOT &&
           ability != ability_id::WYVERN_STING)
        {
            _auto_attack = true;
        }
    }

    if(ability == ability_id::HEROIC_STRIKE || ability == ability_id::CLEAVE)
    {
        _queued = _queued == ability ? ability_id::NONE : ability;
        return true;
    }

    int cast_time = _cast_time(ability);

    if(cast_time)
    {
        if(_player.moving())
        {
            _hud.message("Can't do that while moving", ui::color::RED);
            return false;
        }

        _cast_ability = ability;
        _cast_frames = cast_time;
        _cast_total = cast_time;
        _gcd = global_cooldown;
        _player.sprite().set_casting(true);

        // Channeled spells pay up front; cast spells pay when they finish.
        if(def.flags & ability_flag::CHANNELED)
        {
            _spend(cost);
            _cooldowns[int(ability)] = _cooldown_frames(ability);

            if(ability == ability_id::BLIZZARD || ability == ability_id::VOLLEY)
            {
                ground_zone zone;
                zone.position = _enemies.at(target).position;
                zone.ability = ability;
                zone.radius = ground_radius;
                zone.frames = cast_time;
                zone.tick = 30;
                zone.value = ability_value(ability);
                zone.channel = true;
                zone.circle = _effects.circle(zone.position, zone.radius, cast_time,
                                              ability == ability_id::BLIZZARD ? circle_style::FROST :
                                                                               circle_style::DANGER);
                _add_zone(zone);
            }
        }

        return true;
    }

    if(def.cast_time && _buffs[int(buff_id::PRESENCE_OF_MIND)])
    {
        _end_buff(buff_id::PRESENCE_OF_MIND);
    }

    _spend(cost);
    _gcd = global_cooldown;
    _cooldowns[int(ability)] = _cooldown_frames(ability);
    _apply_ability(ability, target);
    return true;
}

void combat::_finish_cast()
{
    ability_id ability = _cast_ability;
    const ability_def& def = get_ability(ability);
    _cast_ability = ability_id::NONE;
    _player.sprite().set_casting(false);

    if(def.flags & ability_flag::CHANNELED)
    {
        _update_channel();
        return;
    }

    if(def.target == ability_target::ENEMY)
    {
        if(! _target_valid())
        {
            _hud.message("No target", ui::color::RED);
            return;
        }

        if(! def.range && _target_distance() > melee_range)
        {
            _hud.message("Out of range", ui::color::RED);
            return;
        }
    }

    int cost = _cost(ability);

    if(_power() < cost)
    {
        _hud.message(uses_mana() ? "Not enough mana" : "Not enough rage", ui::color::RED);
        return;
    }

    _spend(cost);
    _cooldowns[int(ability)] = _cooldown_frames(ability);
    _apply_ability(ability, def.target == ability_target::ENEMY ? _target : -1);
}

void combat::_update_channel()
{
    // A channel that stopped takes its rain of ice or arrows with it.
    if(_cast_ability != ability_id::NONE)
    {
        return;
    }

    for(auto it = _zones.begin(); it != _zones.end();)
    {
        if(it->channel)
        {
            _effects.remove_circle(it->circle);
            it = _zones.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

int combat::_roll_weapon(bool& crit) const
{
    int damage = random_range(_stats.melee_min, _stats.melee_max);
    crit = random_chance(_buffs[int(buff_id::RECKLESSNESS)] ? 100 : _stats.crit);
    return damage;
}

bool combat::_roll_crit(ability_id ability, int index) const
{
    bool spell = is_spell(ability);
    int chance = spell ? _stats.spell_crit : _stats.crit;
    chance += talent_value(talent_effect::ABILITY_CRIT, ability);

    if(index >= 0 && index < _enemies.count() && _enemies.at(index).frozen())
    {
        chance += talent_value(talent_effect::FROZEN_CRIT);
    }

    if(spell && get_ability(ability).damage_school == school::FIRE && _buffs[int(buff_id::COMBUSTION)])
    {
        chance += _buff_values[int(buff_id::COMBUSTION)];
    }

    if(! spell && _buffs[int(buff_id::RECKLESSNESS)])
    {
        chance = 100;
    }

    return random_chance(chance);
}

int combat::_roll_spell(ability_id ability, bool& crit) const
{
    int base = ability_value(ability);
    int damage = random_range(base * 9 / 10, base * 11 / 10) + _stats.spell_power * base / 60;
    crit = _roll_crit(ability, _target);
    return damage;
}

bool combat::_roll_miss(int index) const
{
    const enemy& target = _enemies.at(index);
    int difference = int(target.level) - int(character().level);
    int chance = 5 + bn::max(0, difference) * 2;

    if(target.ai.evasion_frames > 0)
    {
        chance += target.ai.evasion_percent;
    }

    return random_chance(chance);
}

bool combat::damage_enemy(int index, int amount, bool crit, bool periodic, school damage_school)
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
        amount += amount * target.marked_percent / 100;
    }

    if(damage_school == school::PHYSICAL && target.sunder_stacks > 0)
    {
        amount += amount * target.sunder_stacks * target.sunder_percent / 100;
    }

    if(damage_school == school::FIRE && target.scorch_stacks > 0)
    {
        amount += amount * target.scorch_stacks * 3 / 100;
    }

    amount = bn::max(1, amount * _stats.damage_percent / 100);

    if(crit && ! periodic)
    {
        amount = amount * _stats.crit_percent / 100;
    }

    // Its own defenses: Shield Wall and the like take a share off, Mana Shield soaks the rest.
    enemy_ability_state& ai = target.ai;

    if(ai.guard_frames > 0 && (! ai.guard_physical || damage_school == school::PHYSICAL))
    {
        amount = bn::max(1, amount * (100 - ai.guard_percent) / 100);
    }

    if(ai.absorb > 0)
    {
        int absorbed = bn::min(int(ai.absorb), amount);
        ai.absorb -= absorbed;
        amount -= absorbed;

        if(amount <= 0)
        {
            _combat_frames = 0;
            _texts.show(_head(target.position, target.sprite ? target.sprite->height() : 24), "Absorb",
                        floating_texts::style::INFO);
            _enemies.damage(index, 0);
            return false;
        }
    }

    _combat_frames = 0;
    int height = target.sprite ? target.sprite->height() : 24;

    if(! periodic)
    {
        play_sound(sound_id::HIT);
    }

    _texts.show_number(_head(target.position, height), amount,
                       crit ? floating_texts::style::CRIT : floating_texts::style::DAMAGE_DEALT);

    if(_target < 0)
    {
        _target = index;
    }

    if(index == _polymorph_target)
    {
        _polymorph_target = -1;
    }

    return _enemies.damage(index, amount);
}

void combat::_weapon_strike(int index, ability_id ability, int bonus, bool can_miss)
{
    enemy& target = _enemies.at(index);
    _player.sprite().play_attack();

    if(can_miss && _roll_miss(index))
    {
        _texts.show(_head(target.position, 24), "Miss", floating_texts::style::INFO);
        _enemies.aggro(index);
        return;
    }

    bool crit;
    int damage = _roll_weapon(crit) + bonus;
    crit = crit || _roll_crit(ability, index);
    _effects.spark(_head(target.position, 8));
    damage_enemy(index, damage, crit);
}

void combat::_spell_hit(int index, ability_id ability, int damage, bool crit)
{
    // Fire spells feed Combustion: more crit chance with each one, until three crits.
    if(get_ability(ability).damage_school == school::FIRE && _buffs[int(buff_id::COMBUSTION)])
    {
        if(crit && ++_combustion_crits >= 3)
        {
            _end_buff(buff_id::COMBUSTION);
        }
        else
        {
            _buff_values[int(buff_id::COMBUSTION)] += 10;
        }
    }

    damage_enemy(index, damage, crit, false, get_ability(ability).damage_school);
}

void combat::_melee_swing(int index)
{
    enemy& target = _enemies.at(index);
    _player.sprite().play_attack();
    _swing_timer = _stats.melee_speed;
    _combat_frames = 0;

    ability_id queued = _queued;
    _queued = ability_id::NONE;
    bool heroic = queued == ability_id::HEROIC_STRIKE && _power() >= _cost(queued);
    bool cleave = queued == ability_id::CLEAVE && _power() >= _cost(queued);

    if(_roll_miss(index))
    {
        // Half the misses are dodges, which open up Overpower.
        if(random_chance(50))
        {
            _texts.show(_head(target.position, 24), "Dodge", floating_texts::style::INFO);
            _overpower_frames = reactive_frames;
        }
        else
        {
            _texts.show(_head(target.position, 24), "Miss", floating_texts::style::INFO);
        }

        _enemies.aggro(index);
        return;
    }

    bool crit;
    int damage = _roll_weapon(crit);

    if(heroic || cleave)
    {
        _spend(_cost(queued));
        damage += ability_value(queued);
        crit = crit || _roll_crit(queued, index);
    }

    _effects.spark(_head(target.position, 8));

    if(! heroic && ! cleave)
    {
        _gain_rage(crit ? damage * 2 : damage, true);
    }

    // Cleave and Sweeping Strikes also hit someone else close by.
    int second = -1;

    if(cleave || (_buffs[int(buff_id::SWEEPING_STRIKES)] && _buff_values[int(buff_id::SWEEPING_STRIKES)] > 0))
    {
        second = _enemies.nearest(_player.position(), melee_range + 6, index, true);
    }

    damage_enemy(index, damage, crit);

    if(second >= 0 && _enemies.at(second).alive())
    {
        damage_enemy(second, damage, crit);

        if(_buffs[int(buff_id::SWEEPING_STRIKES)] && --_buff_values[int(buff_id::SWEEPING_STRIKES)] <= 0)
        {
            _end_buff(buff_id::SWEEPING_STRIKES);
        }
    }
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

    if(! _auto_attack || ! _target_valid() || _cast_ability != ability_id::NONE || _player.dashing() ||
       _buffs[int(buff_id::ICE_BLOCK)] || _controlled())
    {
        return;
    }

    // Swinging at a polymorphed or trapped enemy would wake it.
    if(_enemies.at(_target).incapacitate_frames > 0)
    {
        return;
    }

    int d = _target_distance();

    // A target out of reach while someone else hits the player in melee: turn to that one.
    if(d > melee_range && ! (_stats.has_ranged && character().player_class == class_id::HUNTER))
    {
        if(++_out_of_reach_frames > out_of_reach_switch_frames)
        {
            int closer = _enemies.nearest(_player.position(), melee_range, _target, true);

            if(closer >= 0)
            {
                _target = closer;
                d = _target_distance();
            }

            _out_of_reach_frames = 0;
        }
    }
    else
    {
        _out_of_reach_frames = 0;
    }

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
        if(index == player_target)
        {
            return _dead ? nullptr : &_player.position();
        }

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
    // An enemy's spell reaching the player.
    if(hit.target == player_target)
    {
        _enemy_ability_lands(hit.caster, hit.enemy_ability, hit.damage, _player.position());
        return;
    }

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

    const ability_def& def = get_ability(hit.ability);
    int damage = hit.damage;

    switch(hit.ability)
    {

    case ability_id::FIREBALL:
    case ability_id::PYROBLAST:
    {
        int ticks = def.duration / dot_interval;
        int total = hit.ability == ability_id::PYROBLAST ? hit.damage / 2 : 2 + character().level / 2;

        if(ticks > 0)
        {
            target.dot_damage = bn::max(1, total / ticks);
            target.dot_ticks = ticks;
            target.dot_timer = dot_interval;
        }
        break;
    }

    case ability_id::FROSTBOLT:
        target.slow_frames = def.duration;
        target.slow_percent = 40;

        if(random_chance(talent_value(talent_effect::FREEZE_CHANCE)))
        {
            target.root_frames = 5 * seconds;
            _texts.show(_head(target.position, 30), "Frozen", floating_texts::style::INFO);
        }
        break;

    case ability_id::ICE_LANCE:
        if(target.frozen())
        {
            damage *= 3;
        }
        break;

    case ability_id::SERPENT_STING:
    {
        int ticks = def.duration / dot_interval;
        target.dot_damage = bn::max(1, ability_value(hit.ability) / ticks);
        target.dot_ticks = ticks;
        target.dot_timer = dot_interval;
        _enemies.aggro(hit.target);
        return;
    }

    case ability_id::WYVERN_STING:
        if(_enemies.incapacitate(hit.target, incapacitate_kind::ASLEEP, def.duration))
        {
            target.sting_damage = ability_value(hit.ability);
            _texts.show(_head(target.position, 30), "Asleep", floating_texts::style::INFO);
        }
        else
        {
            _texts.show(_head(target.position, 30), "Immune", floating_texts::style::INFO);
        }
        return;

    case ability_id::CONCUSSIVE_SHOT:
        target.slow_frames = def.duration;
        target.slow_percent = 50;
        break;

    case ability_id::SCATTER_SHOT:
        _interrupt(hit.target);
        damage_enemy(hit.target, damage, hit.crit);

        if(target.alive() && _enemies.incapacitate(hit.target, incapacitate_kind::DISORIENTED, def.duration))
        {
            _texts.show(_head(target.position, 30), "Disoriented", floating_texts::style::INFO);

            if(hit.target == _target)
            {
                _auto_attack = false;
            }
        }
        return;

    default:
        break;
    }

    if(hit.ability == ability_id::NONE)
    {
        damage_enemy(hit.target, damage, hit.crit);
    }
    else
    {
        _spell_hit(hit.target, hit.ability, damage, hit.crit);
    }
}

void combat::_area(ability_id ability, int radius)
{
    const ability_def& def = get_ability(ability);
    const bn::fixed_point& center = _player.position();

    // Cone of Cold only hits what is in front of the player.
    int face_x = 0;
    int face_y = 0;

    switch(_player.direction())
    {

    case facing::UP:
        face_y = -1;
        break;

    case facing::LEFT:
        face_x = -1;
        break;

    case facing::RIGHT:
        face_x = 1;
        break;

    default:
        face_y = 1;
        break;
    }

    for(int index = 0, limit = _enemies.count(); index < limit; ++index)
    {
        enemy& item = _enemies.at(index);

        if(! item.alive() || item.state == enemy_state::EVADE ||
           distance_squared(item.position, center) > radius * radius)
        {
            continue;
        }

        if(ability == ability_id::CONE_OF_COLD)
        {
            int dx = (item.position.x() - center.x()).integer();
            int dy = (item.position.y() - center.y()).integer();
            int along = dx * face_x + dy * face_y;
            int across = bn::abs(dx * face_y - dy * face_x);

            if(along <= 0 || across > along + 8)
            {
                continue;
            }
        }

        switch(ability)
        {

        case ability_id::THUNDER_CLAP:
            item.slow_frames = def.duration;
            item.slow_percent = 30;
            damage_enemy(index, ability_value(ability), false);
            break;

        case ability_id::WHIRLWIND:
        case ability_id::WHIRLING_BLADES:
        {
            bool crit;
            int damage = _roll_weapon(crit) + (ability == ability_id::WHIRLWIND ? ability_value(ability) : 0);
            _effects.spark(_head(item.position, 8));
            damage_enemy(index, damage, crit || _roll_crit(ability, index));
            break;
        }

        case ability_id::DEMORALIZING_SHOUT:
            item.weaken_frames = def.duration;
            item.weaken_percent = ability_value(ability);
            _enemies.aggro(index);
            break;

        case ability_id::INTIMIDATING_SHOUT:
            _enemies.aggro(index);

            if(! item.boss())
            {
                item.fear_frames = def.duration;
                item.incapacitate_frames = 0;
            }
            break;

        default:
        {
            bool crit;
            int damage = _roll_spell(ability, crit);
            crit = _roll_crit(ability, index);

            if(ability == ability_id::FROST_NOVA)
            {
                item.root_frames = def.duration;
            }
            else if(ability == ability_id::CONE_OF_COLD || ability == ability_id::BLAST_WAVE)
            {
                item.slow_frames = def.duration;
                item.slow_percent = 50;

                if(ability == ability_id::CONE_OF_COLD && random_chance(talent_value(talent_effect::FREEZE_CHANCE)))
                {
                    item.root_frames = 5 * seconds;
                }

                // Blast Wave pushes enemies back.
                if(ability == ability_id::BLAST_WAVE)
                {
                    bn::fixed dx = item.position.x() - center.x();
                    bn::fixed dy = item.position.y() - center.y();
                    bn::fixed length = bn::max(bn::fixed(1), bn::fixed(bn::sqrt(dx * dx + dy * dy)));

                    for(int step = 0; step < 8; ++step)
                    {
                        bn::fixed nx = item.position.x() + dx * 2 / length;
                        bn::fixed ny = item.position.y() + dy * 2 / length;

                        if(! enemies::fits(nx, ny))
                        {
                            break;
                        }

                        item.position = bn::fixed_point(nx, ny);
                    }
                }
            }

            _spell_hit(index, ability, damage, crit);
            break;
        }
        }
    }

    switch(ability)
    {

    case ability_id::FROST_NOVA:
    case ability_id::CONE_OF_COLD:
        _effects.circle(center, radius, 30, circle_style::FROST);
        break;

    case ability_id::BLAST_WAVE:
        _effects.circle(center, radius, 30, circle_style::FIRE);
        break;

    case ability_id::ARCANE_EXPLOSION:
        _effects.burst(_head(center, 4), projectile_kind::ARCANE);
        break;

    case ability_id::DEMORALIZING_SHOUT:
    case ability_id::INTIMIDATING_SHOUT:
        _texts.show(_head(center, 26), def.name, floating_texts::style::INFO);
        break;

    default:
        _effects.burst(_head(center, 4), projectile_kind::NONE);
        break;
    }
}

void combat::_add_zone(const ground_zone& zone)
{
    if(_zones.full())
    {
        _effects.remove_circle(_zones.front().circle);
        _zones.erase(_zones.begin());
    }

    _zones.push_back(zone);
}

void combat::_update_zones()
{
    struct sprung_trap
    {
        ground_zone trap;
        int index;
    };

    bn::vector<sprung_trap, 4> sprung;

    for(auto it = _zones.begin(); it != _zones.end();)
    {
        ground_zone& zone = *it;
        bool done = --zone.frames <= 0;

        if(zone.trap)
        {
            // Armed after a moment; goes off under the first enemy to step on it.
            if(zone.tick > 0)
            {
                --zone.tick;
            }
            else
            {
                int index = _enemies.nearest(zone.position, zone.radius);

                if(index >= 0 && ! sprung.full())
                {
                    sprung.push_back(sprung_trap{ zone, index });
                    done = true;
                }
            }
        }
        else if(--zone.tick <= 0)
        {
            const ability_def& def = get_ability(zone.ability);
            bool burning = zone.ability == ability_id::FLAMESTRIKE || zone.ability == ability_id::EXPLOSIVE_TRAP;
            zone.tick = burning ? 2 * seconds : zone.ability == ability_id::FROST_TRAP ? 15 : seconds;

            for(int index = 0, limit = _enemies.count(); index < limit; ++index)
            {
                enemy& item = _enemies.at(index);

                if(! item.alive() || item.state == enemy_state::EVADE ||
                   distance_squared(item.position, zone.position) > zone.radius * zone.radius)
                {
                    continue;
                }

                if(zone.ability == ability_id::BLIZZARD || zone.ability == ability_id::FROST_TRAP)
                {
                    item.slow_frames = bn::max(item.slow_frames, 2 * seconds);
                    item.slow_percent = zone.ability == ability_id::FROST_TRAP ? zone.value : 50;
                }

                if(zone.ability != ability_id::FROST_TRAP)
                {
                    damage_enemy(index, zone.value, false, burning, def.damage_school);
                }
            }
        }

        if(done)
        {
            _effects.remove_circle(zone.circle);
            it = _zones.erase(it);
        }
        else
        {
            ++it;
        }
    }

    for(sprung_trap& item : sprung)
    {
        _spring_trap(item.trap, item.index);
    }
}

void combat::_spring_trap(ground_zone& trap, int index)
{
    const ability_def& def = get_ability(trap.ability);
    int percent = 100 + talent_value(talent_effect::TRAP_PERCENT);
    enemy& target = _enemies.at(index);
    _enemies.aggro(index);

    switch(trap.ability)
    {

    case ability_id::IMMOLATION_TRAP:
    {
        int ticks = def.duration / dot_interval;
        target.dot_damage = bn::max(1, trap.value * percent / 100 / ticks);
        target.dot_ticks = ticks;
        target.dot_timer = dot_interval;
        _effects.burst(_head(target.position, 6), projectile_kind::FIRE);
        break;
    }

    case ability_id::FREEZING_TRAP:
        if(_enemies.incapacitate(index, incapacitate_kind::FROZEN, trap.value * seconds * percent / 100))
        {
            _texts.show(_head(target.position, 30), "Frozen", floating_texts::style::INFO);
        }
        else
        {
            _texts.show(_head(target.position, 30), "Immune", floating_texts::style::INFO);
        }

        _effects.burst(_head(target.position, 6), projectile_kind::FROST);
        break;

    case ability_id::FROST_TRAP:
    {
        ground_zone ice;
        ice.position = trap.position;
        ice.ability = trap.ability;
        ice.radius = ground_radius;
        ice.frames = def.duration * percent / 100;
        ice.tick = 1;
        ice.value = trap.value;
        ice.circle = _effects.circle(ice.position, ice.radius, ice.frames, circle_style::FROST);
        _add_zone(ice);
        break;
    }

    case ability_id::EXPLOSIVE_TRAP:
    {
        int damage = trap.value * percent / 100;
        _effects.burst(_head(trap.position, 4), projectile_kind::FIRE);

        for(int other = 0, limit = _enemies.count(); other < limit; ++other)
        {
            const enemy& item = _enemies.at(other);

            if(item.alive() && item.state != enemy_state::EVADE &&
               distance_squared(item.position, trap.position) <= ground_radius * ground_radius)
            {
                damage_enemy(other, damage, random_chance(_stats.crit), false, school::FIRE);
            }
        }

        ground_zone fire;
        fire.position = trap.position;
        fire.ability = trap.ability;
        fire.radius = ground_radius;
        fire.frames = def.duration * percent / 100;
        fire.tick = 2 * seconds;
        fire.value = bn::max(1, damage / 4);
        fire.circle = _effects.circle(fire.position, fire.radius, fire.frames, circle_style::FIRE);
        _add_zone(fire);
        break;
    }

    default:
        break;
    }
}

void combat::_interrupt(int index)
{
    enemy& target = _enemies.at(index);

    if(target.casting() && (get_enemy_ability(target.ai.casting).flags & enemy_ability_flag::INTERRUPTIBLE))
    {
        stop_enemy_cast(index, true);
        return;
    }

    // For now only the bosses' heavy blows can be stopped while they wind up.
    if((target.id == enemy_id::PRINCESS || target.id == enemy_id::KAM_DEEPFURY) && target.phase == 2 &&
       target.special_timer > 0)
    {
        target.phase = 1;
        target.special_timer = gore_interval;
        _texts.show(_head(target.position, 40), "Interrupted", floating_texts::style::INFO);
    }
}

void combat::_polymorph(int index, int frames)
{
    // One sheep at a time.
    if(_polymorph_target >= 0 && _polymorph_target != index && _polymorph_target < _enemies.count())
    {
        enemy& old = _enemies.at(_polymorph_target);

        if(old.incapacitated == incapacitate_kind::POLYMORPH)
        {
            _enemies.break_control(_polymorph_target);
        }
    }

    enemy& target = _enemies.at(index);

    if(! _enemies.incapacitate(index, incapacitate_kind::POLYMORPH, frames))
    {
        _texts.show(_head(target.position, 30), "Immune", floating_texts::style::INFO);
        return;
    }

    _polymorph_target = index;
    _texts.show(_head(target.position, 30), "Polymorph", floating_texts::style::INFO);

    if(index == _target)
    {
        clear_target();
    }
}

void combat::_set_armor(buff_id armor, ability_id ability)
{
    // One mage armor at a time.
    _buffs[int(buff_id::FROST_ARMOR)] = 0;
    _buffs[int(buff_id::MOLTEN_ARMOR)] = 0;
    _buffs[int(buff_id::MAGE_ARMOR)] = 0;
    _set_buff(armor, get_ability(ability).duration, ability_value(ability));
    _texts.show(_head(_player.position(), 26), get_ability(ability).name, floating_texts::style::INFO);
}

void combat::_set_aspect(buff_id aspect, ability_id ability)
{
    // One aspect at a time, until changed.
    _buffs[int(buff_id::ASPECT_OF_THE_HAWK)] = 0;
    _buffs[int(buff_id::ASPECT_OF_THE_MONKEY)] = 0;
    _buffs[int(buff_id::ASPECT_OF_THE_CHEETAH)] = 0;
    _set_buff(aspect, permanent_buff, ability_value(ability));
    _texts.show(_head(_player.position(), 26), get_ability(ability).name, floating_texts::style::INFO);
}

void combat::_conjure(ability_id ability)
{
    int rank = bn::max(1, ability_rank(ability));
    item_id item = ability == ability_id::CONJURE_WATER ? conjured_water[bn::min(rank, 7) - 1] :
                                                          conjured_food[bn::min(rank, 6) - 1];
    int count = ability_value(ability);
    int left = add_item(item, count);

    if(left == count)
    {
        _hud.message("Your bags are full", ui::color::RED);
        return;
    }

    bn::string<32> text = "+";
    text += bn::to_string<4>(count - left);
    text += " ";
    text += get_item(item).name;
    _hud.message(text, ui::color::GREEN);
}

void combat::_counter_hit(int index)
{
    if(distance_squared(_player.position(), _enemies.at(index).position) <= melee_range * melee_range)
    {
        _weapon_strike(index, ability_id::RETALIATION, 0);
    }
}

void combat::_apply_ability(ability_id ability, int target_index)
{
    const ability_def& def = get_ability(ability);
    character_data& data = character();
    bn::fixed_point from = _head(_player.position(), 8);
    play_sound(sound_id::SPELL);
    _break_free(ability);

    // Plain buffs.
    buff_id buff = buff_of(ability);

    if(buff != buff_id::COUNT)
    {
        int before = _stats.max_health;
        _set_buff(buff, def.duration, ability_value(ability));
        _texts.show(_head(_player.position(), 26), def.name, floating_texts::style::INFO);

        if(ability == ability_id::LAST_STAND)
        {
            heal_player(_stats.max_health - before);
        }
        else if(ability == ability_id::BERSERKER_RAGE)
        {
            data.power = bn::min(100, int(data.power) + ability_value(ability));
        }

        return;
    }

    switch(ability)
    {

    case ability_id::CHARGE:
        _charging = true;
        _charge_target = target_index;
        _player.dash_to(_enemies.at(target_index).position, melee_range - 6);
        break;

    case ability_id::REND:
    {
        enemy& target = _enemies.at(target_index);
        int ticks = def.duration / dot_interval;
        target.dot_damage = bn::max(1, ability_value(ability) / ticks);
        target.dot_ticks = ticks;
        target.dot_timer = dot_interval;
        _enemies.aggro(target_index);
        _player.sprite().play_attack();
        _texts.show(_head(target.position, 24), "Rend", floating_texts::style::INFO);
        break;
    }

    case ability_id::THUNDER_CLAP:
    case ability_id::WHIRLWIND:
    case ability_id::DEMORALIZING_SHOUT:
    case ability_id::INTIMIDATING_SHOUT:
        _player.sprite().play_attack();
        _area(ability, def.range);
        break;

    case ability_id::HAMSTRING:
    case ability_id::WING_CLIP:
    {
        enemy& target = _enemies.at(target_index);
        target.slow_frames = def.duration;
        target.slow_percent = 50;
        _player.sprite().play_attack();
        damage_enemy(target_index, ability_value(ability), false);
        break;
    }

    case ability_id::EXECUTE:
    {
        // Extra rage is converted into extra damage.
        int extra = _power();
        _spend(extra);
        _player.sprite().play_attack();
        _effects.spark(_head(_enemies.at(target_index).position, 8));
        damage_enemy(target_index, ability_value(ability) + extra * 3, _roll_crit(ability, target_index));
        break;
    }

    case ability_id::MORTAL_STRIKE:
    case ability_id::RAPTOR_STRIKE:
    case ability_id::SLAM:
        _weapon_strike(target_index, ability, ability_value(ability));
        break;

    case ability_id::OVERPOWER:
        _overpower_frames = 0;
        _weapon_strike(target_index, ability, ability_value(ability), false);
        break;

    case ability_id::COUNTERATTACK:
        _dodged_frames = 0;
        _enemies.at(target_index).root_frames = def.duration;
        _weapon_strike(target_index, ability, ability_value(ability), false);
        break;

    case ability_id::REVENGE:
    case ability_id::MONGOOSE_BITE:
    case ability_id::SHIELD_SLAM:
    {
        if(ability != ability_id::SHIELD_SLAM)
        {
            _dodged_frames = 0;
        }

        int value = ability_value(ability);
        _player.sprite().play_attack();
        _effects.spark(_head(_enemies.at(target_index).position, 8));
        damage_enemy(target_index, random_range(value * 9 / 10, value * 11 / 10) + _stats.attack_power / 7,
                     _roll_crit(ability, target_index));
        break;
    }

    case ability_id::BLOODTHIRST:
    {
        int damage = _stats.attack_power * ability_value(ability) / 100;
        _player.sprite().play_attack();
        _effects.spark(_head(_enemies.at(target_index).position, 8));
        damage_enemy(target_index, damage, _roll_crit(ability, target_index));
        heal_player(_stats.max_health / 20);
        break;
    }

    case ability_id::PUMMEL:
    case ability_id::SHIELD_BASH:
    case ability_id::CONCUSSION_BLOW:
    {
        enemy& target = _enemies.at(target_index);
        _player.sprite().play_attack();
        _effects.spark(_head(target.position, 8));

        if(ability == ability_id::CONCUSSION_BLOW)
        {
            target.stun_frames = def.duration;
            _texts.show(_head(target.position, 30), "Stunned", floating_texts::style::INFO);
        }
        else
        {
            _interrupt(target_index);

            if(ability == ability_id::SHIELD_BASH)
            {
                target.stun_frames = bn::max(target.stun_frames, int(def.duration));
            }
        }

        damage_enemy(target_index, ability_value(ability), false);
        break;
    }

    case ability_id::SUNDER_ARMOR:
    {
        enemy& target = _enemies.at(target_index);
        target.sunder_stacks = bn::min(5, target.sunder_stacks + 1);
        target.sunder_percent = ability_value(ability);
        target.sunder_frames = def.duration;
        _enemies.aggro(target_index);
        _player.sprite().play_attack();

        bn::string<16> text = "Sunder x";
        text += bn::to_string<4>(target.sunder_stacks);
        _texts.show(_head(target.position, 24), text, floating_texts::style::INFO);
        break;
    }

    case ability_id::DISARM:
    {
        enemy& target = _enemies.at(target_index);
        target.disarm_frames = def.duration;
        _enemies.aggro(target_index);
        _texts.show(_head(target.position, 24), "Disarmed", floating_texts::style::INFO);
        break;
    }

    case ability_id::FIREBALL:
    case ability_id::FROSTBOLT:
    case ability_id::PYROBLAST:
    case ability_id::ARCANE_MISSILES:
    case ability_id::ARCANE_SHOT:
    case ability_id::ICE_LANCE:
    case ability_id::ARCANE_BLAST:
    {
        bool crit;
        int damage = _roll_spell(ability, crit);

        if(ability == ability_id::ARCANE_BLAST)
        {
            // Each blast in a row hits a fifth harder, up to three.
            int& stacks = _buff_values[int(buff_id::ARCANE_BLAST)];
            stacks = _buffs[int(buff_id::ARCANE_BLAST)] ? stacks : 0;
            damage = damage * (100 + 20 * stacks) / 100;
            _set_buff(buff_id::ARCANE_BLAST, def.duration, bn::min(3, stacks + 1));
        }

        if(_roll_miss(target_index))
        {
            damage = 0;
        }

        _effects.launch(from, def.projectile, projectile_hit{ target_index, damage, crit, ability });
        _player.sprite().play_attack();
        break;
    }

    case ability_id::FIRE_BLAST:
    case ability_id::SCORCH:
    {
        bool crit;
        int damage = _roll_spell(ability, crit);
        enemy& target = _enemies.at(target_index);
        _effects.burst(_head(target.position, 10), projectile_kind::FIRE);
        _spell_hit(target_index, ability, damage, crit);

        if(ability == ability_id::SCORCH && target.alive())
        {
            target.scorch_stacks = bn::min(5, target.scorch_stacks + 1);
            target.scorch_frames = def.duration;
        }
        break;
    }

    case ability_id::FLAMESTRIKE:
    {
        // Fire falls on everyone around the target, then the ground burns.
        bn::fixed_point center = _enemies.at(target_index).position;
        _effects.burst(_head(center, 4), projectile_kind::FIRE);

        for(int index = 0, limit = _enemies.count(); index < limit; ++index)
        {
            const enemy& item = _enemies.at(index);

            if(item.alive() && item.state != enemy_state::EVADE &&
               distance_squared(item.position, center) <= ground_radius * ground_radius)
            {
                bool crit;
                int damage = _roll_spell(ability, crit);
                _spell_hit(index, ability, damage, _roll_crit(ability, index));
            }
        }

        ground_zone zone;
        zone.position = center;
        zone.ability = ability;
        zone.radius = ground_radius;
        zone.frames = def.duration;
        zone.tick = 2 * seconds;
        zone.value = bn::max(1, ability_value(ability) / 4);
        zone.circle = _effects.circle(center, ground_radius, def.duration, circle_style::FIRE);
        _add_zone(zone);
        break;
    }

    case ability_id::FROST_ARMOR:
        _set_armor(buff_id::FROST_ARMOR, ability);
        break;

    case ability_id::MOLTEN_ARMOR:
        _set_armor(buff_id::MOLTEN_ARMOR, ability);
        break;

    case ability_id::MAGE_ARMOR:
        _set_armor(buff_id::MAGE_ARMOR, ability);
        break;

    case ability_id::FROST_NOVA:
    case ability_id::ARCANE_EXPLOSION:
    case ability_id::CONE_OF_COLD:
    case ability_id::BLAST_WAVE:
        _area(ability, def.range);
        break;

    case ability_id::COMBUSTION:
        _combustion_crits = 0;
        _set_buff(buff_id::COMBUSTION, def.duration, 0);
        _texts.show(_head(_player.position(), 26), def.name, floating_texts::style::INFO);
        break;

    case ability_id::PRESENCE_OF_MIND:
        _set_buff(buff_id::PRESENCE_OF_MIND, permanent_buff, 0);
        _texts.show(_head(_player.position(), 26), def.name, floating_texts::style::INFO);
        break;

    case ability_id::ICE_BLOCK:
        _cast_ability = ability_id::NONE;
        _player.sprite().set_casting(false);
        _set_buff(buff_id::ICE_BLOCK, def.duration, 0);
        _texts.show(_head(_player.position(), 26), def.name, floating_texts::style::INFO);
        break;

    case ability_id::COLD_SNAP:
        for(ability_id frost : { ability_id::FROST_NOVA, ability_id::CONE_OF_COLD, ability_id::ICE_BARRIER,
                                  ability_id::ICE_BLOCK })
        {
            _cooldowns[int(frost)] = 0;
        }

        _texts.show(_head(_player.position(), 26), def.name, floating_texts::style::INFO);
        break;

    case ability_id::CONJURE_WATER:
    case ability_id::CONJURE_FOOD:
        _conjure(ability);
        break;

    case ability_id::BLINK:
        _player.blink(ability_value(ability));
        _effects.burst(_head(_player.position(), 8), projectile_kind::ARCANE);
        break;

    case ability_id::COUNTERSPELL:
    {
        enemy& target = _enemies.at(target_index);
        target.silence_frames = def.duration;
        _interrupt(target_index);
        _enemies.aggro(target_index);
        _effects.burst(_head(target.position, 10), projectile_kind::ARCANE);
        _texts.show(_head(target.position, 24), "Silenced", floating_texts::style::INFO);
        break;
    }

    case ability_id::POLYMORPH:
        _polymorph(target_index, ability_value(ability) * seconds);
        break;

    case ability_id::SLOW:
    {
        enemy& target = _enemies.at(target_index);
        target.slow_frames = def.duration;
        target.slow_percent = ability_value(ability);
        _enemies.aggro(target_index);
        _texts.show(_head(target.position, 24), "Slowed", floating_texts::style::INFO);
        break;
    }

    case ability_id::TELEPORT_STORMWIND:
    {
        const home_def& city = get_home(home_id::STORMWIND);
        remove_item(item_id::TELEPORTATION_RUNE, 1);
        teleport_map = city.map;
        teleport_point = bn::fixed_point(city.point.x, city.point.y);
        break;
    }

    case ability_id::SERPENT_STING:
    case ability_id::CONCUSSIVE_SHOT:
    case ability_id::AIMED_SHOT:
    case ability_id::SCATTER_SHOT:
    case ability_id::WYVERN_STING:
    {
        bool crit = _roll_crit(ability, target_index);
        int damage = ability == ability_id::SERPENT_STING || ability == ability_id::WYVERN_STING ? 1 :
                random_range(_stats.ranged_min, _stats.ranged_max) + ability_value(ability);

        if(_roll_miss(target_index))
        {
            damage = 0;
        }

        _effects.launch(from, projectile_kind::ARROW, projectile_hit{ target_index, damage, crit, ability });
        _player.sprite().play_attack();
        break;
    }

    case ability_id::HUNTERS_MARK:
    {
        enemy& target = _enemies.at(target_index);
        target.marked_frames = def.duration;
        target.marked_percent = ability_value(ability);
        _enemies.aggro(target_index);
        _texts.show(_head(target.position, 24), "Marked", floating_texts::style::INFO);
        break;
    }

    case ability_id::ASPECT_OF_THE_HAWK:
        _set_aspect(buff_id::ASPECT_OF_THE_HAWK, ability);
        break;

    case ability_id::ASPECT_OF_THE_MONKEY:
        _set_aspect(buff_id::ASPECT_OF_THE_MONKEY, ability);
        break;

    case ability_id::ASPECT_OF_THE_CHEETAH:
        _set_aspect(buff_id::ASPECT_OF_THE_CHEETAH, ability);
        break;

    case ability_id::TRUESHOT_AURA:
        _set_buff(buff_id::TRUESHOT_AURA, permanent_buff, ability_value(ability));
        _texts.show(_head(_player.position(), 26), def.name, floating_texts::style::INFO);
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
                int damage = random_range(_stats.ranged_min, _stats.ranged_max) + ability_value(ability);
                _effects.launch(from, projectile_kind::ARROW,
                                projectile_hit{ index, damage, _roll_crit(ability, index), ability });
                ++hits;
            }
        }

        _player.sprite().play_attack();
        break;
    }

    case ability_id::FEIGN_DEATH:
        // Everyone fighting the hunter gives up and walks home.
        _enemies.reset_combat();
        clear_target();
        _combat_frames = combat_timeout;
        _feign_frames = feign_frames;
        _player.sprite().set_dead(true);
        break;

    case ability_id::IMMOLATION_TRAP:
    case ability_id::FREEZING_TRAP:
    case ability_id::FROST_TRAP:
    case ability_id::EXPLOSIVE_TRAP:
    {
        // One trap at a time, at the hunter's feet.
        for(auto it = _zones.begin(); it != _zones.end(); ++it)
        {
            if(it->trap)
            {
                _effects.remove_circle(it->circle);
                _zones.erase(it);
                break;
            }
        }

        ground_zone trap;
        trap.position = _player.position();
        trap.ability = ability;
        trap.radius = trap_radius;
        trap.frames = trap_frames;
        trap.tick = trap_arm_frames;
        trap.value = ability_value(ability);
        trap.trap = true;
        trap.circle = _effects.circle(trap.position, trap_radius, trap_frames, circle_style::TRAP);
        _add_zone(trap);
        break;
    }

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

void combat::_end_buff(buff_id buff)
{
    if(_buffs[int(buff)])
    {
        _buffs[int(buff)] = 0;
        refresh_stats();
    }
}

void combat::_update_buffs()
{
    bool changed = false;

    for(int index = 0; index < int(buff_id::COUNT); ++index)
    {
        int& frames = _buffs[index];

        if(frames <= 0 || frames == permanent_buff)
        {
            continue;
        }

        --frames;

        // Bleeding, poison, disease and burning hurt every three seconds, the last one as they end.
        if(is_periodic(buff_id(index)) && frames % dot_interval == 0)
        {
            _periodic_tick(buff_id(index));
        }

        if(frames == 0)
        {
            changed = true;
        }
    }

    if(changed)
    {
        refresh_stats();
    }

    // Whirling Blades strikes everything around once a second.
    int& blades = _buffs[int(buff_id::WHIRLING_BLADES)];

    if(blades > 0 && blades % seconds == 0)
    {
        _player.sprite().play_attack();
        _area(ability_id::WHIRLING_BLADES, get_ability(ability_id::WHIRLING_BLADES).range);
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

    if(_controlled())
    {
        return fail("You can't do that now");
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

    case item_type::HEARTHSTONE:
    {
        if(in_combat())
        {
            return fail("You are in combat");
        }

        if(data.play_frames < data.hearthstone_ready)
        {
            int minutes = int((data.hearthstone_ready - data.play_frames) / 3600) + 1;
            _hearth_text = "Ready in ";
            _hearth_text += bn::to_string<4>(minutes);
            _hearth_text += minutes == 1 ? " minute" : " minutes";
            return fail(_hearth_text.c_str());
        }

        // The game takes the player home, as after Teleport.
        const home_def& home = get_home(home_id(data.home));
        data.hearthstone_ready = data.play_frames + hearthstone_cooldown;
        teleport_map = home.map;
        teleport_point = bn::fixed_point(home.point.x, home.point.y);
        return true;
    }

    default:
        return false;
    }

    remove_item(item, 1);
    return true;
}

ability_id combat::missing_buff() const
{
    if(_dead || in_combat())
    {
        return ability_id::NONE;
    }

    struct reminder
    {
        ability_id ability;
        buff_id buffs[3];   // any of them will do
    };

    // Each mage knows one armor, each hunter can pick any aspect.
    constexpr reminder reminders[] = {
        { ability_id::BATTLE_SHOUT, { buff_id::BATTLE_SHOUT, buff_id::BATTLE_SHOUT, buff_id::BATTLE_SHOUT } },
        { ability_id::ARCANE_INTELLECT, { buff_id::ARCANE_INTELLECT, buff_id::ARCANE_INTELLECT,
                                          buff_id::ARCANE_INTELLECT } },
        { ability_id::MOLTEN_ARMOR, { buff_id::MOLTEN_ARMOR, buff_id::MAGE_ARMOR, buff_id::FROST_ARMOR } },
        { ability_id::MAGE_ARMOR, { buff_id::MOLTEN_ARMOR, buff_id::MAGE_ARMOR, buff_id::FROST_ARMOR } },
        { ability_id::FROST_ARMOR, { buff_id::MOLTEN_ARMOR, buff_id::MAGE_ARMOR, buff_id::FROST_ARMOR } },
        { ability_id::TRUESHOT_AURA, { buff_id::TRUESHOT_AURA, buff_id::TRUESHOT_AURA, buff_id::TRUESHOT_AURA } },
        { ability_id::ASPECT_OF_THE_HAWK, { buff_id::ASPECT_OF_THE_HAWK, buff_id::ASPECT_OF_THE_MONKEY,
                                            buff_id::ASPECT_OF_THE_CHEETAH } },
        { ability_id::ASPECT_OF_THE_MONKEY, { buff_id::ASPECT_OF_THE_HAWK, buff_id::ASPECT_OF_THE_MONKEY,
                                              buff_id::ASPECT_OF_THE_CHEETAH } },
    };

    for(const reminder& item : reminders)
    {
        if(knows_ability(item.ability) && ! _buffs[int(item.buffs[0])] && ! _buffs[int(item.buffs[1])] &&
           ! _buffs[int(item.buffs[2])])
        {
            return item.ability;
        }
    }

    return ability_id::NONE;
}

bool combat::use_item_slot(int slot)
{
    item_id item = item_bar_item(slot);

    if(item == item_id::NONE)
    {
        constexpr const char* missing[] = { "No healing potions", "No food", "No drinks", "No hearthstone" };
        item_type type = character().item_bar[slot] != item_id::NONE ? get_item(character().item_bar[slot]).type :
                                                                       item_bar_default(slot);
        int index = type == item_type::POTION ? 0 : type == item_type::FOOD ? 1 : type == item_type::DRINK ? 2 : 3;
        _hud.message(missing[index], ui::color::RED);
        return false;
    }

    return use_item(item);
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

    for(int row = 0, rows = bag_row_count(); row < rows; ++row)
    {
        const item_stack& slot = data.bags[row];
        const item_def& def = get_item(slot.item);

        if(def.type == wanted && def.level <= data.level &&
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
        // The five second rule: mana only comes back after not spending any for a while, or slowly
        // with Mage Armor.
        int regen = _since_cast >= combat_timeout ? _stats.power_regen : 0;

        if(! regen && _buffs[int(buff_id::MAGE_ARMOR)])
        {
            regen = bn::max(1, _stats.power_regen * _buff_values[int(buff_id::MAGE_ARMOR)] / 100);
        }

        if(regen && data.power < _stats.max_power)
        {
            data.power = bn::min(_stats.max_power, int(data.power) + regen);
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

bool combat::enemy_attacks(int index, int percent)
{
    enemy& attacker = _enemies.at(index);

    if(_dead)
    {
        return false;
    }

    _combat_frames = 0;

    // Getting hit while not targeting anything targets the attacker and fights back.
    if(! _target_valid() && ! _buffs[int(buff_id::ICE_BLOCK)])
    {
        _target = index;
        _auto_attack = true;
    }

    bn::fixed_point head = _head(_player.position(), 26);

    if(_buffs[int(buff_id::ICE_BLOCK)])
    {
        _texts.show(head, "Immune", floating_texts::style::INFO);
        return false;
    }

    // Retaliation strikes back at every swing, whether it lands or not.
    if(_buffs[int(buff_id::RETALIATION)])
    {
        _counter_hit(index);

        if(! attacker.alive())
        {
            return false;
        }
    }

    if(_buffs[int(buff_id::DETERRENCE)] || random_chance(_stats.dodge))
    {
        _texts.show(head, "Dodge", floating_texts::style::INFO);
        _dodged_frames = reactive_frames;
        return false;
    }

    if(random_chance(5))
    {
        _texts.show(head, "Miss", floating_texts::style::INFO);
        return false;
    }

    int swing = _enemy_swing(attacker);
    int damage = random_range(swing * 3 / 4, swing * 5 / 4) * percent / 100;

    if(attacker.weaken_frames > 0)
    {
        damage = damage * (100 - bn::min(attacker.weaken_percent, 75)) / 100;
    }

    if(attacker.disarm_frames > 0)
    {
        damage /= 2;
    }

    if(random_chance(5))
    {
        damage = damage * 3 / 2;
    }

    // Armor mitigation, from classic.
    int armor = _stats.armor;
    int reduction = armor * 100 / (armor + 400 + 85 * attacker.level);
    reduction = bn::min(reduction, 75);
    damage = bn::max(1, damage * (100 - reduction) / 100);

    // A blocked hit does half damage and opens up Revenge.
    if(_stats.block > 0 && random_chance(_stats.block))
    {
        _texts.show(_head(_player.position(), 34), "Block", floating_texts::style::INFO);
        damage = bn::max(1, damage / 2);
        _dodged_frames = reactive_frames;
    }

    if(_buffs[int(buff_id::FROST_ARMOR)])
    {
        attacker.slow_frames = 5 * seconds;
        attacker.slow_percent = 30;
    }

    if(_buffs[int(buff_id::MOLTEN_ARMOR)])
    {
        damage_enemy(index, _buff_values[int(buff_id::MOLTEN_ARMOR)], false, true, school::FIRE);
    }

    // The Cheetah is quick but careless: a hit dazes.
    if(_buffs[int(buff_id::ASPECT_OF_THE_CHEETAH)])
    {
        _set_buff(buff_id::DAZED, daze_frames, 0);
    }

    damage_player(damage, attacker.position);
    return true;
}

void combat::damage_player(int amount, const bn::fixed_point& from, school damage_school)
{
    if(_dead)
    {
        return;
    }

    (void) from;
    _combat_frames = 0;
    bn::fixed_point head = _head(_player.position(), 26);

    if(_buffs[int(buff_id::ICE_BLOCK)])
    {
        _texts.show(head, "Immune", floating_texts::style::INFO);
        return;
    }

    amount = bn::max(1, amount * _stats.damage_taken_percent / 100);

    // Shields soak damage first: Fire Ward only fire, Ice Barrier anything, Mana Shield anything for
    // two mana a point.
    auto absorb = [this, &amount](buff_id shield, int mana_per_point)
    {
        int& left = _buff_values[int(shield)];

        if(! _buffs[int(shield)] || left <= 0 || amount <= 0)
        {
            return;
        }

        int absorbed = bn::min(left, amount);

        if(mana_per_point)
        {
            absorbed = bn::min(absorbed, _power() / mana_per_point);
            _spend(absorbed * mana_per_point);
        }

        left -= absorbed;
        amount -= absorbed;

        if(left <= 0 || (mana_per_point && _power() < mana_per_point))
        {
            _end_buff(shield);
        }
    };

    if(damage_school == school::FIRE)
    {
        absorb(buff_id::FIRE_WARD, 0);
    }

    absorb(buff_id::ICE_BARRIER, 0);
    absorb(buff_id::MANA_SHIELD, 2);

    if(amount <= 0)
    {
        _texts.show(head, "Absorb", floating_texts::style::INFO);
        return;
    }

    // Damage wakes the player from sleep and polymorph.
    if(player_incapacitated())
    {
        _end_buff(buff_id::ASLEEP);
        _end_buff(buff_id::POLYMORPHED);
    }

    _gain_rage(amount, false);
    _player.sprite().flash();
    play_sound(sound_id::HIT);
    _texts.show_number(head, amount, floating_texts::style::DAMAGE_TAKEN);

    character_data& data = character();
    data.health -= amount;

    // Getting hit pushes back casting a little, like in WoW. Channels keep going.
    if(_cast_ability != ability_id::NONE && ! (get_ability(_cast_ability).flags & ability_flag::CHANNELED))
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

    if(_buffs[int(buff_id::WOUNDED)])
    {
        amount = amount * (100 - _buff_values[int(buff_id::WOUNDED)]) / 100;
    }

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
    play_sound(sound_id::DEATH);
    _cast_ability = ability_id::NONE;
    _player.sprite().set_casting(false);
    _player.sprite().set_dead(true);
    clear_target();
    _feign_frames = 0;
    _dodged_frames = 0;
    _overpower_frames = 0;
    _polymorph_target = -1;

    for(const ground_zone& zone : _zones)
    {
        _effects.remove_circle(zone.circle);
    }

    _zones.clear();

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

    if(index == _polymorph_target)
    {
        _polymorph_target = -1;
    }

    if(index == _target)
    {
        // Keep fighting whoever else is on the player, like a pet would.
        bool attacking = _auto_attack;
        clear_target();
        int next = _enemies.nearest(_player.position(), target_range, index, true);

        if(next >= 0)
        {
            _target = next;
            _auto_attack = attacking;
        }
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
        // Rested experience doubles kill experience while it lasts, as in WoW.
        xp += use_rest_xp(xp);
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
        play_sound(sound_id::LEVEL_UP);

        if(trainable_count() > 0)
        {
            _hud.message("New ranks at your trainer", ui::color::YELLOW);
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

void combat::_summon_add(const enemy& boss, enemy_id add)
{
    // One add at a time: there is no group to pick them up.
    int side = boss.position.x() < _player.position().x() ? -1 : 1;
    bn::fixed_point position(boss.position.x() + side * 28, boss.position.y() + 8);

    if(! _enemies.fits(position.x(), position.y()))
    {
        position = boss.position;
    }

    _enemies.summon(add, position);
}

bool combat::_update_telegraph(enemy& boss, bool around_boss, int radius, const char* name,
                               projectile_kind kind)
{
    if(boss.telegraph_frames > 0)
    {
        if(--boss.telegraph_frames > 0)
        {
            // The boss holds still while it winds up a whirl around itself.
            if(around_boss)
            {
                boss.moving = false;
                return true;
            }

            return false;
        }

        _effects.burst(boss.special_position, kind);

        if(distance_squared(_player.position(), boss.special_position) <= radius * radius)
        {
            damage_player(boss.damage * 3, boss.special_position);
        }

        boss.special_timer = telegraph_interval;
        return false;
    }

    if(boss.special_timer == 0)
    {
        boss.special_position = around_boss ? boss.position : _player.position();
        boss.telegraph_frames = telegraph_windup;
        _effects.circle(boss.special_position, radius, telegraph_windup, circle_style::DANGER);
        _texts.show(_head(boss.position, 44), name, floating_texts::style::DAMAGE_TAKEN);
    }

    return false;
}

bool combat::_update_wind_up(int index, bool in_melee, const char* name, const char* message)
{
    // Phase 1 waits for the next heavy blow; phase 2 holds still for a moment, then hits for more
    // than double. Returns true while the boss winds up.
    enemy& boss = _enemies.at(index);

    if(boss.phase == 0)
    {
        boss.phase = 1;
        boss.special_timer = gore_interval;
    }
    else if(boss.phase == 1 && boss.special_timer == 0 && in_melee)
    {
        boss.phase = 2;
        boss.special_timer = gore_windup;
        _texts.show(_head(boss.position, 40), name, floating_texts::style::DAMAGE_TAKEN);
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

            _hud.message(message, ui::color::RED);
            enemy_attacks(index, 250);
            boss.attack_timer = boss.def->attack_speed * 6;
        }

        boss.phase = 1;
        boss.special_timer = gore_interval;
    }

    return false;
}

void combat::_update_frenzy(enemy& boss, int health_percent, int below, int phase, const char* name)
{
    // From the phase it reaches below the health threshold, the boss swings half again as often.
    if(boss.phase == phase - 1 && health_percent <= below)
    {
        boss.phase = phase;
        bn::string<48> text = name;
        text += " goes into a frenzy!";
        _hud.message(text, ui::color::RED);
        _texts.show(_head(boss.position, 44), "Frenzy", floating_texts::style::DAMAGE_TAKEN);
    }

    if(boss.phase == phase && boss.attack_timer > 0 && random_chance(50))
    {
        --boss.attack_timer;
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
        if(_update_wind_up(index, in_melee, "Gore", "Princess gores you!"))
        {
            return true;
        }
        break;

    case enemy_id::SNEED:
        // Calls engineers at two thirds of his health; from half health, hurls saw blades at where
        // the player stands, marked on the ground a moment before they land.
        if(boss.phase == 0)
        {
            boss.phase = 1;
            boss.special_timer = 6 * seconds;
            _hud.message("Sneed: Get out of my mine!", ui::color::RED);
        }

        if(boss.phase == 1 && health_percent <= 66)
        {
            boss.phase = 2;
            _summon_add(boss, enemy_id::GOBLIN_ENGINEER);
            _hud.message("Sneed: Engineers, to me!", ui::color::RED);
        }

        if(boss.phase >= 2 && health_percent <= 50 &&
           _update_telegraph(boss, false, saw_radius, "Saw Blade", projectile_kind::FIRE))
        {
            return true;
        }
        break;

    case enemy_id::VANCLEEF:
        // A Blackguard at 70% and 30% of his health; from half health, a whirl of blades around him
        // that the player has to step away from.
        if(boss.phase == 0)
        {
            boss.phase = 1;
            boss.special_timer = 6 * seconds;
            _hud.message("VanCleef: Who dares?", ui::color::RED);
        }

        if(boss.phase == 1 && health_percent <= 70)
        {
            boss.phase = 2;
            _summon_add(boss, enemy_id::DEFIAS_BLACKGUARD);
            _hud.message("VanCleef: Lapdogs, to me!", ui::color::RED);
        }

        if(boss.phase == 2 && health_percent <= 30)
        {
            boss.phase = 3;
            _summon_add(boss, enemy_id::DEFIAS_BLACKGUARD);
            _hud.message("VanCleef: For the Brotherhood!", ui::color::RED);
        }

        if(health_percent <= 50 &&
           _update_telegraph(boss, true, flurry_radius, "Blade Flurry", projectile_kind::ARCANE))
        {
            return true;
        }
        break;

    case enemy_id::HOGGER:
        _update_frenzy(boss, health_percent, 35, 1, "Hogger");
        break;

    case enemy_id::TARGORR:
        if(boss.phase == 0)
        {
            boss.phase = 1;
            _hud.message("Targorr: Fresh meat!", ui::color::RED);
        }

        _update_frenzy(boss, health_percent, 50, 2, "Targorr");
        break;

    case enemy_id::KAM_DEEPFURY:
        // Shield Slam: raises his shield for a moment, then a hit for more than double.
        if(_update_wind_up(index, in_melee, "Shield Slam", "Kam Deepfury slams you!"))
        {
            return true;
        }
        break;

    case enemy_id::BAZIL_THREDD:
        // Smoke bombs at the player's feet from the start; a rioter at two thirds and one third of his
        // health; a frenzy near the end.
        if(boss.phase == 0)
        {
            boss.phase = 1;
            boss.special_timer = 5 * seconds;
            _hud.message("Bazil: Nobody takes me alive!", ui::color::RED);
        }

        if(boss.phase == 1 && health_percent <= 66)
        {
            boss.phase = 2;
            _summon_add(boss, enemy_id::DEFIAS_RIOTER);
            _hud.message("Bazil: Open the cells, lads!", ui::color::RED);
        }

        if(boss.phase == 2 && health_percent <= 33)
        {
            boss.phase = 3;
            _summon_add(boss, enemy_id::DEFIAS_RIOTER);
            _hud.message("Bazil: To me, Brotherhood!", ui::color::RED);
        }

        _update_frenzy(boss, health_percent, 15, 4, "Bazil");

        if(_update_telegraph(boss, false, smoke_radius, "Smoke Bomb", projectile_kind::ARCANE))
        {
            return true;
        }
        break;

    default:
        break;
    }

    return false;
}

}
