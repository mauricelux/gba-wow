#include "gw_combat.h"

#include "bn_math.h"
#include "bn_string.h"

#include "gw_enemies.h"
#include "gw_floating_text.h"
#include "gw_hud.h"
#include "gw_player.h"
#include "gw_world.h"

namespace gw
{

// Enemy abilities from the shared table (gw_enemy_abilities): when an enemy uses them, the cast bar
// and red circle that warn the player, what happens when they land, and the debuffs they leave on
// the player.

namespace
{
    constexpr int seconds = 60;
    constexpr int once_per_fight = 0x7FFF;      // the cooldown of an ability used once a fight
    constexpr int interrupt_lockout = 4 * seconds;
    constexpr int flee_frames = 6 * seconds;
    constexpr int flee_percent = 15;
    constexpr int flee_search = 160;            // how far a runner looks for friends
    constexpr int call_friends = 3;             // at most this many answer a call
    constexpr int charge_min = 40;
    constexpr int charge_frames = 60;
    constexpr int charge_speed = 3;
    constexpr int max_sunder_stacks = 5;

    constexpr const char* debuff_names[] = {
        "Chilled", "Rooted", "Stunned", "Asleep", "Polymorphed", "Feared", "Weakened", "Sundered", "Wounded",
        "Bleeding", "Poisoned", "Diseased", "Burning"
    };

    static_assert(sizeof(debuff_names) / sizeof(debuff_names[0]) == enemy_debuff_count, "a name per debuff");

    [[nodiscard]] projectile_kind school_burst(school damage_school)
    {
        switch(damage_school)
        {

        case school::FIRE:
            return projectile_kind::FIRE;

        case school::FROST:
            return projectile_kind::FROST;

        case school::ARCANE:
            return projectile_kind::ARCANE;

        case school::NATURE:
            return projectile_kind::NATURE;

        case school::SHADOW:
            return projectile_kind::SHADOW;

        case school::HOLY:
            return projectile_kind::HOLY;

        default:
            return projectile_kind::NONE;
        }
    }

    [[nodiscard]] bool controls(buff_id debuff)
    {
        return debuff == buff_id::STUNNED || debuff == buff_id::ASLEEP || debuff == buff_id::POLYMORPHED ||
               debuff == buff_id::FEARED;
    }
}

int combat::_enemy_swing(const enemy& item) const
{
    // Enrage and Battle Shout make every hit harder.
    int percent = 100 + item.ai.rally_percent;

    if(item.ai.enraged)
    {
        percent += get_enemy_ability(enemy_ability_id::ENRAGE).value;
    }

    return item.damage * percent / 100;
}

bool combat::_enemy_sees_player(const enemy& item) const
{
    return world::line_clear(item.position.x().integer(), item.position.y().integer() - 4,
                             _player.position().x().integer(), _player.position().y().integer() - 4);
}

int combat::_hurt_friend(int index, int range, int below) const
{
    // Itself or whoever fights beside it with the least health, at or under below percent.
    const enemy& healer = _enemies.at(index);
    int best = -1;
    int best_percent = below + 1;

    for(int other = 0, limit = _enemies.count(); other < limit; ++other)
    {
        const enemy& item = _enemies.at(other);

        if(item.state != enemy_state::CHASE ||
           (other != index && distance_squared(item.position, healer.position) > range * range))
        {
            continue;
        }

        int percent = item.health_percent();

        if(percent < best_percent)
        {
            best = other;
            best_percent = percent;
        }
    }

    return best;
}

bool combat::enemy_ai_update(int index)
{
    enemy& item = _enemies.at(index);
    enemy_ability_state& ai = item.ai;

    for(int16_t& cooldown : ai.cooldowns)
    {
        if(cooldown > 0)
        {
            --cooldown;
        }
    }

    if(ai.flee_frames > 0)
    {
        _update_flee(index);
        return true;
    }

    if(ai.charging)
    {
        _update_enemy_charge(index);
        return true;
    }

    if(ai.casting != enemy_ability_id::NONE)
    {
        item.moving = false;
        item.direction = facing_towards(item.position, _player.position());

        if(--ai.cast_frames > 0)
        {
            return true;
        }

        // The circle times out by itself as the cast ends.
        enemy_ability_id ability = ai.casting;
        ai.casting = enemy_ability_id::NONE;
        ai.circle = 0;
        _enemy_ability_goes_off(index, ability);
        return true;
    }

    // Runners run for help when nearly dead, once a fight.
    if(item.def->style == ai_style::RUNNER && ! ai.fled && item.health_percent() <= flee_percent)
    {
        _start_flee(index);
        return true;
    }

    int player_distance_squared = distance_squared(item.position, _player.position());

    for(int slot = 0; slot < enemy_ability_slots; ++slot)
    {
        if(_enemy_can_use(index, slot, player_distance_squared))
        {
            return _start_enemy_ability(index, slot);
        }
    }

    return false;
}

bool combat::_enemy_can_use(int index, int slot, int player_distance_squared) const
{
    const enemy& item = _enemies.at(index);
    enemy_ability_id ability = item.def->abilities[slot];

    if(ability == enemy_ability_id::NONE || item.ai.cooldowns[slot] > 0)
    {
        return false;
    }

    const enemy_ability_def& def = get_enemy_ability(ability);

    if((def.flags & enemy_ability_flag::SPELL) && item.silence_frames > 0)
    {
        return false;
    }

    if(def.health_below && def.effect != enemy_effect::HEAL && item.health_percent() > def.health_below)
    {
        return false;
    }

    if((def.flags & enemy_ability_flag::SWING) && item.attack_timer > 0)
    {
        return false;
    }

    int range = def.range;
    bool in_reach = player_distance_squared <= range * range;

    switch(def.effect)
    {

    case enemy_effect::HIT:
        // Nothing that would wake a sleeping player.
        if(player_incapacitated())
        {
            return false;
        }

        if(def.target == enemy_target::AROUND_SELF)
        {
            return player_distance_squared <= (def.radius - 4) * (def.radius - 4);
        }

        return in_reach && (range <= melee_range || _enemy_sees_player(item));

    case enemy_effect::HEAL:
        return _hurt_friend(index, range, def.health_below) >= 0;

    case enemy_effect::CHARGE:
        return ! player_incapacitated() && in_reach && player_distance_squared >= charge_min * charge_min &&
               _enemy_sees_player(item);

    case enemy_effect::BLINK:
        return player_distance_squared <= melee_range * melee_range;

    case enemy_effect::CALL_FOR_HELP:
        return _enemies.idle_friend(index, def.radius) >= 0;

    case enemy_effect::SUMMON:
        return def.summon != enemy_id::NONE;

    case enemy_effect::ENRAGE:
        return ! item.ai.enraged;

    case enemy_effect::RALLY:
        return item.ai.rally_frames <= 0;

    case enemy_effect::GUARD:
        return item.ai.guard_frames <= 0 && (! range || in_reach);

    case enemy_effect::EVASION:
        return item.ai.evasion_frames <= 0;

    case enemy_effect::ABSORB:
        return item.ai.absorb <= 0;

    default:
        return false;
    }
}

bool combat::_start_enemy_ability(int index, int slot)
{
    enemy& item = _enemies.at(index);
    enemy_ability_state& ai = item.ai;
    enemy_ability_id ability = item.def->abilities[slot];
    const enemy_ability_def& def = get_enemy_ability(ability);
    int height = item.sprite ? item.sprite->height() : 24;

    ai.cooldowns[slot] = int16_t((def.flags & enemy_ability_flag::ONCE) ? once_per_fight : def.cooldown * seconds);
    ai.cast_slot = int8_t(slot);
    item.moving = false;
    item.direction = facing_towards(item.position, _player.position());

    if(def.effect == enemy_effect::HEAL)
    {
        ai.heal_target = int16_t(_hurt_friend(index, def.range, def.health_below));
    }

    // Its name over its head, in red: something is coming.
    _texts.show(_head(item.position, height + 6), def.name, floating_texts::style::DAMAGE_TAKEN);

    if(def.cast_time == 0)
    {
        _enemy_ability_goes_off(index, ability);
        return true;
    }

    // A cast: a bar over its head, and a red circle where an area will land.
    ai.casting = ability;
    ai.cast_total = int16_t(def.cast_time * seconds / 10);
    ai.cast_frames = ai.cast_total;

    if(def.target != enemy_target::PLAYER)
    {
        ai.cast_position = def.target == enemy_target::AROUND_SELF ? item.position : _player.position();
        ai.circle = _effects.circle(ai.cast_position, def.radius, ai.cast_total, circle_style::DANGER);
    }

    return true;
}

void combat::stop_enemy_cast(int index, bool interrupted)
{
    enemy& item = _enemies.at(index);
    enemy_ability_state& ai = item.ai;

    if(ai.casting == enemy_ability_id::NONE)
    {
        return;
    }

    if(ai.circle)
    {
        _effects.remove_circle(ai.circle);
    }

    if(interrupted)
    {
        // As in WoW, the interrupted ability can't be cast again for a few seconds.
        if(ai.cast_slot >= 0)
        {
            ai.cooldowns[ai.cast_slot] = int16_t(bn::max(int(ai.cooldowns[ai.cast_slot]), interrupt_lockout));
        }

        int height = item.sprite ? item.sprite->height() : 24;
        _texts.show(_head(item.position, height), "Interrupted", floating_texts::style::INFO);
    }

    ai.casting = enemy_ability_id::NONE;
    ai.cast_frames = 0;
    ai.circle = 0;
}

void combat::_enemy_ability_goes_off(int index, enemy_ability_id ability)
{
    enemy& item = _enemies.at(index);
    enemy_ability_state& ai = item.ai;
    const enemy_ability_def& def = get_enemy_ability(ability);
    int height = item.sprite ? item.sprite->height() : 24;
    int damage = _enemy_swing(item) * def.value / 100;

    switch(def.effect)
    {

    case enemy_effect::HIT:
        if(def.flags & enemy_ability_flag::SWING)
        {
            _enemy_strike(index, ability);
        }
        else if(def.target != enemy_target::PLAYER)
        {
            // Whoever stayed in the red circle.
            _effects.burst(ai.cast_position, school_burst(def.damage_school));

            if(distance_squared(_player.position(), ai.cast_position) <= def.radius * def.radius)
            {
                _enemy_ability_lands(index, ability, damage, ai.cast_position);
            }
        }
        else if(def.projectile != projectile_kind::NONE)
        {
            projectile_hit hit{ player_target, damage, false, ability_id::NONE };
            hit.enemy_ability = ability;
            hit.caster = index;
            _effects.launch(_head(item.position, height / 2), def.projectile, hit);
        }
        else
        {
            _effects.burst(_head(_player.position(), 10), school_burst(def.damage_school));
            _enemy_ability_lands(index, ability, damage, item.position);
        }
        break;

    case enemy_effect::HEAL:
    {
        int target = ai.heal_target;

        if(target >= 0 && target < _enemies.count() && _enemies.at(target).alive())
        {
            enemy& healed = _enemies.at(target);
            int amount = bn::min(healed.max_health * def.value / 100, healed.max_health - healed.health);
            healed.health += amount;
            _effects.burst(_head(healed.position, 10), projectile_kind::HOLY);
            _texts.show_number(_head(healed.position, healed.sprite ? healed.sprite->height() : 24), amount,
                               floating_texts::style::HEAL);
        }
        break;
    }

    case enemy_effect::CHARGE:
        ai.charging = true;
        ai.cast_frames = charge_frames;
        break;

    case enemy_effect::BLINK:
        _enemy_blink(index, def.range);
        break;

    case enemy_effect::CALL_FOR_HELP:
        _call_for_help(index, def.radius);
        break;

    case enemy_effect::SUMMON:
        _summon_add(item, def.summon);
        break;

    case enemy_effect::ENRAGE:
    {
        ai.enraged = true;
        bn::string<48> text = item.def->name;
        text += " is enraged!";
        _hud.message(text, ui::color::RED);
        break;
    }

    case enemy_effect::RALLY:
        // It and its friends fighting nearby.
        for(int other = 0, limit = _enemies.count(); other < limit; ++other)
        {
            enemy& ally = _enemies.at(other);

            if(ally.state == enemy_state::CHASE && ally.def->family == item.def->family &&
               distance_squared(ally.position, item.position) <= def.radius * def.radius)
            {
                ally.ai.rally_frames = int16_t(def.duration * seconds);
                ally.ai.rally_percent = def.value;
            }
        }
        break;

    case enemy_effect::GUARD:
        ai.guard_frames = int16_t(def.duration * seconds);
        ai.guard_percent = def.value;
        ai.guard_physical = def.flags & enemy_ability_flag::PHYSICAL;
        break;

    case enemy_effect::EVASION:
        ai.evasion_frames = int16_t(def.duration * seconds);
        ai.evasion_percent = def.value;
        break;

    case enemy_effect::ABSORB:
        ai.absorb = int16_t(item.max_health * def.value / 100);
        break;

    default:
        break;
    }
}

void combat::_enemy_strike(int index, enemy_ability_id ability)
{
    // In place of a swing; Thrash swings again right away.
    enemy& item = _enemies.at(index);
    const enemy_ability_def& def = get_enemy_ability(ability);
    item.attack_timer = item.swing_frames();

    if(item.sprite)
    {
        item.sprite->play_attack();
    }

    for(int hit = 0; hit < bn::max(1, int(def.hits)); ++hit)
    {
        if(_dead || ! item.alive())
        {
            return;
        }

        if(enemy_attacks(index, def.value) && def.debuff != buff_id::COUNT)
        {
            _enemy_debuff(index, ability, item.position);
        }
    }
}

void combat::_enemy_ability_lands(int caster, enemy_ability_id ability, int damage, const bn::fixed_point& from)
{
    const enemy_ability_def& def = get_enemy_ability(ability);

    if(damage > 0)
    {
        damage_player(damage, from, def.damage_school);
    }

    if(def.debuff != buff_id::COUNT)
    {
        _enemy_debuff(caster, ability, from);
    }
}

void combat::_enemy_debuff(int caster, enemy_ability_id ability, const bn::fixed_point& from)
{
    const enemy_ability_def& def = get_enemy_ability(ability);
    int value = def.debuff_value;

    // Over time: a share of the caster's swing every three seconds.
    if(is_periodic(def.debuff))
    {
        value = bn::max(1, _enemy_swing(_enemies.at(caster)) * def.debuff_value / 100);
    }

    _apply_debuff(def.debuff, def.duration * seconds, value, from);
}

void combat::_update_enemy_charge(int index)
{
    enemy& item = _enemies.at(index);
    enemy_ability_state& ai = item.ai;
    bool there = _enemies.move_towards(item, _player.position(), charge_speed, melee_range - 8);

    if(! there && --ai.cast_frames > 0 && item.stuck_frames < 6)
    {
        return;
    }

    ai.charging = false;
    item.moving = false;

    if(there && ai.cast_slot >= 0)
    {
        _enemy_strike(index, item.def->abilities[ai.cast_slot]);
    }
}

void combat::_start_flee(int index)
{
    enemy& item = _enemies.at(index);
    enemy_ability_state& ai = item.ai;
    stop_enemy_cast(index, false);
    ai.fled = true;
    ai.flee_frames = flee_frames;
    ai.flee_friend = int16_t(_enemies.idle_friend(index, flee_search));

    bn::string<48> text = item.def->name;
    text += " runs for help!";
    _hud.message(text, ui::color::YELLOW);
    _texts.show(_head(item.position, item.sprite ? item.sprite->height() : 24), "Flee",
                floating_texts::style::INFO);
}

void combat::_update_flee(int index)
{
    // Runs to the friend it saw, a little slower than it chases so the player can catch it; without
    // one, just away. Then back to the fight.
    enemy& item = _enemies.at(index);
    enemy_ability_state& ai = item.ai;
    int friend_index = ai.flee_friend;
    bool has_friend = friend_index >= 0 && friend_index < _enemies.count() &&
                      _enemies.at(friend_index).state == enemy_state::IDLE;
    const bn::fixed_point& player_feet = _player.position();
    bn::fixed_point target = has_friend ? _enemies.at(friend_index).position :
            bn::fixed_point(item.position.x() * 2 - player_feet.x(), item.position.y() * 2 - player_feet.y());
    --ai.flee_frames;

    if(item.root_frames <= 0)
    {
        _enemies.move_towards(item, target, _enemies.chase_speed(item) * 3 / 4, has_friend ? 12 : 0);
    }
    else
    {
        item.moving = false;
    }

    if(has_friend && distance_squared(item.position, target) <= 24 * 24)
    {
        _enemies.aggro(friend_index);
        ai.flee_frames = 0;
    }

    if(ai.flee_frames <= 0)
    {
        ai.flee_frames = 0;
        item.attack_timer = bn::max(item.attack_timer, 30);
    }
}

void combat::_call_for_help(int index, int radius)
{
    enemy& item = _enemies.at(index);
    int called = 0;

    for(; called < call_friends; ++called)
    {
        int friend_index = _enemies.idle_friend(index, radius);

        if(friend_index < 0)
        {
            break;
        }

        _enemies.aggro(friend_index);
    }

    if(called)
    {
        bn::string<48> text = item.def->name;
        text += " calls for help!";
        _hud.message(text, ui::color::YELLOW);
    }
}

void combat::_enemy_blink(int index, int distance)
{
    // Straight away from the player, as far as the ground allows.
    enemy& item = _enemies.at(index);
    bn::fixed dx = item.position.x() - _player.position().x();
    bn::fixed dy = item.position.y() - _player.position().y();
    bn::fixed length = bn::sqrt(dx * dx + dy * dy);

    if(length < 1)
    {
        length = 1;
    }

    bn::fixed_point from = item.position;

    for(int step = 3; step > 0; --step)
    {
        bn::fixed reach = distance * step / 3;
        bn::fixed_point to(item.position.x() + dx * reach / length, item.position.y() + dy * reach / length);

        if(_enemies.fits(to.x(), to.y()) &&
           world::line_clear(from.x().integer(), from.y().integer() - 4, to.x().integer(), to.y().integer() - 4))
        {
            item.position = to;
            break;
        }
    }

    _effects.burst(_head(from, 8), projectile_kind::ARCANE);
    _effects.burst(_head(item.position, 8), projectile_kind::ARCANE);
}

void combat::_apply_debuff(buff_id debuff, int frames, int value, const bn::fixed_point& from)
{
    if(_dead || _buffs[int(buff_id::ICE_BLOCK)])
    {
        return;
    }

    if(debuff == buff_id::SUNDERED && _buffs[int(debuff)])
    {
        value = bn::min(_buff_values[int(debuff)] + value, value * max_sunder_stacks);
    }

    if(debuff == buff_id::FEARED)
    {
        _fear_from = from;
    }

    // Losing control stops a cast.
    if(controls(debuff) && _cast_ability != ability_id::NONE)
    {
        _cast_ability = ability_id::NONE;
        _player.sprite().set_casting(false);
        _update_channel();
        _hud.message("Interrupted", ui::color::RED);
    }

    _texts.show(_head(_player.position(), 34), debuff_names[int(debuff) - int(buff_id::CHILLED)],
                floating_texts::style::INFO);
    _set_buff(debuff, frames, value);
}

bool combat::_controlled() const
{
    return _buffs[int(buff_id::STUNNED)] || _buffs[int(buff_id::FEARED)] || player_incapacitated();
}

const char* combat::_control_reason(ability_id ability) const
{
    // Blink and Ice Block get out of a stun, Berserker Rage out of fear and sleep.
    if(_buffs[int(buff_id::STUNNED)] && ability != ability_id::BLINK && ability != ability_id::ICE_BLOCK)
    {
        return "You are stunned";
    }

    if(_buffs[int(buff_id::POLYMORPHED)])
    {
        return "You are polymorphed";
    }

    if(ability != ability_id::BERSERKER_RAGE)
    {
        if(_buffs[int(buff_id::ASLEEP)])
        {
            return "You are asleep";
        }

        if(_buffs[int(buff_id::FEARED)])
        {
            return "You are feared";
        }
    }

    if(_buffs[int(buff_id::ROOTED)] && ability == ability_id::CHARGE)
    {
        return "You can't move";
    }

    return nullptr;
}

void combat::_break_free(ability_id ability)
{
    switch(ability)
    {

    case ability_id::BLINK:
        _end_buff(buff_id::ROOTED);
        _end_buff(buff_id::STUNNED);
        break;

    case ability_id::BERSERKER_RAGE:
        _end_buff(buff_id::FEARED);
        _end_buff(buff_id::ASLEEP);
        break;

    case ability_id::ICE_BLOCK:
        for(int index = 0; index < int(buff_id::COUNT); ++index)
        {
            if(is_debuff(buff_id(index)))
            {
                _buffs[index] = 0;
            }
        }

        refresh_stats();
        break;

    default:
        break;
    }
}

void combat::_periodic_tick(buff_id debuff)
{
    school damage_school = debuff == buff_id::BURNING ? school::FIRE :
            debuff == buff_id::BLEEDING ? school::PHYSICAL : school::NATURE;
    damage_player(_buff_values[int(debuff)], _player.position(), damage_school);
}

}
