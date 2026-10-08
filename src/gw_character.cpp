#include "gw_character.h"

#include "bn_math.h"

#include "gw_map_elwynn.h"
#include "gw_talents.h"

namespace gw
{

namespace
{
    character_data data;

    // Experience per level: classic values scaled down for a handheld session.
    constexpr int xp_table[max_level] = {
        160, 360, 560, 840, 1120, 1440, 1800, 2160, 2600, 3040,
        3520, 4040, 4560, 5160, 5760, 6400, 7080, 7760, 8520, 0
    };

    struct class_base
    {
        int strength;
        int agility;
        int stamina;
        int intellect;
        int spirit;
        // Gains per ten levels.
        int strength_gain;
        int agility_gain;
        int stamina_gain;
        int intellect_gain;
        int spirit_gain;
        int health;
        int health_per_level;
        int mana;
        int mana_per_level;
    };

    constexpr class_base class_bases[] = {
        { 23, 20, 22, 20, 20, 20, 12, 18, 2, 4, 40, 15, 0, 0 },         // warrior
        { 20, 20, 20, 23, 22, 2, 3, 6, 20, 18, 30, 9, 80, 22 },         // mage
        { 20, 23, 21, 20, 21, 6, 22, 12, 8, 9, 36, 12, 60, 16 },        // hunter
    };

    struct race_bonus
    {
        int strength;
        int agility;
        int stamina;
        int intellect;
        int spirit;
    };

    constexpr race_bonus race_bonuses[] = {
        { 0, 0, 0, 0, 2 },          // human
        { 2, -4, 3, -1, -1 },       // dwarf
        { -3, 5, 0, 0, 0 },         // night elf
    };

    [[nodiscard]] int gain(int base, int per_ten, int level)
    {
        return base + per_ten * (level - 1) / 10;
    }

    [[nodiscard]] int weapon_frames(int tenths)
    {
        return tenths * 6;
    }
}

character_data& character()
{
    return data;
}

void new_character(race_id race, class_id player_class)
{
    data = character_data();
    data.race = race;
    data.player_class = player_class;
    data.map = map_id::ELWYNN;
    data.x = map_data::elwynn::start.x;
    data.y = map_data::elwynn::start.y;

    auto equip = [](item_id item)
    {
        data.equipment[int(get_item(item).slot)] = item;
    };

    switch(player_class)
    {

    case class_id::MAGE:
        equip(item_id::BENT_STAFF);
        equip(item_id::APPRENTICES_ROBE);
        equip(item_id::APPRENTICES_PANTS);
        equip(item_id::APPRENTICES_BOOTS);
        learn_ability(ability_id::FIREBALL);
        learn_ability(ability_id::FROST_ARMOR);
        break;

    case class_id::HUNTER:
        equip(item_id::WORN_HATCHET);
        equip(item_id::CRACKED_SHORTBOW);
        equip(item_id::TRAPPERS_VEST);
        equip(item_id::TRAPPERS_PANTS);
        equip(item_id::TRAPPERS_BOOTS);
        learn_ability(ability_id::RAPTOR_STRIKE);
        break;

    default:
        equip(item_id::WORN_SHORTSWORD);
        equip(item_id::RECRUITS_VEST);
        equip(item_id::RECRUITS_PANTS);
        equip(item_id::RECRUITS_BOOTS);
        learn_ability(ability_id::HEROIC_STRIKE);
        break;
    }

    add_item(item_id::TOUGH_JERKY, 4);

    if(player_class == class_id::MAGE)
    {
        add_item(item_id::SPRING_WATER, 4);
    }

    stats s = compute_stats();
    data.health = s.max_health;
    data.power = uses_mana() ? s.max_power : 0;
}

bool has_flag(story_flag flag)
{
    return data.flags & (1u << int(flag));
}

void set_flag(story_flag flag)
{
    data.flags |= 1u << int(flag);
}

bool knows_ability(ability_id ability)
{
    return data.known_abilities & (1u << int(ability));
}

void learn_ability(ability_id ability)
{
    if(knows_ability(ability))
    {
        return;
    }

    data.known_abilities |= 1u << int(ability);

    for(ability_id& slot : data.action_bar)
    {
        if(slot == ability_id::NONE)
        {
            slot = ability;
            return;
        }
    }
}

int xp_for_level(int level)
{
    if(level < 1 || level >= max_level)
    {
        return 0;
    }

    return xp_table[level - 1];
}

namespace
{
    [[nodiscard]] int zero_difference(int level)
    {
        if(level <= 7)
        {
            return 5;
        }

        if(level <= 9)
        {
            return 6;
        }

        if(level <= 11)
        {
            return 7;
        }

        if(level <= 15)
        {
            return 8;
        }

        return 9;
    }
}

bool is_gray(int enemy_level)
{
    return data.level - enemy_level >= zero_difference(data.level);
}

int kill_xp(int enemy_level, bool elite)
{
    if(data.level >= max_level || is_gray(enemy_level))
    {
        return 0;
    }

    int base = 45 + 5 * data.level;
    int difference = enemy_level - data.level;
    int xp;

    if(difference >= 0)
    {
        xp = base * (100 + 5 * bn::min(difference, 4)) / 100;
    }
    else
    {
        int zd = zero_difference(data.level);
        xp = base * (zd + difference) / zd;
    }

    return elite ? xp * 2 : xp;
}

bool uses_mana()
{
    return data.player_class != class_id::WARRIOR;
}

stats compute_stats(const stat_bonus& bonus)
{
    const class_base& base = class_bases[int(data.player_class)];
    const race_bonus& race = race_bonuses[int(data.race)];
    int level = data.level;

    stats s = {};
    s.strength = gain(base.strength, base.strength_gain, level) + race.strength;
    s.agility = gain(base.agility, base.agility_gain, level) + race.agility;
    s.stamina = gain(base.stamina, base.stamina_gain, level) + race.stamina;
    s.intellect = gain(base.intellect, base.intellect_gain, level) + race.intellect;
    s.spirit = gain(base.spirit, base.spirit_gain, level) + race.spirit;

    int weapon_min = 1;
    int weapon_max = 2;
    int weapon_speed = 20;
    bool two_handed = false;

    for(item_id equipped : data.equipment)
    {
        if(equipped == item_id::NONE)
        {
            continue;
        }

        const item_def& item = get_item(equipped);
        s.armor += item.armor;
        s.strength += item.strength;
        s.agility += item.agility;
        s.stamina += item.stamina;
        s.intellect += item.intellect;
        s.spirit += item.spirit;

        if(item.slot == equip_slot::MAIN_HAND)
        {
            weapon_min = item.min_damage;
            weapon_max = item.max_damage;
            weapon_speed = item.speed;
            two_handed = is_two_handed(item);
        }
        else if(item.slot == equip_slot::RANGED)
        {
            s.ranged_min = item.min_damage;
            s.ranged_max = item.max_damage;
            s.ranged_speed = weapon_frames(item.speed);
            s.has_ranged = true;
        }
    }

    (void) two_handed;
    talent_bonus talent = talent_bonuses();
    s.strength += s.strength * talent.strength_percent / 100;
    s.stamina += s.stamina * talent.stamina_percent / 100;
    s.intellect += s.intellect * talent.intellect_percent / 100;

    s.max_health = base.health + base.health_per_level * (level - 1) + 20 + bn::max(0, s.stamina - 20) * 10;
    s.max_health += s.max_health * (talent.health_percent + bonus.health_percent) / 100;

    if(data.player_class != class_id::WARRIOR)
    {
        s.max_power = base.mana + base.mana_per_level * (level - 1) + bn::max(0, s.intellect - 20) * 15;
        s.max_power += s.max_power * talent.mana_percent / 100;
    }
    else
    {
        s.max_power = 100;
    }

    s.armor += s.agility * 2 + bonus.armor;
    s.armor += s.armor * talent.armor_percent / 100;

    switch(data.player_class)
    {

    case class_id::WARRIOR:
        s.attack_power = s.strength * 2 - 20;
        s.ranged_attack_power = s.agility - 10;
        break;

    case class_id::HUNTER:
        s.attack_power = s.strength + s.agility - 20;
        s.ranged_attack_power = s.agility * 2 - 20;
        break;

    default:
        s.attack_power = s.strength - 10;
        s.ranged_attack_power = 0;
        break;
    }

    s.attack_power += bonus.attack_power + talent.attack_power;
    s.ranged_attack_power += bonus.ranged_attack_power + talent.ranged_attack_power;

    s.melee_speed = weapon_frames(weapon_speed);
    int ap_damage = s.attack_power * weapon_speed / 140;
    s.melee_min = weapon_min + ap_damage;
    s.melee_max = weapon_max + ap_damage;

    if(s.has_ranged)
    {
        int ranged_ap_damage = s.ranged_attack_power * (s.ranged_speed / 6) / 140;
        s.ranged_min += ranged_ap_damage;
        s.ranged_max += ranged_ap_damage;
    }

    int haste = bonus.haste_percent + talent.haste_percent;

    if(haste)
    {
        s.melee_speed = s.melee_speed * 100 / (100 + haste);
        s.ranged_speed = s.ranged_speed * 100 / (100 + haste);
    }

    s.crit = 5 + s.agility / 20 + talent.crit;
    s.spell_crit = 5 + s.intellect / 30 + talent.spell_crit;
    s.dodge = 5 + s.agility / 20 + talent.dodge;
    s.damage_percent = 100 + bonus.damage_percent + talent.damage_percent;
    s.spell_power = level * 2 + talent.spell_power;
    s.health_regen = bn::max(1, s.max_health / 25 + s.spirit / 5);
    s.power_regen = data.player_class == class_id::WARRIOR ? 2 : s.spirit / 4 + 12 + s.max_power / 40;
    return s;
}

int add_item(item_id item, int count)
{
    const item_def& def = get_item(item);
    int stack = def.stack ? def.stack : 1;

    // Top up existing stacks first, then use empty slots.
    for(item_stack& slot : data.bags)
    {
        if(count && slot.item == item && slot.count < stack)
        {
            int added = bn::min(count, stack - int(slot.count));
            slot.count += added;
            count -= added;
        }
    }

    for(item_stack& slot : data.bags)
    {
        if(count && slot.item == item_id::NONE)
        {
            int added = bn::min(count, stack);
            slot.item = item;
            slot.count = added;
            count -= added;
        }
    }

    return count;
}

int remove_item(item_id item, int count)
{
    int removed = 0;

    for(item_stack& slot : data.bags)
    {
        if(count && slot.item == item)
        {
            int taken = bn::min(count, int(slot.count));
            slot.count -= taken;
            count -= taken;
            removed += taken;

            if(slot.count == 0)
            {
                slot.item = item_id::NONE;
            }
        }
    }

    return removed;
}

int item_count(item_id item)
{
    int count = 0;

    for(const item_stack& slot : data.bags)
    {
        if(slot.item == item)
        {
            count += slot.count;
        }
    }

    return count;
}

int free_bag_slots()
{
    int count = 0;

    for(const item_stack& slot : data.bags)
    {
        if(slot.item == item_id::NONE)
        {
            ++count;
        }
    }

    return count;
}

equip_result equip_item(int bag_index)
{
    item_stack& slot = data.bags[bag_index];
    const item_def& item = get_item(slot.item);

    if(slot.item == item_id::NONE || item.slot == equip_slot::NONE)
    {
        return equip_result::NOT_EQUIPMENT;
    }

    if(! can_equip(data.player_class, item))
    {
        return equip_result::WRONG_CLASS;
    }

    if(data.level < item.level)
    {
        return equip_result::LEVEL_TOO_LOW;
    }

    item_id& main_hand = data.equipment[int(equip_slot::MAIN_HAND)];
    item_id& off_hand = data.equipment[int(equip_slot::OFF_HAND)];
    bool frees_off_hand = is_two_handed(item) && off_hand != item_id::NONE;
    bool frees_main_hand = item.slot == equip_slot::OFF_HAND && main_hand != item_id::NONE &&
            is_two_handed(get_item(main_hand));

    // The item leaves its bag slot, so one extra item always fits there.
    if((frees_off_hand || frees_main_hand) && data.equipment[int(item.slot)] != item_id::NONE &&
       free_bag_slots() == 0)
    {
        return equip_result::BAGS_FULL;
    }

    item_id new_item = slot.item;
    item_id old_item = data.equipment[int(item.slot)];
    slot = item_stack();
    data.equipment[int(item.slot)] = new_item;

    if(old_item != item_id::NONE)
    {
        add_item(old_item);
    }

    if(frees_off_hand)
    {
        add_item(off_hand);
        off_hand = item_id::NONE;
    }

    if(frees_main_hand)
    {
        add_item(main_hand);
        main_hand = item_id::NONE;
    }

    return equip_result::OK;
}

bool unequip_item(equip_slot slot)
{
    item_id& equipped = data.equipment[int(slot)];

    if(equipped == item_id::NONE || free_bag_slots() == 0)
    {
        return false;
    }

    add_item(equipped);
    equipped = item_id::NONE;
    return true;
}

int buy_price(item_id item)
{
    return get_item(item).price * 4;
}

int sell_price(item_id item)
{
    return get_item(item).price;
}

}
