#include "gw_combat.h"

#include "bn_keypad.h"
#include "bn_math.h"
#include "bn_string.h"

#include "gw_audio.h"
#include "gw_enemies.h"
#include "gw_floating_text.h"
#include "gw_homes.h"
#include "gw_hud.h"
#include "gw_map_onyxias_lair.h"
#include "gw_map_scholomance.h"
#include "gw_pet.h"
#include "gw_player.h"
#include "gw_quests.h"
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
    constexpr int breath_radius = 30;
    constexpr int breath_reach = 30;                 // how far in front of a dragon its breath lands
    constexpr int shadow_port_interval = 9 * seconds;
    constexpr int onyxia_air_frames = 45 * seconds;
    constexpr int bane_guard_percent = 90;     // Morbent Fel without Sirra's bane
    constexpr int bomb_interval = 10 * seconds; // Thermaplugg sends a Walking Bomb
    constexpr int max_bombs = 3;
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

    if(active(buff_id::ASPECT_OF_THE_BEAST))
    {
        bonus.damage_percent += value(buff_id::ASPECT_OF_THE_BEAST);
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

    if(ability == ability_id::MOUNT && ! _buffs[int(buff_id::MOUNTED)])
    {
        if(in_combat())
        {
            return "Can't mount in combat";
        }

        if(world::map().indoors)
        {
            return "Can't mount indoors";
        }
    }

    if(const char* reason = _pet_reason(ability))
    {
        return reason;
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

    // The fastest of the Cheetah and the mount.
    int percent = 100;
    int bonus = 0;

    if(_buffs[int(buff_id::ASPECT_OF_THE_CHEETAH)])
    {
        bonus = _buff_values[int(buff_id::ASPECT_OF_THE_CHEETAH)];
    }

    if(_buffs[int(buff_id::MOUNTED)])
    {
        bonus = bn::max(bonus, _buff_values[int(buff_id::MOUNTED)]);
    }

    percent += bonus;

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
    _event = -1;
    _events_done = 0;
    _baron_frames = 0;
    _baron_over = false;
    _air_frames = 0;
    _whelp_frames = 0;
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
        dismount();
        _auto_attack = true;
        _player.face(_enemies.at(_target).position);
    }
    else
    {
        clear_target();
    }
}

void combat::dismount()
{
    _end_buff(buff_id::MOUNTED);

    if(_player.mounted())
    {
        _player.dismount();
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
    _update_event();
    _update_baron_clock();

    // A blow dealt or taken ends the ride.
    if(_buffs[int(buff_id::MOUNTED)] && _combat_frames <= 1)
    {
        dismount();
    }

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

    // Mount again gets off; any other ability gets off first.
    if(_buffs[int(buff_id::MOUNTED)])
    {
        dismount();

        if(ability == ability_id::MOUNT)
        {
            return true;
        }
    }

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

        // Sight runs a little above the feet, like the enemies' own: from the head, a table the player
        // stands in front of would hide everything.
        if(def.range && ! world::line_clear(_player.position().x().integer(), _player.position().y().integer() - 4,
                                            target_enemy.position.x().integer(),
                                            target_enemy.position.y().integer() - 4))
        {
            _hud.message("Target not in line of sight", ui::color::RED);
            return false;
        }

        if(ability == ability_id::EXECUTE && target_enemy.health * 5 >= target_enemy.max_health)
        {
            _hud.message("Target health too high", ui::color::RED);
            return false;
        }

        if(ability == ability_id::TAME_BEAST)
        {
            if(const char* reason = _tame_reason(target_enemy))
            {
                _hud.message(reason, ui::color::RED);
                return false;
            }
        }

        _player.face(target_enemy.position);

        // Using an attack on an enemy also starts auto-attacking it, unless it is meant to hold or tame
        // it.
        if(ability != ability_id::POLYMORPH && ability != ability_id::SCATTER_SHOT &&
           ability != ability_id::WYVERN_STING && ability != ability_id::TAME_BEAST)
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

            // The beast fights back while the hunter tames it.
            if(ability == ability_id::TAME_BEAST)
            {
                _tame_target = target;
                _enemies.aggro(target);
                _texts.show(_head(_enemies.at(target).position, 30), "Taming...", floating_texts::style::INFO);
            }

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

        if(ability == ability_id::TAME_BEAST && _tame_target >= 0 && _tame_target < _enemies.count() &&
           _enemies.at(_tame_target).alive())
        {
            _tame(_tame_target);
        }

        _tame_target = -1;
        fish_caught = ability == ability_id::FISHING;
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

    if(target.airborne)
    {
        if(! periodic)
        {
            _texts.show(_head(target.position, target.sprite ? target.sprite->height() : 24), "Immune",
                        floating_texts::style::INFO);
        }

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

    // The pet's bites don't use the player's damage bonuses.
    if(! _pet_hit)
    {
        amount = bn::max(1, amount * _stats.damage_percent / 100);
    }

    if(crit && ! periodic)
    {
        amount = _pet_hit ? amount * 2 : amount * _stats.crit_percent / 100;
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

    if(_target < 0 && ! _pet_hit)
    {
        _target = index;
    }

    if(index == _polymorph_target)
    {
        _polymorph_target = -1;
    }

    if(_enemies.damage(index, amount))
    {
        return true;
    }

    // Threat: the pet's bites count double, so it holds what it fights.
    if(_pet_hit)
    {
        target.pet_threat += amount * 2 + 10;
    }
    else
    {
        target.player_threat += amount;
    }

    return false;
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

        if(world::line_clear(_player.position().x().integer(), _player.position().y().integer() - 4,
                             target.position.x().integer(), target.position.y().integer() - 4))
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

    case ability_id::WATER_ELEMENTAL:
        target.slow_frames = 4 * seconds;
        target.slow_percent = 40;
        pet_hits(hit.target, damage, hit.crit);
        return;

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
    _buffs[int(buff_id::ASPECT_OF_THE_BEAST)] = 0;
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

    case ability_id::ASPECT_OF_THE_BEAST:
        _set_aspect(buff_id::ASPECT_OF_THE_BEAST, ability);
        break;

    case ability_id::CALL_PET:
    {
        const char* reason = nullptr;

        if(! _pet->call(&reason))
        {
            _hud.message(reason, ui::color::RED);
        }
        break;
    }

    case ability_id::REVIVE_PET:
        _pet->revive(ability_value(ability));
        _texts.show(_head(_pet->position(), _pet->height()), "Revived", floating_texts::style::HEAL);
        break;

    case ability_id::MEND_PET:
        _pet->mend(ability_value(ability));
        _texts.show(_head(_pet->position(), _pet->height()), def.name, floating_texts::style::HEAL);
        break;

    case ability_id::KILL_COMMAND:
    case ability_id::INTIMIDATION:
        _pet->command(ability, target_index, ability_value(ability));
        break;

    case ability_id::PET_PASSIVE:
        _hud.message(_pet->toggle_passive() ? "Your pet stays passive" : "Your pet will fight",
                     ui::color::YELLOW);
        break;

    case ability_id::WATER_ELEMENTAL:
        _pet->summon_elemental(def.duration);
        _set_buff(buff_id::WATER_ELEMENTAL, def.duration, 0);
        _effects.burst(_head(_pet->position(), 12), projectile_kind::FROST);
        break;

    case ability_id::MOUNT:
        _set_buff(buff_id::MOUNTED, permanent_buff, ability_value(ability));
        _player.mount(data.race);
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

bool combat::start_fishing()
{
    if(_cast_ability != ability_id::NONE)
    {
        return false;
    }

    if(in_combat())
    {
        _hud.message("Can't fish in combat", ui::color::RED);
        return false;
    }

    if(_player.moving())
    {
        _hud.message("Can't do that while moving", ui::color::RED);
        return false;
    }

    dismount();
    _cast_ability = ability_id::FISHING;
    _cast_frames = get_ability(ability_id::FISHING).cast_time;
    _cast_total = _cast_frames;
    _player.sprite().set_casting(true);
    return true;
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

        dismount();

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
        dismount();
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
        buff_id buffs[4];   // any of them will do
    };

    // Each mage knows one armor, each hunter can pick any aspect.
    using b = buff_id;
    constexpr reminder reminders[] = {
        { ability_id::BATTLE_SHOUT, { b::BATTLE_SHOUT, b::BATTLE_SHOUT, b::BATTLE_SHOUT, b::BATTLE_SHOUT } },
        { ability_id::ARCANE_INTELLECT, { b::ARCANE_INTELLECT, b::ARCANE_INTELLECT, b::ARCANE_INTELLECT,
                                          b::ARCANE_INTELLECT } },
        { ability_id::MOLTEN_ARMOR, { b::MOLTEN_ARMOR, b::MAGE_ARMOR, b::FROST_ARMOR, b::FROST_ARMOR } },
        { ability_id::MAGE_ARMOR, { b::MOLTEN_ARMOR, b::MAGE_ARMOR, b::FROST_ARMOR, b::FROST_ARMOR } },
        { ability_id::FROST_ARMOR, { b::MOLTEN_ARMOR, b::MAGE_ARMOR, b::FROST_ARMOR, b::FROST_ARMOR } },
        { ability_id::TRUESHOT_AURA, { b::TRUESHOT_AURA, b::TRUESHOT_AURA, b::TRUESHOT_AURA, b::TRUESHOT_AURA } },
        { ability_id::ASPECT_OF_THE_BEAST, { b::ASPECT_OF_THE_HAWK, b::ASPECT_OF_THE_MONKEY, b::ASPECT_OF_THE_CHEETAH,
                                             b::ASPECT_OF_THE_BEAST } },
        { ability_id::ASPECT_OF_THE_HAWK, { b::ASPECT_OF_THE_HAWK, b::ASPECT_OF_THE_MONKEY, b::ASPECT_OF_THE_CHEETAH,
                                            b::ASPECT_OF_THE_BEAST } },
        { ability_id::ASPECT_OF_THE_MONKEY, { b::ASPECT_OF_THE_HAWK, b::ASPECT_OF_THE_MONKEY,
                                              b::ASPECT_OF_THE_CHEETAH, b::ASPECT_OF_THE_BEAST } },
    };

    for(const reminder& item : reminders)
    {
        if(knows_ability(item.ability) && ! _buffs[int(item.buffs[0])] && ! _buffs[int(item.buffs[1])] &&
           ! _buffs[int(item.buffs[2])] && ! _buffs[int(item.buffs[3])])
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

bool combat::enemy_attacks_pet(int index)
{
    enemy& attacker = _enemies.at(index);

    if(! _pet || ! _pet->active())
    {
        return false;
    }

    _combat_frames = 0;
    bn::fixed_point head = _head(_pet->position(), _pet->height());

    if(random_chance(8))
    {
        _texts.show(head, "Dodge", floating_texts::style::INFO);
        return false;
    }

    int swing = _enemy_swing(attacker);
    int damage = random_range(swing * 3 / 4, swing * 5 / 4);

    if(attacker.weaken_frames > 0)
    {
        damage = damage * (100 - bn::min(attacker.weaken_percent, 75)) / 100;
    }

    if(attacker.disarm_frames > 0)
    {
        damage /= 2;
    }

    // A thick hide takes a share off.
    damage = bn::max(1, damage * 85 / 100);
    _texts.show_number(head, damage, floating_texts::style::DAMAGE_TAKEN);

    if(_pet->damage(damage) && ! _pet->elemental())
    {
        _end_buff(buff_id::WATER_ELEMENTAL);
    }

    return true;
}

bool combat::pet_hits(int index, int damage, bool crit)
{
    enemy& target = _enemies.at(index);

    if(damage <= 0)
    {
        _texts.show(_head(target.position, 24), "Miss", floating_texts::style::INFO);
        _enemies.aggro(index);
        target.pet_threat += 10;
        return false;
    }

    // Aspect of the Beast: the pet hits harder too.
    if(_buffs[int(buff_id::ASPECT_OF_THE_BEAST)])
    {
        damage += damage * _buff_values[int(buff_id::ASPECT_OF_THE_BEAST)] / 100;
    }

    _pet_hit = true;
    bool died = damage_enemy(index, damage, crit);
    _pet_hit = false;
    return died;
}

void combat::pet_casts(int index, int damage, bool crit)
{
    projectile_hit hit = { index, damage, crit, ability_id::WATER_ELEMENTAL };
    _effects.launch(_head(_pet->position(), 16), projectile_kind::FROST, hit);
}

void combat::pet_freeze(const bn::fixed_point& center, int radius)
{
    _effects.circle(center, radius, 30, circle_style::FROST);

    for(int index = 0, limit = _enemies.count(); index < limit; ++index)
    {
        enemy& item = _enemies.at(index);

        if(item.alive() && item.state != enemy_state::EVADE && ! item.boss() &&
           distance_squared(item.position, center) <= radius * radius)
        {
            item.root_frames = 4 * seconds;
            _texts.show(_head(item.position, 30), "Frozen", floating_texts::style::INFO);
        }
    }
}

const char* combat::_pet_reason(ability_id ability) const
{
    switch(ability)
    {

    case ability_id::CALL_PET:
    case ability_id::PET_PASSIVE:
        if(! _pet->tamed())
        {
            return "You have no pet";
        }

        return ability == ability_id::CALL_PET && _pet->dead() ? "Your pet is dead" : nullptr;

    case ability_id::REVIVE_PET:
        if(! _pet->tamed())
        {
            return "You have no pet";
        }

        return _pet->dead() ? nullptr : "Your pet is alive";

    case ability_id::MEND_PET:
    case ability_id::KILL_COMMAND:
    case ability_id::INTIMIDATION:
        if(_pet->dead())
        {
            return "Your pet is dead";
        }

        return _pet->active() && ! _pet->elemental() ? nullptr : "Your pet is not here";

    default:
        return nullptr;
    }
}

const char* combat::_tame_reason(const enemy& target) const
{
    if(target.def->family != enemy_family::BEAST)
    {
        return "Only beasts can be tamed";
    }

    if(target.elite())
    {
        return "Too strong to tame";
    }

    if(target.level > character().level)
    {
        return "Too high level to tame";
    }

    return nullptr;
}

void combat::_tame(int index)
{
    enemy& beast = _enemies.at(index);
    enemy_id species = beast.id;
    _effects.burst(_head(beast.position, 10), projectile_kind::NATURE);
    _enemies.remove(index);

    if(_target == index)
    {
        clear_target();
    }

    _pet->tame(species);

    if(quests_on_tame(_hud) && on_quest_progress)
    {
        on_quest_progress(callback_context);
    }
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

    // Getting hit pushes back casting a little, like in WoW. Channels keep going, but a hit scares
    // the fish away.
    if(_cast_ability == ability_id::FISHING)
    {
        _cast_ability = ability_id::NONE;
        _player.sprite().set_casting(false);
    }
    else if(_cast_ability != ability_id::NONE && ! (get_ability(_cast_ability).flags & ability_flag::CHANNELED))
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
    _event = -1;

    for(int& frames : _buffs)
    {
        frames = 0;
    }

    dismount();
    refresh_stats();
    _enemies.reset_combat();

    if(_pet)
    {
        _pet->end_elemental();
    }
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

    _boss_killed(item);

    // A revived enemy gave its experience the first time it died.
    if(! item.tapped || item.revived)
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

        if(trainable_count(data.player_class) > 0)
        {
            _hud.message("New ranks at your trainer", ui::color::YELLOW);
        }

        if(data.level == ability_level(ability_id::MOUNT))
        {
            _hud.message("You can learn to ride", ui::color::YELLOW);
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

bool combat::_update_breath(enemy& boss, const char* name, projectile_kind kind)
{
    // A dragon's breath lands in front of it, between it and the hero, and it holds still while it
    // draws breath: the hero steps aside or behind it.
    if(boss.telegraph_frames == 0 && boss.special_timer == 0)
    {
        bn::fixed dx = bn::clamp(_player.position().x() - boss.position.x(), bn::fixed(-256), bn::fixed(256));
        bn::fixed dy = bn::clamp(_player.position().y() - boss.position.y(), bn::fixed(-256), bn::fixed(256));
        bn::fixed length = bn::sqrt(dx * dx + dy * dy);

        if(length < 1)
        {
            dx = -1;
            length = 1;
        }

        boss.special_position = bn::fixed_point(boss.position.x() + dx * breath_reach / length,
                                                boss.position.y() + dy * breath_reach / length);
        boss.telegraph_frames = telegraph_windup;
        _effects.circle(boss.special_position, breath_radius, telegraph_windup, circle_style::DANGER);
        _texts.show(_head(boss.position, 44), name, floating_texts::style::DAMAGE_TAKEN);
    }

    return _update_telegraph(boss, true, breath_radius, name, kind);
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
        text += " is in a frenzy!";
        _hud.message(text, ui::color::RED);
        _texts.show(_head(boss.position, 44), "Frenzy", floating_texts::style::DAMAGE_TAKEN);
    }

    if(boss.phase == phase && boss.attack_timer > 0 && random_chance(50))
    {
        --boss.attack_timer;
    }
}

void combat::_shadow_port(int index)
{
    // Steps out on the ledge of the map's patrol list farthest from the player.
    enemy& boss = _enemies.at(index);
    const point_def* best = nullptr;
    int best_distance = -1;

    for(const point_def& point : world::map().patrol)
    {
        bn::fixed_point position(point.x, point.y);
        int player_distance = distance_squared(position, _player.position());

        if(distance_squared(position, boss.position) > 16 * 16 && player_distance > best_distance)
        {
            best = &point;
            best_distance = player_distance;
        }
    }

    if(! best)
    {
        return;
    }

    if(boss.casting())
    {
        stop_enemy_cast(index, false);
    }

    _effects.burst(boss.position, projectile_kind::SHADOW);
    boss.position = bn::fixed_point(best->x, best->y);
    boss.moving = false;
    _effects.burst(boss.position, projectile_kind::SHADOW);
    _texts.show(_head(boss.position, 44), "Shadow Port", floating_texts::style::DAMAGE_TAKEN);
}

void combat::_boss_greeting(enemy& boss, const char* message)
{
    if(boss.phase == 0)
    {
        boss.phase = 1;
        _hud.message(message, ui::color::RED);
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

    // --- Duskwood ----------------------------------------------------------------------------------

    case enemy_id::MOR_LADIM:
        _boss_greeting(boss, "Mor'Ladim: Who wakes me?");
        _update_frenzy(boss, health_percent, 30, 2, "Mor'Ladim");
        break;

    case enemy_id::STALVAN_MISTMANTLE:
        _boss_greeting(boss, "Stalvan: She is mine!");
        _update_frenzy(boss, health_percent, 30, 2, "Stalvan");
        break;

    case enemy_id::MORBENT_FEL:
        // Steel and spells barely scratch him unless the player carries Sirra's bane (the quest to
        // kill him is in the log).
        if(boss.phase == 0)
        {
            boss.phase = 1;
            _hud.message(quest_wants_kill(enemy_id::MORBENT_FEL) ? "Morbent Fel: The bane?!" :
                                                                   "Morbent: Steel can't hurt me!",
                         ui::color::RED);
        }

        if(! quest_wants_kill(enemy_id::MORBENT_FEL))
        {
            boss.ai.guard_frames = 2;
            boss.ai.guard_percent = bane_guard_percent;
            boss.ai.guard_physical = false;
        }
        break;

    case enemy_id::STITCHES:
        _boss_greeting(boss, "Stitches lumbers at you!");
        _update_frenzy(boss, health_percent, 25, 2, "Stitches");
        break;

    // --- Shadowfang Keep ---------------------------------------------------------------------------

    case enemy_id::RETHILGORE:
        // Maul: rears up for a moment, then a hit for more than double.
        if(_update_wind_up(index, in_melee, "Maul", "Rethilgore mauls you!"))
        {
            return true;
        }
        break;

    case enemy_id::RAZORCLAW_THE_BUTCHER:
        // From half health, spins his cleavers around him; a frenzy near the end.
        _boss_greeting(boss, "Razorclaw: More meat!");
        _update_frenzy(boss, health_percent, 25, 2, "Razorclaw");

        if(health_percent <= 50 &&
           _update_telegraph(boss, true, flurry_radius, "Cleaver Spin", projectile_kind::ARCANE))
        {
            return true;
        }
        break;

    case enemy_id::BARON_SILVERLAINE:
        // Veil of Shadow lands where the player stood, from the start.
        if(boss.phase == 0)
        {
            boss.phase = 1;
            boss.special_timer = 4 * seconds;
            _hud.message("Silverlaine: Leave my keep!", ui::color::RED);
        }

        if(_update_telegraph(boss, false, smoke_radius, "Veil of Shadow", projectile_kind::SHADOW))
        {
            return true;
        }
        break;

    case enemy_id::COMMANDER_SPRINGVALE:
        // Heals himself under half health (interrupt it); Divine Protection once, near the end.
        _boss_greeting(boss, "Springvale: I serve another!");

        if(boss.phase == 1 && health_percent <= 25)
        {
            boss.phase = 2;
            boss.ai.guard_frames = 6 * seconds;
            boss.ai.guard_percent = 50;
            boss.ai.guard_physical = false;
            _texts.show(_head(boss.position, 44), "Divine Protection", floating_texts::style::DAMAGE_TAKEN);
        }
        break;

    case enemy_id::ODO_THE_BLINDWATCHER:
        _boss_greeting(boss, "Odo the Blindwatcher howls!");
        _update_frenzy(boss, health_percent, 40, 2, "Odo");
        break;

    case enemy_id::ARUGAL:
        // Shadow Port: every few seconds he vanishes and steps out on another ledge of his chamber;
        // a Lupine Horror at two thirds and one third of his health.
        if(boss.phase == 0)
        {
            boss.phase = 1;
            boss.special_timer = shadow_port_interval;
            _hud.message("Arugal: Who dares enter?", ui::color::RED);
        }

        if(boss.phase == 1 && health_percent <= 66)
        {
            boss.phase = 2;
            _summon_add(boss, enemy_id::LUPINE_HORROR);
            _hud.message("Arugal: To me, my children!", ui::color::RED);
        }

        if(boss.phase == 2 && health_percent <= 33)
        {
            boss.phase = 3;
            _summon_add(boss, enemy_id::LUPINE_HORROR);
            _hud.message("Arugal: You'll be one of them!", ui::color::RED);
        }

        if(boss.special_timer == 0)
        {
            _shadow_port(index);
            boss.special_timer = shadow_port_interval;
        }
        break;

    // --- The Wetlands ------------------------------------------------------------------------------

    case enemy_id::BALGARAS_THE_FOUL:
        _boss_greeting(boss, "Balgaras: The span will burn!");
        _update_frenzy(boss, health_percent, 30, 2, "Balgaras");
        break;

    case enemy_id::NEK_ROSH:
        // A grunt comes running at half health; a frenzy near the end.
        _boss_greeting(boss, "Nek'rosh: Die, dwarf-friend!");

        if(boss.phase == 1 && health_percent <= 50)
        {
            boss.phase = 2;
            _summon_add(boss, enemy_id::DRAGONMAW_GRUNT);
            _hud.message("Nek'rosh: Grunts, to me!", ui::color::RED);
        }

        _update_frenzy(boss, health_percent, 25, 3, "Nek'rosh");
        break;

    // --- Blackfathom Deeps -------------------------------------------------------------------------

    case enemy_id::GHAMOO_RA:
        // Shell Slam: draws into its shell for a moment, then a hit for more than double.
        if(_update_wind_up(index, in_melee, "Shell Slam", "Ghamoo-ra slams you!"))
        {
            return true;
        }
        break;

    case enemy_id::LADY_SAREVESS:
        // Forked Lightning is one of her abilities; a myrmidon guards her from half health.
        _boss_greeting(boss, "Sarevess: You will drown here!");

        if(boss.phase == 1 && health_percent <= 50)
        {
            boss.phase = 2;
            _summon_add(boss, enemy_id::BLACKFATHOM_MYRMIDON);
            _hud.message("Sarevess: Guards, to me!", ui::color::RED);
        }
        break;

    case enemy_id::GELIHAST:
        _boss_greeting(boss, "Gelihast: Mrglmrglmrgl!");

        if(boss.phase == 1 && health_percent <= 60)
        {
            boss.phase = 2;
            _summon_add(boss, enemy_id::BLINDLIGHT_MURLOC);
        }

        _update_frenzy(boss, health_percent, 30, 3, "Gelihast");
        break;

    case enemy_id::TWILIGHT_LORD_KELRIS:
        // Mind Blast lands where the player stood; an acolyte joins at half health.
        if(boss.phase == 0)
        {
            boss.phase = 1;
            boss.special_timer = 5 * seconds;
            _hud.message("Kelris: Who dares disturb me?", ui::color::RED);
        }

        if(boss.phase == 1 && health_percent <= 50)
        {
            boss.phase = 2;
            _summon_add(boss, enemy_id::TWILIGHT_ACOLYTE);
            _hud.message("Kelris: Brothers, to me!", ui::color::RED);
        }

        if(_update_telegraph(boss, false, smoke_radius, "Mind Blast", projectile_kind::SHADOW))
        {
            return true;
        }
        break;

    case enemy_id::AKU_MAI:
        // From half health, lashes everything around it with its heads.
        _boss_greeting(boss, "Aku'mai rises from the deep!");

        if(health_percent <= 50 &&
           _update_telegraph(boss, true, flurry_radius, "Thrashing Heads", projectile_kind::NATURE))
        {
            return true;
        }
        break;

    // --- Gnomeregan --------------------------------------------------------------------------------

    case enemy_id::GRUBBIS:
        _boss_greeting(boss, "Grubbis: Grubbis smash!");

        if(boss.phase == 1 && health_percent <= 50)
        {
            boss.phase = 2;
            _summon_add(boss, enemy_id::CAVERNDEEP_BURROWER);
            _hud.message("A burrower digs up to help!", ui::color::RED);
        }
        break;

    case enemy_id::VISCOUS_FALLOUT:
        _boss_greeting(boss, "Viscous Fallout oozes at you!");

        if(boss.phase == 1 && health_percent <= 50)
        {
            boss.phase = 2;
            _summon_add(boss, enemy_id::IRRADIATED_SLIME);
            _hud.message("Viscous Fallout splits!", ui::color::RED);
        }
        break;

    case enemy_id::ELECTROCUTIONER_6000:
        // Megavolt: charges up for a moment, then a shock for more than double.
        if(_update_wind_up(index, in_melee, "Megavolt", "The Electrocutioner zaps you!"))
        {
            return true;
        }
        break;

    case enemy_id::CROWD_PUMMELER:
        _boss_greeting(boss, "Pummeler: CROWD PUMMEL ENGAGED");
        _update_frenzy(boss, health_percent, 30, 2, "Crowd Pummeler");
        break;

    case enemy_id::MEKGINEER_THERMAPLUGG:
        // A Walking Bomb climbs out of a random hatch every few seconds (the map's patrol points),
        // at most three at a time; a frenzy near the end.
        if(boss.phase == 0)
        {
            boss.phase = 1;
            boss.special_timer = 4 * seconds;
            _hud.message("Thermaplugg: Usurpers! Begone!", ui::color::RED);
        }

        if(boss.special_timer == 0)
        {
            boss.special_timer = bomb_interval;
            _launch_bomb();
        }

        _update_frenzy(boss, health_percent, 20, 2, "Thermaplugg");
        break;

    // --- Hillsbrad Foothills -----------------------------------------------------------------------

    case enemy_id::GRAVIS_SLIPKNOT:
        // A thief comes out of the shadows at half health.
        _boss_greeting(boss, "Gravis: Nobody leaves alive!");

        if(boss.phase == 1 && health_percent <= 50)
        {
            boss.phase = 2;
            _summon_add(boss, enemy_id::SYNDICATE_THIEF);
            _hud.message("Gravis: To me, Syndicate!", ui::color::RED);
        }
        break;

    case enemy_id::BLOODFANG:
        _boss_greeting(boss, "Bloodfang roars!");
        _update_frenzy(boss, health_percent, 30, 2, "Bloodfang");
        break;

    // --- Scarlet Monastery: Graveyard --------------------------------------------------------------

    case enemy_id::INTERROGATOR_VISHAS:
        _boss_greeting(boss, "Vishas: Tell me your secrets!");
        _update_frenzy(boss, health_percent, 25, 2, "Vishas");
        break;

    case enemy_id::AZSHIR_THE_SLEEPLESS:
        _boss_greeting(boss, "Azshir the Sleepless wails!");
        break;

    case enemy_id::BLOODMAGE_THALNOS:
        _boss_greeting(boss, "Thalnos: No rest for the dead!");
        _update_frenzy(boss, health_percent, 25, 2, "Thalnos");
        break;

    case enemy_id::IRONSPINE:
        _boss_greeting(boss, "Ironspine rattles to life!");
        _update_frenzy(boss, health_percent, 30, 2, "Ironspine");
        break;

    // --- Scarlet Monastery: Library ----------------------------------------------------------------

    case enemy_id::HOUNDMASTER_LOKSEY:
        // Another hound off the leash at 60%, then Bloodlust near the end.
        _boss_greeting(boss, "Loksey: Release the hounds!");

        if(boss.phase == 1 && health_percent <= 60)
        {
            boss.phase = 2;
            _summon_add(boss, enemy_id::SCARLET_TRACKING_HOUND);
        }

        _update_frenzy(boss, health_percent, 30, 3, "Loksey");
        break;

    case enemy_id::ARCANIST_DOAN:
        // At half health an Arcane Bubble shields him while Detonation builds: run out of its circle.
        _boss_greeting(boss, "Doan: Who disturbs my study?");

        if(boss.phase == 1 && health_percent <= 50)
        {
            boss.phase = 2;
            boss.ai.guard_frames = 3 * seconds;
            boss.ai.guard_percent = 100;
            boss.ai.guard_physical = false;
            _texts.show(_head(boss.position, 44), "Arcane Bubble", floating_texts::style::DAMAGE_TAKEN);
            _hud.message("Doan: Burn in righteous fire!", ui::color::RED);
        }
        break;

    // --- Stranglethorn Vale ------------------------------------------------------------------------

    case enemy_id::KING_BANGALASH:
        // A panther comes out of the jungle at his call.
        _boss_greeting(boss, "King Bangalash roars!");

        if(boss.phase == 1 && health_percent <= 50)
        {
            boss.phase = 2;
            _summon_add(boss, enemy_id::SHADOWMAW_PANTHER);
            _hud.message("A panther answers the call!", ui::color::RED);
        }
        break;

    case enemy_id::FLEET_MASTER_FIRALLON:
        _boss_greeting(boss, "Firallon: Repel boarders!");
        _update_frenzy(boss, health_percent, 30, 2, "Firallon");
        break;

    // --- Scarlet Monastery: Armory -----------------------------------------------------------------

    case enemy_id::HEROD:
        // Whirlwind comes from his ability table: a long wind-up, then a spin. Out of the circle!
        _boss_greeting(boss, "Herod: Ah, I've been waiting!");
        _update_frenzy(boss, health_percent, 20, 2, "Herod");
        break;

    // --- Scarlet Monastery: Cathedral --------------------------------------------------------------

    case enemy_id::HIGH_INQUISITOR_FAIRBANKS:
        _boss_greeting(boss, "Fairbanks: Leave this place!");
        _update_frenzy(boss, health_percent, 25, 2, "Fairbanks");
        break;

    case enemy_id::SCARLET_COMMANDER_MOGRAINE:
        _boss_greeting(boss, "Mograine: Burn, infidel!");
        break;

    case enemy_id::HIGH_INQUISITOR_WHITEMANE:
        // At half health Deep Sleep (from her table) puts the hero to sleep, and she raises Mograine
        // where he fell, or calls him if he still stands by the altar.
        _boss_greeting(boss, "Whitemane: You will pay!");

        if(boss.phase == 1 && health_percent <= 50)
        {
            boss.phase = 2;
            int mograine = _find_enemy(enemy_id::SCARLET_COMMANDER_MOGRAINE);

            if(mograine >= 0 && _enemies.revive(mograine, 40))
            {
                _effects.burst(_enemies.at(mograine).position, projectile_kind::HOLY);
                _texts.show(_head(_enemies.at(mograine).position, 44), "Resurrection",
                            floating_texts::style::DAMAGE_TAKEN);
                _hud.message("Whitemane: Arise, my champion!", ui::color::RED);
            }
            else if(mograine >= 0)
            {
                _enemies.aggro(mograine);
            }
        }
        break;

    // --- Tanaris -----------------------------------------------------------------------------------

    case enemy_id::CALIPH_SCORPIDSTING:
        _boss_greeting(boss, "Caliph: The desert is mine!");
        _update_frenzy(boss, health_percent, 30, 2, "Caliph");
        break;

    case enemy_id::ANDRE_FIREBEARD:
        _boss_greeting(boss, "Firebeard: Light 'em up, boys!");
        _update_frenzy(boss, health_percent, 30, 2, "Firebeard");
        break;

    // --- Razorfen Kraul ----------------------------------------------------------------------------

    case enemy_id::AGGEM_THORNCURSE:
        // A boar comes out of the pens at half health.
        _boss_greeting(boss, "Aggem: The thorns will drink!");

        if(boss.phase == 1 && health_percent <= 50)
        {
            boss.phase = 2;
            _summon_add(boss, enemy_id::RAGING_AGAMAR);
            _hud.message("Aggem calls a boar!", ui::color::RED);
        }
        break;

    case enemy_id::DEATH_SPEAKER_JARGBA:
        _boss_greeting(boss, "Jargba: Your soul is mine!");
        _update_frenzy(boss, health_percent, 25, 2, "Jargba");
        break;

    case enemy_id::OVERLORD_RAMTUSK:
        _boss_greeting(boss, "Ramtusk: Intruders! Kill them!");
        _update_frenzy(boss, health_percent, 30, 2, "Ramtusk");
        break;

    case enemy_id::AGATHELOS_THE_RAGING:
        _boss_greeting(boss, "Agathelos bellows!");
        _update_frenzy(boss, health_percent, 30, 2, "Agathelos");
        break;

    case enemy_id::CHARLGA_RAZORFLANK:
        // Heals herself and her guards (her HEALER table); a quilguard runs in at half health.
        _boss_greeting(boss, "Charlga: The thorns shield us!");

        if(boss.phase == 1 && health_percent <= 50)
        {
            boss.phase = 2;
            _summon_add(boss, enemy_id::RAZORFEN_QUILGUARD);
            _hud.message("Charlga: Guards, to me!", ui::color::RED);
        }
        break;

    // --- Razorfen Downs ----------------------------------------------------------------------------

    case enemy_id::TUTEN_KASH:
        _update_frenzy(boss, health_percent, 30, 1, "Tuten'kash");
        break;

    case enemy_id::MORDRESH_FIRE_EYE:
        _boss_greeting(boss, "Mordresh: You will burn!");
        _update_frenzy(boss, health_percent, 25, 2, "Mordresh");
        break;

    case enemy_id::GLUTTON:
        _boss_greeting(boss, "Glutton: Me hungry!");
        _update_frenzy(boss, health_percent, 50, 2, "Glutton");
        break;

    case enemy_id::AMNENNAR_THE_COLDBRINGER:
        // A Frozen Spectre at half health and another at a quarter; Frost Nova is in his table.
        _boss_greeting(boss, "Amnennar: None leave alive!");

        if(boss.phase == 1 && health_percent <= 50)
        {
            boss.phase = 2;
            _summon_add(boss, enemy_id::FROZEN_SPECTRE);
            _hud.message("Amnennar: Come, spirits!", ui::color::RED);
        }

        if(boss.phase == 2 && health_percent <= 25)
        {
            boss.phase = 3;
            _summon_add(boss, enemy_id::FROZEN_SPECTRE);
            _hud.message("Amnennar: Rise, my servants!", ui::color::RED);
        }
        break;

    // --- Zul'Farrak --------------------------------------------------------------------------------

    case enemy_id::ANTU_SUL:
        // Her basilisk brood climbs out of the sand at three quarters and a quarter of her health.
        _boss_greeting(boss, "Antu'sul: Lunch has arrived!");

        if(boss.phase == 1 && health_percent <= 75)
        {
            boss.phase = 2;
            _summon_add(boss, enemy_id::SULLITHUZ_BROODLING);
            _hud.message("Antu'sul calls her brood!", ui::color::RED);
        }

        if(boss.phase == 2 && health_percent <= 25)
        {
            boss.phase = 3;
            _summon_add(boss, enemy_id::SULLITHUZ_BROODLING);
            _hud.message("Antu'sul calls her brood!", ui::color::RED);
        }
        break;

    case enemy_id::THEKA_THE_MARTYR:
        // Shield Wall, from his table, near the end.
        _boss_greeting(boss, "Theka: My faith is my shield!");
        break;

    case enemy_id::WITCH_DOCTOR_ZUM_RAH:
        // Raises a zombie from the graveyard at two thirds and one third of his health.
        _boss_greeting(boss, "Zum'rah: Sands take you!");

        if((boss.phase == 1 && health_percent <= 66) || (boss.phase == 2 && health_percent <= 33))
        {
            ++boss.phase;
            _summon_add(boss, enemy_id::ZULFARRAK_ZOMBIE);
            _hud.message("Zum'rah raises the dead!", ui::color::RED);
        }
        break;

    case enemy_id::GAHZ_RILLA:
        _update_frenzy(boss, health_percent, 30, 1, "Gahz'rilla");
        break;

    case enemy_id::NEKRUM_GUTCHEWER:
        _update_frenzy(boss, health_percent, 30, 1, "Nekrum");
        break;

    case enemy_id::RUUZLU:
        _boss_greeting(boss, "Ruuzlu: For Ukorz!");
        _update_frenzy(boss, health_percent, 30, 2, "Ruuzlu");
        break;

    case enemy_id::CHIEF_UKORZ_SANDSCALP:
        // Ruuzlu, his guard, joins the fight.
        if(boss.phase == 0)
        {
            int ruuzlu = _find_enemy(enemy_id::RUUZLU);

            if(ruuzlu >= 0 && _enemies.at(ruuzlu).state == enemy_state::IDLE)
            {
                _enemies.aggro(ruuzlu);
            }
        }

        _boss_greeting(boss, "Ukorz: Who dares enter here?");
        _update_frenzy(boss, health_percent, 25, 2, "Ukorz");
        break;

    // --- Maraudon ----------------------------------------------------------------------------------

    case enemy_id::NOXXION:
        // Splits off two spawn of slime at two thirds and one third of its health.
        _boss_greeting(boss, "Noxxion bubbles and seethes!");

        if((boss.phase == 1 && health_percent <= 66) || (boss.phase == 2 && health_percent <= 33))
        {
            ++boss.phase;
            _summon_add(boss, enemy_id::NOXXIOUS_SPAWN);
            _summon_add(boss, enemy_id::NOXXIOUS_SPAWN);
            _hud.message("Noxxion splits apart!", ui::color::RED);
        }
        break;

    case enemy_id::RAZORLASH:
        _update_frenzy(boss, health_percent, 30, 1, "Razorlash");
        break;

    case enemy_id::LORD_VYLETONGUE:
        // A shadowstalker steps out of the dark at half his health.
        _boss_greeting(boss, "Vyletongue: You will rot here!");

        if(boss.phase == 1 && health_percent <= 50)
        {
            boss.phase = 2;
            _summon_add(boss, enemy_id::PUTRIDUS_SHADOWSTALKER);
            _hud.message("Vyletongue: Kill the intruder!", ui::color::RED);
        }
        break;

    case enemy_id::CELEBRAS_THE_CURSED:
        // From under two thirds of his health, thorns burst out of the ground around him.
        _boss_greeting(boss, "Celebras: Free... me...");

        if(health_percent <= 66 &&
           _update_telegraph(boss, true, flurry_radius, "Thorn Burst", projectile_kind::NATURE))
        {
            return true;
        }
        break;

    case enemy_id::LANDSLIDE:
        // Breaks a shardling off himself at half health, and rages near the end.
        _boss_greeting(boss, "The ground shakes!");

        if(boss.phase == 1 && health_percent <= 50)
        {
            boss.phase = 2;
            _summon_add(boss, enemy_id::THERADRIM_SHARDLING);
            _hud.message("A shard breaks off Landslide!", ui::color::RED);
        }

        _update_frenzy(boss, health_percent, 25, 3, "Landslide");
        break;

    case enemy_id::TINKERER_GIZLOCK:
        // Lobs bombs at where the hero stands, marked on the ground before they go off.
        _boss_greeting(boss, "Gizlock: Get out of my lab!");

        if(health_percent <= 80 &&
           _update_telegraph(boss, false, saw_radius, "Bomb", projectile_kind::FIRE))
        {
            return true;
        }
        break;

    case enemy_id::ROTGRIP:
        _update_frenzy(boss, health_percent, 30, 1, "Rotgrip");
        break;

    case enemy_id::PRINCESS_THERADRAS:
    {
        // Hurls boulders at where the hero stands: one that lands on the hero throws them away from her.
        _boss_greeting(boss, "Theradras: You will be buried!");

        bool landing = boss.telegraph_frames == 1;
        bn::fixed_point landing_spot = boss.special_position;

        if(_update_telegraph(boss, false, smoke_radius, "Boulder", projectile_kind::NATURE))
        {
            return true;
        }

        if(landing && distance_squared(_player.position(), landing_spot) <= smoke_radius * smoke_radius)
        {
            _player.knock_back(boss.position, 48);
            _hud.message("The boulder throws you back!", ui::color::RED);
        }

        _update_frenzy(boss, health_percent, 25, 2, "Theradras");
        break;
    }

    // --- Dire Maul ---------------------------------------------------------------------------------

    case enemy_id::ZEVRIM_THORNHOOF:
        _boss_greeting(boss, "Zevrim: Your blood for the altar!");
        _update_frenzy(boss, health_percent, 30, 2, "Zevrim");
        break;

    case enemy_id::HYDROSPAWN:
        // Splits into two hydrolings at half its health.
        if(boss.phase <= 1 && health_percent <= 50)
        {
            boss.phase = 2;
            _summon_add(boss, enemy_id::HYDROLING);
            _summon_add(boss, enemy_id::HYDROLING);
            _hud.message("Hydrospawn splits!", ui::color::RED);
        }
        break;

    case enemy_id::LETHTENDRIS:
        // Bolts of void at where the hero stands, from two thirds of her health.
        _boss_greeting(boss, "Lethtendris: Caught in my web!");

        if(health_percent <= 66 &&
           _update_telegraph(boss, false, saw_radius, "Void Bolt", projectile_kind::SHADOW))
        {
            return true;
        }
        break;

    case enemy_id::ALZZIN_THE_WILDSHAPER:
        // Calls a lasher out of the Felvine at half health, and rages near the end.
        _boss_greeting(boss, "Alzzin: The Felvine is mine!");

        if(boss.phase == 1 && health_percent <= 50)
        {
            boss.phase = 2;
            _summon_add(boss, enemy_id::WHIP_LASHER);
            _hud.message("Alzzin calls the Felvine!", ui::color::RED);
        }

        _update_frenzy(boss, health_percent, 25, 3, "Alzzin");
        break;

    case enemy_id::TENDRIS_WARPWOOD:
        // Wakes a lasher at two thirds and one third of his health.
        _boss_greeting(boss, "Tendris: The grove will crush you!");

        if((boss.phase == 1 && health_percent <= 66) || (boss.phase == 2 && health_percent <= 33))
        {
            ++boss.phase;
            _summon_add(boss, enemy_id::WHIP_LASHER);
            _hud.message("Tendris wakes the grove!", ui::color::RED);
        }
        break;

    case enemy_id::IMMOL_THAR:
        // Opens an eye at two thirds and one third of its health; maddened near the end.
        _boss_greeting(boss, "Immol'thar roars!");

        if((boss.phase == 1 && health_percent <= 66) || (boss.phase == 2 && health_percent <= 33))
        {
            ++boss.phase;
            _summon_add(boss, enemy_id::EYE_OF_IMMOL_THAR);
            _hud.message("An eye of Immol'thar opens!", ui::color::RED);
        }

        _update_frenzy(boss, health_percent, 20, 4, "Immol'thar");
        break;

    case enemy_id::PRINCE_TORTHELDRIN:
        // Arcane blasts around himself from half his health.
        _boss_greeting(boss, "Tortheldrin: You freed the demon?");

        if(health_percent <= 50 &&
           _update_telegraph(boss, true, flurry_radius, "Arcane Blast", projectile_kind::ARCANE))
        {
            return true;
        }
        break;

    case enemy_id::CHO_RUSH_THE_OBSERVER:
        _boss_greeting(boss, "Cho'Rush: The king sees you.");
        break;

    case enemy_id::KING_GORDOK:
        // Cho'Rush, his adviser, joins the fight.
        if(boss.phase == 0)
        {
            int cho_rush = _find_enemy(enemy_id::CHO_RUSH_THE_OBSERVER);

            if(cho_rush >= 0 && _enemies.at(cho_rush).state == enemy_state::IDLE)
            {
                _enemies.aggro(cho_rush);
            }
        }

        _boss_greeting(boss, "Gordok: Me smash you!");
        _update_frenzy(boss, health_percent, 25, 2, "King Gordok");
        break;

    // --- Uldaman -----------------------------------------------------------------------------------

    case enemy_id::REVELOSH:
        // Calls lightning down on where the hero stands.
        _boss_greeting(boss, "Revelosh: Treasure is mine!");

        if(health_percent <= 80 &&
           _update_telegraph(boss, false, saw_radius, "Chain Lightning", projectile_kind::NATURE))
        {
            return true;
        }
        break;

    case enemy_id::GRIMLOK:
        _boss_greeting(boss, "Grimlok: Smash the stealers!");
        _update_frenzy(boss, health_percent, 30, 2, "Grimlok");
        break;

    case enemy_id::GALGANN_FIREHAMMER:
        // A ring of fire around himself from two thirds of his health.
        _boss_greeting(boss, "Galgann: The relics are ours!");

        if(health_percent <= 66 &&
           _update_telegraph(boss, true, flurry_radius, "Fire Nova", projectile_kind::FIRE))
        {
            return true;
        }
        break;

    case enemy_id::ANCIENT_STONE_KEEPER:
        // Whips up a sand storm where the hero stands.
        _boss_greeting(boss, "The stone keeper stirs!");

        if(_update_telegraph(boss, false, smoke_radius, "Sand Storm", projectile_kind::NATURE))
        {
            return true;
        }
        break;

    case enemy_id::IRONAYA:
    {
        // Stamps the ground around herself: standing in it throws the hero back.
        _boss_greeting(boss, "Ironaya: None may enter!");

        bool landing = boss.telegraph_frames == 1;

        if(_update_telegraph(boss, true, flurry_radius, "Arcing Smash", projectile_kind::NATURE))
        {
            return true;
        }

        if(landing && distance_squared(_player.position(), boss.position) <= flurry_radius * flurry_radius)
        {
            _player.knock_back(boss.position, 48);
            _hud.message("Ironaya throws you back!", ui::color::RED);
        }

        _update_frenzy(boss, health_percent, 25, 2, "Ironaya");
        break;
    }

    case enemy_id::OBSIDIAN_SENTINEL:
        // Sheds a shard of itself at three quarters, half and a quarter of its health.
        _boss_greeting(boss, "The sentinel grinds to life!");

        if((boss.phase == 1 && health_percent <= 75) || (boss.phase == 2 && health_percent <= 50) ||
           (boss.phase == 3 && health_percent <= 25))
        {
            ++boss.phase;
            _summon_add(boss, enemy_id::OBSIDIAN_SHARD);
            _summon_add(boss, enemy_id::OBSIDIAN_SHARD);
            _hud.message("Shards break off the sentinel!", ui::color::RED);
        }
        break;

    case enemy_id::ARCHAEDAS:
        // Wakes more of his guardians at two thirds and one third of his health.
        _boss_greeting(boss, "Archaedas: Who disturbs me?");

        if((boss.phase == 1 && health_percent <= 66) || (boss.phase == 2 && health_percent <= 33))
        {
            ++boss.phase;
            _summon_add(boss, boss.phase == 2 ? enemy_id::EARTHEN_GUARDIAN : enemy_id::VAULT_WARDER);
            _hud.message("Archaedas wakes his guardians!", ui::color::RED);
        }

        if(boss.phase >= 2 && _update_telegraph(boss, true, flurry_radius, "Ground Tremor", projectile_kind::NATURE))
        {
            return true;
        }
        break;

    // --- The Sunken Temple -------------------------------------------------------------------------

    case enemy_id::ATAL_ALARION:
        // Sweeps the floor around himself.
        _boss_greeting(boss, "The altar's guardian wakes!");

        if(_update_telegraph(boss, true, flurry_radius, "Sweeping Slam", projectile_kind::NATURE))
        {
            return true;
        }

        _update_frenzy(boss, health_percent, 25, 2, "Atal'alarion");
        break;

    case enemy_id::JAMMAL_AN_THE_PROPHET:
        // Ogom fights at his side; flames rain on where the hero stands.
        if(boss.phase == 0)
        {
            int ogom = _find_enemy(enemy_id::OGOM_THE_WRETCHED);

            if(ogom >= 0 && _enemies.at(ogom).state == enemy_state::IDLE)
            {
                _enemies.aggro(ogom);
            }
        }

        _boss_greeting(boss, "Jammal'an: Hakkar will rise!");

        if(health_percent <= 80 &&
           _update_telegraph(boss, false, ground_radius, "Flamestrike", projectile_kind::FIRE))
        {
            return true;
        }
        break;

    case enemy_id::OGOM_THE_WRETCHED:
        _boss_greeting(boss, "Ogom: The prophet commands!");
        break;

    case enemy_id::HAZZAS:
        _boss_greeting(boss, "Hazzas: Into the Nightmare!");

        if(_update_breath(boss, "Acid Breath", projectile_kind::NATURE))
        {
            return true;
        }
        break;

    case enemy_id::MORPHAZ:
        _boss_greeting(boss, "Morphaz: Sleep forever!");

        if(_update_breath(boss, "Acid Breath", projectile_kind::NATURE))
        {
            return true;
        }
        break;

    case enemy_id::DREAMSCYTHE:
        _boss_greeting(boss, "Dreamscythe: You dare wake us?");

        if(_update_breath(boss, "Acid Breath", projectile_kind::NATURE))
        {
            return true;
        }

        _update_frenzy(boss, health_percent, 30, 2, "Dreamscythe");
        break;

    case enemy_id::WEAVER:
        _boss_greeting(boss, "Weaver: Your dreams are ours!");

        if(_update_breath(boss, "Frost Breath", projectile_kind::FROST))
        {
            return true;
        }
        break;

    case enemy_id::SHADE_OF_ERANIKUS:
        // Calls whelps out of the Nightmare at two thirds and one third of his health, and breathes
        // on whatever stands in front of him.
        _boss_greeting(boss, "Eranikus: Join the Nightmare!");

        if((boss.phase == 1 && health_percent <= 66) || (boss.phase == 2 && health_percent <= 33))
        {
            ++boss.phase;
            _summon_add(boss, enemy_id::NIGHTMARE_WHELP);
            _summon_add(boss, enemy_id::NIGHTMARE_WHELP);
            _hud.message("Whelps pour out of the dream!", ui::color::RED);
        }

        if(_update_breath(boss, "Nightmare Breath", projectile_kind::SHADOW))
        {
            return true;
        }
        break;

    // --- Blackrock Depths --------------------------------------------------------------------------

    case enemy_id::LORD_ROCCOR:
        // The floor bursts into flame where the hero stands.
        _boss_greeting(boss, "Roccor: Burn, intruder!");

        if(_update_telegraph(boss, false, saw_radius, "Ground Tremor", projectile_kind::FIRE))
        {
            return true;
        }
        break;

    case enemy_id::HIGH_INTERROGATOR_GERSTAHN:
        _boss_greeting(boss, "Gerstahn: Back in your cell!");
        _update_frenzy(boss, health_percent, 30, 2, "Gerstahn");
        break;

    case enemy_id::GOROSH_THE_DERVISH:
        _boss_greeting(boss, "Gorosh: Spin to win!");

        if(_update_telegraph(boss, true, flurry_radius, "Whirlwind", projectile_kind::ARROW))
        {
            return true;
        }
        break;

    case enemy_id::GRIZZLE:
        _boss_greeting(boss, "Grizzle roars into the ring!");
        _update_frenzy(boss, health_percent, 40, 2, "Grizzle");
        break;

    case enemy_id::HEDRUM_THE_CREEPER:
        _boss_greeting(boss, "Hedrum creeps into the ring!");

        if(_update_telegraph(boss, false, saw_radius, "Poison Spit", projectile_kind::NATURE))
        {
            return true;
        }
        break;

    case enemy_id::OK_THOR_THE_BREAKER:
        _boss_greeting(boss, "Ok'thor: Me break you!");

        if(_update_telegraph(boss, false, ground_radius, "Arcane Blast", projectile_kind::ARCANE))
        {
            return true;
        }
        break;

    case enemy_id::GOLEM_LORD_ARGELMACH:
        // Wakes a golem of his hall at two thirds and one third of his health.
        _boss_greeting(boss, "Argelmach: Golems, crush them!");

        if((boss.phase == 1 && health_percent <= 66) || (boss.phase == 2 && health_percent <= 33))
        {
            ++boss.phase;
            _summon_add(boss, enemy_id::RAGEREAVER_GOLEM);
            _hud.message("Argelmach wakes a golem!", ui::color::RED);
        }
        break;

    case enemy_id::GENERAL_ANGERFORGE:
        // Calls his reservists at a third of his health.
        _boss_greeting(boss, "Angerforge: To arms!");

        if(boss.phase == 1 && health_percent <= 33)
        {
            boss.phase = 2;
            _summon_add(boss, enemy_id::ANVILRAGE_RESERVIST);
            _summon_add(boss, enemy_id::ANVILRAGE_RESERVIST);
            _hud.message("Angerforge: Reservists, to me!", ui::color::RED);
        }
        break;

    case enemy_id::AMBASSADOR_FLAMELASH:
        // Burning spirits climb out of the lava at three quarters, half and a quarter of his health.
        _boss_greeting(boss, "Flamelash: Ragnaros sees you!");

        if((boss.phase == 1 && health_percent <= 75) || (boss.phase == 2 && health_percent <= 50) ||
           (boss.phase == 3 && health_percent <= 25))
        {
            ++boss.phase;
            _summon_add(boss, enemy_id::BURNING_SPIRIT);
            _summon_add(boss, enemy_id::BURNING_SPIRIT);
            _hud.message("Burning spirits rise!", ui::color::RED);
        }
        break;

    case enemy_id::MAGMUS:
        // Fire bursts from the floor where the hero stands.
        _boss_greeting(boss, "Magmus: None pass the gate!");

        if(_update_telegraph(boss, false, smoke_radius, "Fiery Burst", projectile_kind::FIRE))
        {
            return true;
        }

        _update_frenzy(boss, health_percent, 25, 2, "Magmus");
        break;

    case enemy_id::EMPEROR_DAGRAN_THAURISSAN:
        // Moira fights at his side and heals him; fire rains on where the hero stands.
        if(boss.phase == 0)
        {
            int moira = _find_enemy(enemy_id::PRINCESS_MOIRA_BRONZEBEARD);

            if(moira >= 0 && _enemies.at(moira).state == enemy_state::IDLE)
            {
                _enemies.aggro(moira);
            }
        }

        _boss_greeting(boss, "Thaurissan: Kneel before me!");

        if(_update_telegraph(boss, false, ground_radius, "Hand of Thaurissan", projectile_kind::FIRE))
        {
            return true;
        }

        _update_frenzy(boss, health_percent, 25, 2, "Thaurissan");
        break;

    // --- The Plaguelands ---------------------------------------------------------------------------

    case enemy_id::ARAJ_THE_SUMMONER:
        // Raises a skeleton from Andorhal's dead at half health.
        _boss_greeting(boss, "Araj: Rise, my servants!");

        if(boss.phase == 1 && health_percent <= 50)
        {
            boss.phase = 2;
            _summon_add(boss, enemy_id::SKELETAL_FLAYER);
            _hud.message("Araj raises the dead!", ui::color::RED);
        }
        break;

    case enemy_id::GRAND_INQUISITOR_ISILLIEN:
        _boss_greeting(boss, "Isillien: Burn, heretic!");

        if(_update_telegraph(boss, false, saw_radius, "Holy Fire", projectile_kind::HOLY))
        {
            return true;
        }
        break;

    case enemy_id::HED_MUSH_THE_ROTTING:
        // A cloud of rot around him: the hero steps out of it.
        if(_update_telegraph(boss, true, flurry_radius, "Rot Cloud", projectile_kind::NATURE))
        {
            return true;
        }
        break;

    case enemy_id::CRUSADER_LORD_VALDELMAR:
        if(_update_wind_up(index, in_melee, "Crusader Strike", "Valdelmar smites you!"))
        {
            return true;
        }
        break;

    // --- Blackrock Spire ---------------------------------------------------------------------------

    case enemy_id::HIGHLORD_OMOKK:
        if(_update_wind_up(index, in_melee, "Smash", "Omokk smashes you!"))
        {
            return true;
        }
        break;

    case enemy_id::WAR_MASTER_VOONE:
        // Throws axes at where the hero stands.
        _boss_greeting(boss, "Voone: Me smash you!");

        if(_update_telegraph(boss, false, saw_radius, "Throw Axe", projectile_kind::ARROW))
        {
            return true;
        }
        break;

    case enemy_id::MOTHER_SMOLDERWEB:
        // Her brood crawls out of the webs at two thirds and one third of her health.
        _boss_greeting(boss, "Mother Smolderweb hisses!");

        if((boss.phase == 1 && health_percent <= 66) || (boss.phase == 2 && health_percent <= 33))
        {
            ++boss.phase;
            _summon_add(boss, enemy_id::SPIRE_SPIDERLING);
            _summon_add(boss, enemy_id::SPIRE_SPIDERLING);
            _hud.message("Spiderlings swarm out!", ui::color::RED);
        }
        break;

    case enemy_id::OVERLORD_WYRMTHALAK:
        // Calls two veterans from the stairs at half health.
        _boss_greeting(boss, "Wyrmthalak: You dare?");

        if(boss.phase == 1 && health_percent <= 50)
        {
            boss.phase = 2;
            _summon_add(boss, enemy_id::BLACKHAND_VETERAN);
            _summon_add(boss, enemy_id::BLACKHAND_VETERAN);
            _hud.message("Wyrmthalak: Guards, to me!", ui::color::RED);
        }
        break;

    case enemy_id::PYROGUARD_EMBERSEER:
        // Flames burst around him; free at last, he rages near the end.
        _boss_greeting(boss, "Emberseer burns with fury!");

        if(_update_telegraph(boss, true, flurry_radius, "Fire Nova", projectile_kind::FIRE))
        {
            return true;
        }

        _update_frenzy(boss, health_percent, 25, 2, "Emberseer");
        break;

    case enemy_id::THE_BEAST:
        _boss_greeting(boss, "The Beast roars!");

        if(_update_breath(boss, "Flame Break", projectile_kind::FIRE))
        {
            return true;
        }

        _update_frenzy(boss, health_percent, 30, 2, "The Beast");
        break;

    case enemy_id::GENERAL_DRAKKISATH:
        // Breathes fire in front of him; whelps fly in at two thirds and one third of his health.
        _boss_greeting(boss, "Drakkisath: You face a god!");

        if((boss.phase == 1 && health_percent <= 66) || (boss.phase == 2 && health_percent <= 33))
        {
            ++boss.phase;
            _summon_add(boss, enemy_id::CHROMATIC_WHELP);
            _summon_add(boss, enemy_id::CHROMATIC_WHELP);
            _hud.message("Whelps answer Drakkisath!", ui::color::RED);
        }

        if(_update_breath(boss, "Flamestrike", projectile_kind::FIRE))
        {
            return true;
        }
        break;

    // --- Scholomance -------------------------------------------------------------------------------

    case enemy_id::JANDICE_BAROV:
        // Splits into illusions at two thirds and one third of her health.
        _boss_greeting(boss, "Jandice: Which one is real?");

        if((boss.phase == 1 && health_percent <= 66) || (boss.phase == 2 && health_percent <= 33))
        {
            ++boss.phase;
            _summon_add(boss, enemy_id::ILLUSION_OF_JANDICE);
            _summon_add(boss, enemy_id::ILLUSION_OF_JANDICE);
            _effects.burst(boss.position, projectile_kind::ARCANE);
            _hud.message("Jandice splits apart!", ui::color::RED);
        }
        break;

    case enemy_id::RATTLEGORE:
        if(_update_wind_up(index, in_melee, "Bone Smash", "Rattlegore crushes you!"))
        {
            return true;
        }
        break;

    case enemy_id::RAS_FROSTWHISPER:
        _boss_greeting(boss, "Ras: Your bones will freeze!");

        if(_update_telegraph(boss, false, ground_radius, "Blizzard", projectile_kind::FROST))
        {
            return true;
        }
        break;

    case enemy_id::INSTRUCTOR_MALICIA:
        _boss_greeting(boss, "Malicia: Class is in session!");

        if(_update_telegraph(boss, false, saw_radius, "Shadow Bolt", projectile_kind::SHADOW))
        {
            return true;
        }
        break;

    case enemy_id::DOCTOR_THEOLEN_KRASTINOV:
        _boss_greeting(boss, "Krastinov: Fresh specimens!");
        _update_frenzy(boss, health_percent, 30, 2, "Krastinov");
        break;

    case enemy_id::LOREKEEPER_POLKELT:
        if(_update_telegraph(boss, false, saw_radius, "Volatile Infection", projectile_kind::NATURE))
        {
            return true;
        }
        break;

    case enemy_id::LORD_ALEXEI_BAROV:
    case enemy_id::LADY_ILLUCIA_BAROV:
    {
        // The Barovs fight together: waking one wakes the other.
        bool alexei = boss.id == enemy_id::LORD_ALEXEI_BAROV;

        if(boss.phase == 0)
        {
            int other = _find_enemy(alexei ? enemy_id::LADY_ILLUCIA_BAROV : enemy_id::LORD_ALEXEI_BAROV);

            if(other >= 0 && _enemies.at(other).state == enemy_state::IDLE)
            {
                _enemies.aggro(other);
            }
        }

        _boss_greeting(boss, alexei ? "Alexei: Our land, our dead!" : "Illucia: Kneel, peasant!");

        if(_update_telegraph(boss, ! alexei, alexei ? saw_radius : flurry_radius,
                             alexei ? "Unholy Aura" : "Shadow Shock", projectile_kind::SHADOW))
        {
            return true;
        }
        break;
    }

    case enemy_id::DARKMASTER_GANDLING:
        // Shadow Portal: sends the hero to another corner of his study and raises students around
        // them, then waits a while before the next.
        if(boss.phase == 0)
        {
            boss.phase = 1;
            boss.special_timer = 10 * seconds;
            _hud.message("Gandling: School is out!", ui::color::RED);
        }

        if(boss.special_timer == 0)
        {
            _gandling_portal();
            boss.special_timer = 18 * seconds;
        }

        if(boss.phase == 1 && health_percent <= 25)
        {
            boss.phase = 2;
            _hud.message("Gandling: You will not leave!", ui::color::RED);
        }
        break;

    // --- Stratholme --------------------------------------------------------------------------------

    case enemy_id::TIMMY_THE_CRUEL:
        _boss_greeting(boss, "Timmy: TIMMY!");
        _update_frenzy(boss, health_percent, 50, 2, "Timmy");
        break;

    case enemy_id::MALOR_THE_ZEALOUS:
        if(_update_wind_up(index, in_melee, "Holy Strike", "Malor strikes you down!"))
        {
            return true;
        }
        break;

    case enemy_id::BALNAZZAR:
        // Dathrohan's mask slips at half health.
        _boss_greeting(boss, "Dathrohan: The Light guides!");

        if(boss.phase == 1 && health_percent <= 50)
        {
            boss.phase = 2;
            _effects.burst(boss.position, projectile_kind::SHADOW);
            _hud.message("Dathrohan is Balnazzar!", ui::color::RED);
        }

        if(boss.phase >= 2 &&
           _update_telegraph(boss, false, ground_radius, "Psychic Scream", projectile_kind::SHADOW))
        {
            return true;
        }
        break;

    case enemy_id::MALEKI_THE_PALLID:
        if(_update_telegraph(boss, false, ground_radius, "Ice Tomb", projectile_kind::FROST))
        {
            return true;
        }
        break;

    case enemy_id::RAMSTEIN_THE_GORGER:
        if(_update_wind_up(index, in_melee, "Trample", "Ramstein tramples you!"))
        {
            return true;
        }
        break;

    case enemy_id::EMBERSTRIFE:
        _boss_greeting(boss, "Emberstrife roars!");

        if(_update_breath(boss, "Flame Breath", projectile_kind::FIRE))
        {
            return true;
        }
        break;

    case enemy_id::ONYXIA:
        return _update_onyxia(index, health_percent);

    case enemy_id::BARON_RIVENDARE:
        // Raises bone minions at three quarters, half and a quarter of his health.
        _boss_greeting(boss, "Rivendare: Kneel to the Lich!");

        if((boss.phase == 1 && health_percent <= 75) || (boss.phase == 2 && health_percent <= 50) ||
           (boss.phase == 3 && health_percent <= 25))
        {
            ++boss.phase;
            _summon_add(boss, enemy_id::BONE_MINION);
            _summon_add(boss, enemy_id::BONE_MINION);
            _hud.message("Rivendare raises the dead!", ui::color::RED);
        }
        break;

    default:
        break;
    }

    return false;
}

bool combat::_update_onyxia(int index, int health_percent)
{
    namespace ol = map_data::onyxias_lair;
    enemy& boss = _enemies.at(index);

    // Three phases. On the ground she breathes fire in front of her. From 65% health she takes to the
    // air for a while, out of reach: she breathes fire down whole lanes of the cave and her whelps
    // pour out of the nests at the sides. Then she lands, the floor erupts under the hero and she
    // grows frenzied near the end.
    _boss_greeting(boss, "Onyxia: You dare enter here?");

    if(boss.phase == 1)
    {
        if(health_percent > 65)
        {
            return _update_breath(boss, "Flame Breath", projectile_kind::FIRE);
        }

        boss.phase = 2;
        boss.airborne = true;
        boss.telegraph_frames = 0;
        boss.special_timer = 4 * seconds;
        _air_frames = onyxia_air_frames;
        _whelp_frames = 2 * seconds;
        _lane_vertical = false;

        if(boss.casting())
        {
            stop_enemy_cast(index, false);
        }

        _hud.message("Onyxia takes to the air!", ui::color::RED);
    }

    if(boss.phase == 2)
    {
        // Flies up over the middle of the cave and hangs there.
        bn::fixed_point lift(ol::lift.x, ol::lift.y);
        bn::fixed dx = bn::clamp(lift.x() - boss.position.x(), bn::fixed(-2), bn::fixed(2));
        bn::fixed dy = bn::clamp(lift.y() - boss.position.y(), bn::fixed(-2), bn::fixed(2));
        boss.position = bn::fixed_point(boss.position.x() + dx, boss.position.y() + dy);
        boss.moving = dx != 0 || dy != 0;
        boss.direction = facing_towards(boss.position, _player.position());

        if(--_whelp_frames <= 0)
        {
            _whelp_frames = 15 * seconds;

            const point_def nests[] = { ol::whelps_a, ol::whelps_b };

            for(const point_def& nest : nests)
            {
                bn::fixed_point position(nest.x, nest.y);

                if(_enemies.summon(enemy_id::ONYXIAN_WHELP, position) >= 0)
                {
                    _effects.burst(position, projectile_kind::FIRE);
                }
            }

            _hud.message("Whelps pour from the nests!", ui::color::RED);
        }

        _update_deep_breath(boss);

        if(--_air_frames <= 0)
        {
            boss.phase = 3;
            boss.airborne = false;
            boss.telegraph_frames = 0;
            boss.special_timer = 3 * seconds;
            _hud.message("Onyxia lands!", ui::color::RED);
        }

        return true;
    }

    // Back on the ground: Bellowing Roar cracks the floor under the hero.
    _update_frenzy(boss, health_percent, 25, 4, "Onyxia");
    return _update_telegraph(boss, false, ground_radius, "Bellowing Roar", projectile_kind::FIRE);
}

void combat::_update_deep_breath(enemy& boss)
{
    // Deep Breath: a lane of fire across the cave through where the hero stands, north to south or
    // west to east in turn. It goes off after the usual wind-up: step out of the lane.
    constexpr int lane_circles = 5;
    constexpr int lane_step = 52;

    if(boss.telegraph_frames > 0)
    {
        if(--boss.telegraph_frames > 0)
        {
            return;
        }

        bool hit = false;

        for(int index = 0; index < lane_circles; ++index)
        {
            int offset = (index - lane_circles / 2) * lane_step;
            bn::fixed_point spot = _lane_vertical ?
                        bn::fixed_point(boss.special_position.x(), boss.special_position.y() + offset) :
                        bn::fixed_point(boss.special_position.x() + offset, boss.special_position.y());
            _effects.burst(spot, projectile_kind::FIRE);
            hit = hit || distance_squared(_player.position(), spot) <= breath_radius * breath_radius;
        }

        if(hit)
        {
            damage_player(boss.damage * 4, boss.special_position, school::FIRE);
        }

        _lane_vertical = ! _lane_vertical;
        boss.special_timer = 6 * seconds;
        return;
    }

    if(boss.special_timer == 0)
    {
        boss.special_position = _player.position();
        boss.telegraph_frames = telegraph_windup;

        for(int index = 0; index < lane_circles; ++index)
        {
            int offset = (index - lane_circles / 2) * lane_step;
            bn::fixed_point spot = _lane_vertical ?
                        bn::fixed_point(boss.special_position.x(), boss.special_position.y() + offset) :
                        bn::fixed_point(boss.special_position.x() + offset, boss.special_position.y());
            _effects.circle(spot, breath_radius, telegraph_windup, circle_style::DANGER);
        }

        _texts.show(_head(boss.position, 52), "Deep Breath", floating_texts::style::DAMAGE_TAKEN);
    }
}

void combat::_gandling_portal()
{
    namespace sc = map_data::scholomance;
    constexpr point_def corners[] = { sc::gandling_a, sc::gandling_b, sc::gandling_c, sc::gandling_d };
    const point_def* best = &corners[0];
    int best_distance = -1;

    for(const point_def& corner : corners)
    {
        int d = distance_squared(bn::fixed_point(corner.x, corner.y), _player.position());

        if(d > best_distance)
        {
            best = &corner;
            best_distance = d;
        }
    }

    bn::fixed_point position(best->x, best->y);
    _effects.burst(_player.position(), projectile_kind::SHADOW);
    _player.set_position(position);
    _effects.burst(position, projectile_kind::SHADOW);
    _hud.message("A Shadow Portal takes you!", ui::color::RED);

    for(int side = -1; side <= 1; side += 2)
    {
        bn::fixed_point student(position.x() + side * 32, position.y() + 16);

        if(_enemies.fits(student.x(), student.y()) && _enemies.summon(enemy_id::RISEN_STUDENT, student) >= 0)
        {
            _effects.burst(student, projectile_kind::SHADOW);
        }
    }
}

void combat::_boss_killed(const enemy& boss)
{
    _event_boss_killed(boss);

    switch(boss.id)
    {

    case enemy_id::HEROD:
        // The trainees he drilled run in from the training grounds to avenge him.
        for(const point_def& door : world::map().patrol)
        {
            _enemies.summon(enemy_id::SCARLET_TRAINEE, bn::fixed_point(door.x, door.y));
        }

        _hud.message("The trainees rush in!", ui::color::RED);
        break;

    case enemy_id::KING_GORDOK:
        // The Gordok left standing bow to the hero who killed their king, and leave.
        for(int index = 0, limit = _enemies.count(); index < limit; ++index)
        {
            const enemy& item = _enemies.at(index);

            if(item.alive() && item.def->family == enemy_family::OGRE)
            {
                _effects.burst(item.position, projectile_kind::ARCANE);
                _enemies.remove(index);
            }
        }

        _hud.message("The Gordok bow to their new king!", ui::color::GREEN);
        break;

    case enemy_id::EMPEROR_DAGRAN_THAURISSAN:
    {
        // His spell breaks with him: Moira lowers her hands and leaves the fight.
        int moira = _find_enemy(enemy_id::PRINCESS_MOIRA_BRONZEBEARD);

        if(moira >= 0 && _enemies.at(moira).alive())
        {
            _effects.burst(_enemies.at(moira).position, projectile_kind::HOLY);
            _enemies.remove(moira);
            _hud.message("Moira: What have I done?", ui::color::GREEN);
        }
        break;
    }

    case enemy_id::BARON_RIVENDARE:
        // In time, Ysida Harmon is saved from the slaughterhouse.
        if(_baron_frames > 0)
        {
            _baron_frames = 0;
            _baron_over = true;
            _hud.message("Ysida Harmon is saved!", ui::color::GREEN);

            if(quests_on_rescue(enemy_id::BARON_RIVENDARE, _hud) && on_quest_progress)
            {
                on_quest_progress(callback_context);
            }
        }
        break;

    case enemy_id::SCARLET_COMMANDER_MOGRAINE:
        // Whitemane stays at the altar, praying for him: the hero gets to catch a breath before her.
        if(! boss.revived)
        {
            int whitemane = _find_enemy(enemy_id::HIGH_INQUISITOR_WHITEMANE);

            if(whitemane >= 0 && _enemies.at(whitemane).state == enemy_state::IDLE)
            {
                _hud.message("Whitemane: Mograine, no!", ui::color::RED);
            }
        }
        break;

    default:
        break;
    }
}

int combat::_find_enemy(enemy_id id) const
{
    for(int index = 0, limit = _enemies.count(); index < limit; ++index)
    {
        if(_enemies.at(index).id == id)
        {
            return index;
        }
    }

    return -1;
}

void combat::_launch_bomb()
{
    const auto& hatches = world::map().patrol;
    int bombs = 0;

    for(int index = 0, limit = _enemies.count(); index < limit; ++index)
    {
        const enemy& item = _enemies.at(index);
        bombs += item.summoned && item.alive() && item.id == enemy_id::WALKING_BOMB;
    }

    if(hatches.empty() || bombs >= max_bombs)
    {
        return;
    }

    const point_def& hatch = hatches[random_range(0, hatches.size() - 1)];
    bn::fixed_point position(hatch.x, hatch.y);

    if(_enemies.summon(enemy_id::WALKING_BOMB, position) >= 0)
    {
        _effects.burst(position, projectile_kind::FIRE);
        _hud.message("A Walking Bomb is coming!", ui::color::RED);
    }
}

}
