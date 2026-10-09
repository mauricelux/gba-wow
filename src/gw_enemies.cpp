#include "gw_enemies.h"

#include "bn_affine_mat_attributes.h"
#include "bn_math.h"
#include "bn_sprite_items_fx_icons.h"
#include "bn_sprite_items_fx_markers.h"

#include "gw_abilities.h"
#include "gw_character.h"
#include "gw_combat.h"
#include "gw_loot.h"
#include "gw_world.h"

namespace gw
{

namespace
{
    constexpr int seconds = 60;
    constexpr int corpse_frames = 30 * seconds;
    constexpr int looted_corpse_frames = 2 * seconds;
    constexpr int leash_distance = 300;
    constexpr int dot_interval = 3 * seconds;
    constexpr int awake_distance = 260;
    constexpr int social_distance = 40;
    constexpr int wander_radius = 40;

    constexpr int sprite_margin_x = 150;
    constexpr int sprite_margin_y = 112;

    constexpr int sparkle_first_frame = 3;   // fx_markers: two twinkle frames
    constexpr int sparkle_bg_priority = 1;

    constexpr int hitbox_left = -4;
    constexpr int hitbox_right = 3;
    constexpr int hitbox_top = -4;

    [[nodiscard]] bool fits(bn::fixed x, bn::fixed y)
    {
        int feet_x = x.floor_integer();
        int feet_y = y.floor_integer();
        return world::area_free(feet_x + hitbox_left, feet_y + hitbox_top, feet_x + hitbox_right, feet_y);
    }
}

bool enemies::fits(bn::fixed x, bn::fixed y)
{
    return gw::fits(x, y);
}

bool enemy::has_loot() const
{
    if(loot_money)
    {
        return true;
    }

    for(const loot_slot& slot : loot)
    {
        if(slot.item != item_id::NONE)
        {
            return true;
        }
    }

    return false;
}

enemies::enemies(const bn::camera_ptr& camera) :
    _camera(camera)
{
}

void enemies::load(const map_info& map)
{
    _enemies.clear();

    for(const spawn_def& spawn : map.spawns)
    {
        if(_enemies.full())
        {
            break;
        }

        enemy& item = _enemies.emplace_back();
        item.id = spawn.enemy;
        item.def = &get_enemy_def(spawn.enemy);
        item.spawn = bn::fixed_point(spawn.x, spawn.y);
        _spawn(item);
    }
}

void enemies::_spawn(enemy& item)
{
    const enemy_def& def = *item.def;
    item.level = random_range(def.min_level, def.max_level);
    item.max_health = enemy_base_health(item.level) * def.health_percent / 100;
    item.health = item.max_health;
    item.damage = enemy_base_damage(item.level) * def.damage_percent / 100;
    item.position = item.spawn;
    item.wander_target = item.spawn;
    item.state = enemy_state::IDLE;
    item.state_timer = random_range(30, 240);
    item.direction = facing(random_range(0, 3));
    item.moving = false;
    item.wandering = false;
    item.tapped = false;
    item.stun_frames = 0;
    item.root_frames = 0;
    item.slow_frames = 0;
    item.dot_ticks = 0;
    item.marked_frames = 0;
    item.incapacitate_frames = 0;
    item.incapacitated = incapacitate_kind::NONE;
    item.sting_damage = 0;
    item.fear_frames = 0;
    item.weaken_frames = 0;
    item.disarm_frames = 0;
    item.sunder_frames = 0;
    item.sunder_stacks = 0;
    item.scorch_frames = 0;
    item.scorch_stacks = 0;
    item.silence_frames = 0;
    item.phase = 0;
    item.special_timer = 0;
    item.telegraph_frames = 0;
    item.loot_money = 0;

    for(loot_slot& slot : item.loot)
    {
        slot = loot_slot();
    }
}

void enemies::update(const bn::fixed_point& player_feet, bool player_alive)
{
    ++_frame;

    for(int index = 0, limit = _enemies.size(); index < limit; ++index)
    {
        enemy& item = _enemies[index];
        _update_enemy(index, player_feet, player_alive);
        _update_sprite(item, player_feet);
    }

    // Keep chasing enemies from stacking on the same pixel.
    for(int a = 0, limit = _enemies.size(); a < limit; ++a)
    {
        enemy& first = _enemies[a];

        if(first.state != enemy_state::CHASE)
        {
            continue;
        }

        for(int b = a + 1; b < limit; ++b)
        {
            enemy& second = _enemies[b];

            if(second.state != enemy_state::CHASE)
            {
                continue;
            }

            bn::fixed dx = second.position.x() - first.position.x();
            bn::fixed dy = second.position.y() - first.position.y();

            if(bn::abs(dx) < 10 && bn::abs(dy) < 6)
            {
                bn::fixed push = dx >= 0 ? bn::fixed(0.5) : bn::fixed(-0.5);

                if(fits(second.position.x() + push, second.position.y()))
                {
                    second.position.set_x(second.position.x() + push);
                }

                if(fits(first.position.x() - push, first.position.y()))
                {
                    first.position.set_x(first.position.x() - push);
                }
            }
        }
    }
}

bn::fixed enemies::_speed(const enemy& item, bool chasing) const
{
    bn::fixed speed = chasing ? bn::fixed(1.1) : bn::fixed(0.4);

    if(chasing && (item.def->flags & enemy_flag::FAST))
    {
        speed = 1.35;
    }

    if(item.slow_frames > 0)
    {
        speed = speed * (100 - item.slow_percent) / 100;
    }

    return speed;
}

int enemies::_aggro_radius(const enemy& item) const
{
    if(item.def->flags & enemy_flag::PASSIVE || is_gray(item.level))
    {
        return 0;
    }

    int radius = 44 + 4 * (int(item.level) - int(character().level));
    return bn::clamp(radius, 16, 76);
}

bool enemies::_move_towards(enemy& item, const bn::fixed_point& target, bn::fixed speed, int stop_distance)
{
    bn::fixed dx = target.x() - item.position.x();
    bn::fixed dy = target.y() - item.position.y();
    bn::fixed adx = bn::abs(dx);
    bn::fixed ady = bn::abs(dy);

    if(adx <= stop_distance && ady <= stop_distance)
    {
        item.moving = false;
        return true;
    }

    bn::fixed step_x = 0;
    bn::fixed step_y = 0;

    if(adx > 1)
    {
        step_x = dx > 0 ? speed : -speed;
    }

    if(ady > 1)
    {
        step_y = dy > 0 ? speed : -speed;
    }

    if(step_x != 0 && step_y != 0)
    {
        step_x *= bn::fixed(0.7071);
        step_y *= bn::fixed(0.7071);
    }

    bool moved = false;

    if(step_x != 0 && fits(item.position.x() + step_x, item.position.y()))
    {
        item.position.set_x(item.position.x() + step_x);
        moved = true;
    }

    if(step_y != 0 && fits(item.position.x(), item.position.y() + step_y))
    {
        item.position.set_y(item.position.y() + step_y);
        moved = true;
    }

    item.moving = moved;
    item.stuck_frames = moved ? 0 : item.stuck_frames + 1;

    if(moved)
    {
        ++item.walk_counter;
        item.direction = facing_towards(bn::fixed_point(0, 0), bn::fixed_point(dx, dy));
    }

    return false;
}

void enemies::_update_enemy(int index, const bn::fixed_point& player_feet, bool player_alive)
{
    enemy& item = _enemies[index];

    if(item.state == enemy_state::GONE)
    {
        if(item.summoned || (item.def->flags & enemy_flag::NO_RESPAWN))
        {
            return;
        }

        if(--item.state_timer <= 0 && distance_squared(item.spawn, player_feet) > 200 * 200)
        {
            _spawn(item);
        }

        return;
    }

    if(item.state == enemy_state::DEAD)
    {
        if(--item.state_timer <= 0)
        {
            item.state = enemy_state::GONE;
            item.state_timer = item.def->respawn_seconds * seconds;
        }

        return;
    }

    int player_distance_squared = distance_squared(item.position, player_feet);

    if(item.state == enemy_state::IDLE && player_distance_squared > awake_distance * awake_distance)
    {
        return;
    }

    // Debuffs tick down whatever the enemy is doing.
    if(item.slow_frames > 0)
    {
        --item.slow_frames;
    }

    if(item.marked_frames > 0)
    {
        --item.marked_frames;
    }

    if(item.weaken_frames > 0)
    {
        --item.weaken_frames;
    }

    if(item.disarm_frames > 0)
    {
        --item.disarm_frames;
    }

    if(item.silence_frames > 0)
    {
        --item.silence_frames;
    }

    if(item.sunder_frames > 0 && --item.sunder_frames == 0)
    {
        item.sunder_stacks = 0;
    }

    if(item.scorch_frames > 0 && --item.scorch_frames == 0)
    {
        item.scorch_stacks = 0;
    }

    if(item.dot_ticks > 0 && --item.dot_timer <= 0)
    {
        item.dot_timer = dot_interval;
        --item.dot_ticks;

        if(_combat->damage_enemy(index, item.dot_damage, false, true))
        {
            return;
        }
    }

    if(item.stun_frames > 0)
    {
        --item.stun_frames;
        item.moving = false;
        return;
    }

    if(item.incapacitate_frames > 0)
    {
        if(--item.incapacitate_frames == 0)
        {
            break_control(index);
        }

        item.moving = false;
        return;
    }

    if(item.fear_frames > 0 && item.state == enemy_state::CHASE)
    {
        // Runs straight away from the player.
        --item.fear_frames;
        bn::fixed_point away(item.position.x() * 2 - player_feet.x(), item.position.y() * 2 - player_feet.y());
        _move_towards(item, away, _speed(item, false) * 2, 0);
        return;
    }

    switch(item.state)
    {

    case enemy_state::IDLE:
    {
        int radius = _aggro_radius(item);

        if(player_alive && radius && player_distance_squared < radius * radius &&
           world::line_clear(item.position.x().integer(), item.position.y().integer() - 4,
                             player_feet.x().integer(), player_feet.y().integer() - 4))
        {
            aggro(index);
            break;
        }

        if(item.wandering)
        {
            if(_move_towards(item, item.wander_target, _speed(item, false), 1) || item.stuck_frames > 30)
            {
                item.wandering = false;
                item.moving = false;
                item.state_timer = random_range(120, 360);
            }
        }
        else if(--item.state_timer <= 0 && ! item.boss())
        {
            // Bosses hold their ground, so their guards can be pulled one at a time.
            bn::fixed_point target(item.spawn.x() + random_range(-wander_radius, wander_radius),
                                   item.spawn.y() + random_range(-wander_radius, wander_radius));

            if(fits(target.x(), target.y()))
            {
                item.wander_target = target;
                item.wandering = true;
            }
            else
            {
                item.state_timer = 60;
            }
        }
        break;
    }

    case enemy_state::CHASE:
    {
        if(! player_alive || distance_squared(item.position, item.spawn) > leash_distance * leash_distance)
        {
            item.state = enemy_state::EVADE;
            item.stuck_frames = 0;
            break;
        }

        if(item.attack_timer > 0)
        {
            --item.attack_timer;
        }

        if(item.elite() && _combat->boss_update(index))
        {
            break;
        }

        bool in_range = player_distance_squared <= (melee_range - 6) * (melee_range - 6);

        if(! in_range && item.root_frames <= 0)
        {
            _move_towards(item, player_feet, _speed(item, true), melee_range - 8);
        }
        else
        {
            item.moving = false;
        }

        if(item.root_frames > 0)
        {
            --item.root_frames;
        }

        if(player_distance_squared <= melee_range * melee_range)
        {
            item.direction = facing_towards(item.position, player_feet);

            if(item.attack_timer <= 0)
            {
                item.attack_timer = item.def->attack_speed * 6;

                if(item.sprite)
                {
                    item.sprite->play_attack();
                }

                _combat->enemy_attacks(index);
            }
        }
        break;
    }

    case enemy_state::EVADE:
    {
        item.health = item.max_health;
        item.dot_ticks = 0;
        item.slow_frames = 0;
        item.root_frames = 0;
        item.incapacitate_frames = 0;
        item.sting_damage = 0;
        item.fear_frames = 0;
        item.weaken_frames = 0;
        item.disarm_frames = 0;
        item.sunder_frames = 0;
        item.sunder_stacks = 0;
        item.scorch_frames = 0;
        item.scorch_stacks = 0;

        if(_move_towards(item, item.spawn, 2, 2) || item.stuck_frames > 90)
        {
            item.position = item.spawn;
            item.state = enemy_state::IDLE;
            item.state_timer = random_range(60, 180);
            item.moving = false;
            item.tapped = false;
            item.phase = 0;
            item.special_timer = 0;
            item.telegraph_frames = 0;

            if(item.summoned)
            {
                item.state = enemy_state::GONE;
            }
        }
        break;
    }

    default:
        break;
    }
}

void enemies::_update_sprite(enemy& item, const bn::fixed_point& player_feet)
{
    bool near = bn::abs(item.position.x() - player_feet.x()) < sprite_margin_x &&
                bn::abs(item.position.y() - player_feet.y()) < sprite_margin_y;

    if(! near || item.state == enemy_state::GONE)
    {
        item.sprite.reset();
        item.sparkle.reset();
        item.status.reset();
        return;
    }

    if(! item.sprite)
    {
        if(! actor_sprite::can_create(item.def->look))
        {
            return;
        }

        item.sprite.emplace(item.def->look, _camera, bn::fixed(item.def->scale_percent) / 100);
    }

    item.sprite->set_dead(item.state == enemy_state::DEAD);
    item.sprite->update(item.position, item.direction, item.moving, item.walk_counter);
    _update_status(item);

    if(item.state != enemy_state::DEAD || ! item.has_loot())
    {
        item.sparkle.reset();
        return;
    }

    // A twinkle over corpses that still have loot.
    int frame = sparkle_first_frame + ((_frame >> 4) & 1);

    if(! item.sparkle)
    {
        item.sparkle = bn::sprite_items::fx_markers.create_sprite(0, 0, frame);
        item.sparkle->set_camera(_camera);
        item.sparkle->set_bg_priority(sparkle_bg_priority);
    }
    else
    {
        item.sparkle->set_tiles(bn::sprite_items::fx_markers.tiles_item(), frame);
    }

    bn::fixed_point screen = world::to_screen_space(item.position);
    item.sparkle->set_position(screen.x().floor_integer() + 6, screen.y().floor_integer() - 10);
}

void enemies::_update_status(enemy& item)
{
    icon_id icon = icon_id::COUNT;

    if(item.alive() && item.incapacitate_frames > 0)
    {
        switch(item.incapacitated)
        {

        case incapacitate_kind::POLYMORPH:
            icon = icon_id::POLYMORPH;
            break;

        case incapacitate_kind::FROZEN:
            icon = icon_id::FREEZING_TRAP;
            break;

        case incapacitate_kind::DISORIENTED:
            icon = icon_id::SCATTER_SHOT;
            break;

        default:
            icon = icon_id::WYVERN_STING;
            break;
        }
    }
    else if(item.alive() && item.fear_frames > 0)
    {
        icon = icon_id::INTIMIDATING_SHOUT;
    }

    if(icon == icon_id::COUNT)
    {
        item.status.reset();
        return;
    }

    // A small icon over the head saying what holds it.
    if(! _small)
    {
        bn::affine_mat_attributes attributes;
        attributes.set_scale(0.5);
        _small = bn::sprite_affine_mat_ptr::create(attributes);
    }

    if(! item.status)
    {
        item.status = bn::sprite_items::fx_icons.create_sprite(0, 0, int(icon));
        item.status->set_camera(_camera);
        item.status->set_bg_priority(sparkle_bg_priority);
        item.status->set_affine_mat(*_small);
    }
    else
    {
        item.status->set_tiles(bn::sprite_items::fx_icons.tiles_item(), int(icon));
    }

    bn::fixed_point screen = world::to_screen_space(item.position);
    item.status->set_position(screen.x().floor_integer(), screen.y().floor_integer() - item.sprite->height() - 6);
}

int enemies::nearest(const bn::fixed_point& from, int max_distance, int exclude, bool fighting_only) const
{
    int best = -1;
    int best_distance = max_distance * max_distance + 1;

    for(int index = 0, limit = _enemies.size(); index < limit; ++index)
    {
        const enemy& item = _enemies[index];

        if(index == exclude || ! item.alive() || item.state == enemy_state::EVADE ||
           (fighting_only && item.state != enemy_state::CHASE))
        {
            continue;
        }

        int d = distance_squared(from, item.position);

        if(d < best_distance)
        {
            best = index;
            best_distance = d;
        }
    }

    return best;
}

int enemies::nearest_corpse(const bn::fixed_point& from, int max_distance) const
{
    int best = -1;
    int best_distance = max_distance * max_distance + 1;

    for(int index = 0, limit = _enemies.size(); index < limit; ++index)
    {
        const enemy& item = _enemies[index];

        if(item.state != enemy_state::DEAD || ! item.has_loot())
        {
            continue;
        }

        int d = distance_squared(from, item.position);

        if(d < best_distance)
        {
            best = index;
            best_distance = d;
        }
    }

    return best;
}

void enemies::aggro(int index)
{
    enemy& item = _enemies[index];

    if(item.state != enemy_state::IDLE)
    {
        return;
    }

    item.state = enemy_state::CHASE;
    item.attack_timer = 24;
    item.wandering = false;

    // Humanoids call one friend of the same kind standing close by.
    if(get_look(item.def->look).creature && item.def->look != look_id::KOBOLD_VERMIN &&
       item.def->look != look_id::KOBOLD_TUNNELER)
    {
        return;
    }

    for(int other = 0, limit = _enemies.size(); other < limit; ++other)
    {
        enemy& friend_enemy = _enemies[other];

        if(other != index && friend_enemy.id == item.id && friend_enemy.state == enemy_state::IDLE &&
           ! (friend_enemy.def->flags & enemy_flag::PASSIVE) &&
           distance_squared(friend_enemy.position, item.position) < social_distance * social_distance)
        {
            friend_enemy.state = enemy_state::CHASE;
            friend_enemy.attack_timer = 40;
            friend_enemy.wandering = false;
            return;
        }
    }
}

bool enemies::damage(int index, int amount)
{
    enemy& item = _enemies[index];

    if(! item.alive() || item.state == enemy_state::EVADE)
    {
        return false;
    }

    item.tapped = true;

    if(item.state == enemy_state::IDLE)
    {
        aggro(index);
    }

    if(item.sprite)
    {
        item.sprite->flash();
    }

    // Damage breaks every hold, and scares the fear out of it.
    if(item.incapacitate_frames > 0)
    {
        break_control(index);
    }

    item.fear_frames = 0;
    item.health -= amount;

    if(item.health <= 0)
    {
        _die(index);
        return true;
    }

    return false;
}

bool enemies::incapacitate(int index, incapacitate_kind kind, int frames)
{
    enemy& item = _enemies[index];

    if(! item.alive() || item.state == enemy_state::EVADE || item.boss())
    {
        return false;
    }

    if(item.state == enemy_state::IDLE)
    {
        aggro(index);
    }

    item.incapacitate_frames = frames;
    item.incapacitated = kind;
    item.fear_frames = 0;
    item.moving = false;
    return true;
}

void enemies::break_control(int index)
{
    enemy& item = _enemies[index];
    item.incapacitate_frames = 0;

    if(item.incapacitated == incapacitate_kind::ASLEEP && item.sting_damage > 0)
    {
        // Wyvern Sting's poison works over 12 seconds once the target wakes.
        item.dot_ticks = 4;
        item.dot_damage = bn::max(1, item.sting_damage / 4);
        item.dot_timer = dot_interval;
        item.sting_damage = 0;
    }

    item.incapacitated = incapacitate_kind::NONE;
}

void enemies::_die(int index)
{
    enemy& item = _enemies[index];
    item.health = 0;
    item.state = enemy_state::DEAD;
    item.state_timer = corpse_frames;
    item.moving = false;
    item.dot_ticks = 0;
    item.incapacitate_frames = 0;
    item.incapacitated = incapacitate_kind::NONE;
    item.sting_damage = 0;
    item.fear_frames = 0;

    if(item.tapped)
    {
        roll_loot(item);
    }

    _combat->enemy_killed(index);
}

int enemies::summon(enemy_id id, const bn::fixed_point& position)
{
    // Reuse a dead summon's slot before taking a new one.
    int slot = -1;

    for(int index = 0, limit = _enemies.size(); index < limit; ++index)
    {
        if(_enemies[index].summoned && _enemies[index].state == enemy_state::GONE)
        {
            slot = index;
            break;
        }
    }

    if(slot < 0)
    {
        if(_enemies.full())
        {
            return -1;
        }

        _enemies.emplace_back();
        slot = _enemies.size() - 1;
    }

    enemy& item = _enemies[slot];
    item.id = id;
    item.def = &get_enemy_def(id);
    item.spawn = position;
    item.summoned = true;
    item.sprite.reset();
    _spawn(item);
    aggro(slot);
    return slot;
}

bool enemies::any_in_combat() const
{
    for(const enemy& item : _enemies)
    {
        if(item.state == enemy_state::CHASE)
        {
            return true;
        }
    }

    return false;
}

bool enemies::elite_in_combat() const
{
    for(const enemy& item : _enemies)
    {
        if(item.state == enemy_state::CHASE && item.elite())
        {
            return true;
        }
    }

    return false;
}

void enemies::reset_combat()
{
    for(enemy& item : _enemies)
    {
        if(item.state == enemy_state::CHASE)
        {
            item.state = enemy_state::EVADE;
            item.stuck_frames = 0;
        }
    }
}

void enemies::corpse_looted(int index)
{
    enemy& item = _enemies[index];

    if(item.state == enemy_state::DEAD && item.state_timer > looted_corpse_frames)
    {
        item.state_timer = looted_corpse_frames;
    }
}

}
