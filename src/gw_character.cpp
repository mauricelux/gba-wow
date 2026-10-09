#include "gw_character.h"

#include <new>

#include "bn_math.h"

#include "gw_item_sets.h"
#include "gw_map_elwynn.h"
#include "gw_maps.h"
#include "gw_talents.h"

namespace gw
{

namespace
{
    BN_DATA_EWRAM character_data data;

    // Experience per level: classic values scaled down for a handheld session up to level 14. From
    // level 15 each level is what its zone hands out: about 28 kills and 4 quests of its level, so the
    // quests of Redridge, the Deadmines and the Stockade take a hero from 15 to 21.
    constexpr int xp_table[max_level] = {
        160, 360, 560, 840, 1120, 1440, 1800, 2160, 2600, 2950,
        3250, 3550, 3850, 4150, 9000, 11500, 13250, 14500, 15200, 15950,
        16900, 17900, 18900, 19900, 20950, 22000, 23050, 24150, 25300, 26450,
        27600, 28800, 30000, 31250, 32500, 33750, 35050, 36350, 37700, 39050,
        40450, 41850, 43300, 44700, 46200, 47700, 49200, 50750, 52300, 53850,
        55450, 57100, 58700, 60400, 62050, 63750, 65500, 67250, 69000, 0
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

void reset_character()
{
    // Trivially destructible: a new one in place of the old.
    new(&data) character_data();
}

const char* race_name(race_id race)
{
    constexpr const char* names[] = { "Human", "Dwarf", "Night Elf" };
    return names[int(race)];
}

const char* class_name(class_id player_class)
{
    constexpr const char* names[] = { "Warrior", "Mage", "Hunter" };
    return names[int(player_class)];
}

const char* subclass_name(subclass_id subclass)
{
    constexpr const char* names[] = {
        "", "Arms", "Fury", "Protection", "Arcane", "Fire", "Frost", "Beast Mastery", "Marksmanship", "Survival"
    };
    return names[int(subclass) < int(subclass_id::COUNT) ? int(subclass) : 0];
}

const char* subclass_description(subclass_id subclass)
{
    constexpr const char* texts[] = {
        "",
        "Heavy two-handed hits and bleeding wounds. Battle Stance.",
        "Fast hits and more crits, but takes more damage. Berserker Stance.",
        "Shield and heavy armor, made to outlast anything. Defensive Stance.",
        "Mana-hungry bursts, blinks and the best damage against groups.",
        "Burst and burning damage over time.",
        "Slows, freezes and shatters. The safest way to play a mage.",
        "Hunts beside a tamed beast. Pets arrive in a later update.",
        "Long-range shots and big aimed hits.",
        "Melee and traps. The hardest hunter to kill.",
    };
    return texts[int(subclass) < int(subclass_id::COUNT) ? int(subclass) : 0];
}

const char* stance_name(subclass_id subclass)
{
    switch(subclass)
    {

    case subclass_id::ARMS:
        return "Battle Stance";

    case subclass_id::FURY:
        return "Berserker Stance";

    case subclass_id::PROTECTION:
        return "Defensive Stance";

    default:
        return nullptr;
    }
}

bool class_allowed(race_id race, class_id player_class)
{
    switch(player_class)
    {

    case class_id::MAGE:
        return race == race_id::HUMAN;

    case class_id::HUNTER:
        return race != race_id::HUMAN;

    default:
        return true;
    }
}

look_id player_look(race_id race, class_id player_class)
{
    switch(race)
    {

    case race_id::DWARF:
        return player_class == class_id::HUNTER ? look_id::DWARF_HUNTER : look_id::DWARF_WARRIOR;

    case race_id::NIGHT_ELF:
        return player_class == class_id::HUNTER ? look_id::ELF_HUNTER : look_id::ELF_WARRIOR;

    default:
        return player_class == class_id::MAGE ? look_id::HUMAN_MAGE : look_id::HUMAN_WARRIOR;
    }
}

namespace
{
    // The kit's starting abilities, attacks first so they take the first action slots.
    void learn_starters()
    {
        for(int pass = 0; pass < 2; ++pass)
        {
            for(int index = 1; index < ability_count; ++index)
            {
                auto ability = ability_id(index);
                const ability_def& def = get_ability(ability);

                if((def.flags & ability_flag::STARTER) && in_kit(ability, data.subclass) &&
                   (def.target == ability_target::SELF) == (pass == 1))
                {
                    learn_ability(ability);
                }
            }
        }
    }
}

void new_character(race_id race, class_id player_class, subclass_id subclass)
{
    reset_character();
    data.race = race;
    data.player_class = player_class;
    data.subclass = subclass;
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
        break;

    case class_id::HUNTER:
        equip(item_id::WORN_HATCHET);
        equip(item_id::CRACKED_SHORTBOW);
        equip(item_id::TRAPPERS_VEST);
        equip(item_id::TRAPPERS_PANTS);
        equip(item_id::TRAPPERS_BOOTS);
        break;

    default:
        equip(item_id::WORN_SHORTSWORD);
        equip(item_id::RECRUITS_VEST);
        equip(item_id::RECRUITS_PANTS);
        equip(item_id::RECRUITS_BOOTS);
        break;
    }

    learn_starters();

    add_item(item_id::HEARTHSTONE);
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
    return data.flags[int(flag) / 32] & (1u << (int(flag) % 32));
}

void set_flag(story_flag flag)
{
    data.flags[int(flag) / 32] |= 1u << (int(flag) % 32);
}

bool chest_opened(int chest)
{
    return data.chests_opened[chest / 32] & (1u << (chest % 32));
}

void set_chest_opened(int chest)
{
    data.chests_opened[chest / 32] |= 1u << (chest % 32);
}

int opened_chest_count()
{
    // Treasure only: a lit brazier is a chest of level 0.
    int result = 0;

    for(const map_info& map : all_maps())
    {
        for(const chest_def& chest : map.chests)
        {
            result += chest.level != 0 && chest_opened(chest.id);
        }
    }

    return result;
}

void choose_subclass(subclass_id subclass)
{
    data.subclass = subclass;

    for(uint8_t& rank : data.talents)
    {
        rank = 0;
    }

    data.talent_points_spent = 0;

    for(int index = 1; index < ability_count; ++index)
    {
        auto ability = ability_id(index);
        uint8_t& rank = data.ability_ranks[index];

        if(rank)
        {
            bool kept = in_kit(ability, subclass) && ! (get_ability(ability).flags & ability_flag::TALENT);
            rank = kept ? 1 : 0;
        }
    }

    for(auto& bar : data.action_bars)
    {
        for(ability_id& slot : bar)
        {
            if(! knows_ability(slot))
            {
                slot = ability_id::NONE;
            }
        }
    }

    learn_starters();
}

bool knows_ability(ability_id ability)
{
    return ability != ability_id::NONE && int(ability) < ability_count && data.ability_ranks[int(ability)] > 0;
}

int ability_rank(ability_id ability)
{
    return int(ability) < ability_count ? data.ability_ranks[int(ability)] : 0;
}

namespace
{
    [[nodiscard]] bool on_a_bar(ability_id ability)
    {
        for(const auto& bar : data.action_bars)
        {
            for(ability_id slot : bar)
            {
                if(slot == ability)
                {
                    return true;
                }
            }
        }

        return false;
    }

    // A free slot of the bar, in button order; false if it is full.
    bool place_on(bar_id bar, ability_id ability)
    {
        for(int slot = 0; slot < action_slots; ++slot)
        {
            ability_id& own = data.action_bars[int(bar)][slot];

            if(own == ability_id::NONE && bar_has_slot(bar, slot))
            {
                own = ability;
                return true;
            }
        }

        return false;
    }

    // Its own bar first, then the others: Combat, Utility, Buffs.
    void place(ability_id ability)
    {
        bar_id own = default_bar(ability);

        if(place_on(own, ability))
        {
            return;
        }

        for(int bar = 0; bar < bar_count; ++bar)
        {
            if(bar_id(bar) != own && place_on(bar_id(bar), ability))
            {
                return;
            }
        }
    }
}

void learn_ability(ability_id ability, int rank)
{
    if(ability == ability_id::NONE || int(ability) >= ability_count)
    {
        return;
    }

    bool known = knows_ability(ability);
    uint8_t& own = data.ability_ranks[int(ability)];
    own = uint8_t(bn::max(int(own), bn::clamp(rank, 1, bn::max(1, rank_count(ability)))));

    if(! known && ! on_a_bar(ability))
    {
        place(ability);
    }
}

void forget_ability(ability_id ability)
{
    if(int(ability) < ability_count)
    {
        data.ability_ranks[int(ability)] = 0;
    }

    for(auto& bar : data.action_bars)
    {
        for(ability_id& slot : bar)
        {
            if(slot == ability)
            {
                slot = ability_id::NONE;
            }
        }
    }
}

void set_bar_slot(bar_id bar, int slot, ability_id ability)
{
    if(ability != ability_id::NONE)
    {
        for(auto& other : data.action_bars)
        {
            for(ability_id& own : other)
            {
                if(own == ability)
                {
                    own = ability_id::NONE;
                }
            }
        }
    }

    if(slot >= 0 && slot < action_slots && bar_has_slot(bar, slot))
    {
        data.action_bars[int(bar)][slot] = ability;
    }
}

void arrange_bars()
{
    ability_id* combat = data.action_bars[int(bar_id::COMBAT)];

    for(int slot = 0; slot < action_slots; ++slot)
    {
        ability_id ability = combat[slot];

        if(ability != ability_id::NONE && default_bar(ability) != bar_id::COMBAT)
        {
            combat[slot] = ability_id::NONE;

            if(! place_on(default_bar(ability), ability))
            {
                combat[slot] = ability;
            }
        }
    }

    for(int index = 1; index < ability_count; ++index)
    {
        auto ability = ability_id(index);

        if(knows_ability(ability) && ! on_a_bar(ability))
        {
            place(ability);
        }
    }
}

int ability_value(ability_id ability)
{
    int value = rank_value(ability, bn::max(1, ability_rank(ability)), data.level);
    int bonus = talent_value(talent_effect::ABILITY_DAMAGE, ability);
    return bonus ? value * (100 + bonus) / 100 : value;
}

int ability_cost(ability_id ability)
{
    int cost = rank_cost(ability, bn::max(1, ability_rank(ability)));
    int percent = talent_bonuses().cost_percent + talent_value(talent_effect::ABILITY_COST, ability);
    return cost - cost * percent / 100;
}

int trainable_rank(ability_id ability)
{
    if(! in_kit(ability, data.subclass))
    {
        return 0;
    }

    int rank = ability_rank(ability);

    if(rank == 0 && (get_ability(ability).flags & (ability_flag::TALENT | ability_flag::QUEST)))
    {
        return 0;
    }

    int level = rank_level(ability, rank + 1);
    return level && level <= data.level ? rank + 1 : 0;
}

int trainable_count(class_id trainer_class)
{
    int count = 0;

    for(int index = 1; index < ability_count; ++index)
    {
        auto ability = ability_id(index);

        if(get_ability(ability).player_class == trainer_class && trainable_rank(ability))
        {
            ++count;
        }
    }

    return count;
}

bool teaches(class_id trainer_class)
{
    return trainer_class == data.player_class || trainer_class == any_class;
}

bool has_shield()
{
    item_id off_hand = data.equipment[int(equip_slot::OFF_HAND)];
    return off_hand != item_id::NONE && get_item(off_hand).type == item_type::SHIELD;
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
    // How many levels below the player an enemy gives no experience, as in WoW.
    [[nodiscard]] int zero_difference(int level)
    {
        constexpr int limits[][2] = {
            { 7, 5 }, { 9, 6 }, { 11, 7 }, { 15, 8 }, { 19, 9 }, { 29, 11 }, { 39, 12 }, { 44, 13 },
            { 49, 14 }, { 54, 15 }, { 59, 16 }
        };

        for(const auto& limit : limits)
        {
            if(level <= limit[0])
            {
                return limit[1];
            }
        }

        return 17;
    }

    [[nodiscard]] int max_rest_xp()
    {
        return xp_for_level(data.level) * rest_max_percent / 100;
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

int use_rest_xp(int kill_xp)
{
    int bonus = bn::min(kill_xp, int(data.rest_xp));
    data.rest_xp -= bonus;
    return bonus;
}

int pending_rest_xp()
{
    uint32_t played = data.play_frames - data.last_rest;
    int steps = int(bn::min(played / rest_frames_per_step, uint32_t(rest_max_percent / rest_step_percent)));
    int pending = xp_for_level(data.level) * rest_step_percent * steps / 100;
    return bn::max(0, bn::min(pending, max_rest_xp() - int(data.rest_xp)));
}

int rest_at_inn()
{
    int added = pending_rest_xp();
    data.rest_xp += added;
    data.last_rest = data.play_frames;
    return added;
}

bool uses_mana()
{
    return data.player_class != class_id::WARRIOR;
}

stats compute_stats(const stat_bonus& extra)
{
    // Buffs and talents come in through extra; the dungeon sets add theirs.
    stat_bonus bonus = extra;
    add_item_set_bonuses(bonus);
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
    s.intellect += bonus.intellect;
    s.strength += s.strength * talent.strength_percent / 100;
    s.agility += s.agility * talent.agility_percent / 100;
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
    int ranged_haste = haste + bonus.ranged_haste_percent;

    if(haste)
    {
        s.melee_speed = s.melee_speed * 100 / (100 + haste);
    }

    if(ranged_haste)
    {
        s.ranged_speed = s.ranged_speed * 100 / (100 + ranged_haste);
    }

    s.crit = 5 + s.agility / 20 + talent.crit + bonus.crit;
    s.spell_crit = 5 + s.intellect / 30 + talent.spell_crit + bonus.spell_crit;
    s.dodge = 5 + s.agility / 20 + talent.dodge + bonus.dodge;
    s.block = has_shield() ? 5 + talent.block + bonus.block : 0;
    s.damage_percent = 100 + bonus.damage_percent + talent.damage_percent;
    s.damage_taken_percent = 100 + bonus.damage_taken_percent - talent.damage_taken;

    // Warriors fight in their subclass's stance.
    if(data.subclass == subclass_id::FURY)
    {
        s.crit += 3;
        s.damage_taken_percent += 10;
    }
    else if(data.subclass == subclass_id::PROTECTION)
    {
        s.damage_taken_percent -= 10;
    }

    s.damage_taken_percent = bn::max(10, s.damage_taken_percent);

    s.crit_percent = 200 + talent.crit_damage_percent;
    s.rage_percent = 100 + talent.rage_percent;
    s.spell_power = level * 2 + talent.spell_power;
    s.health_regen = bn::max(1, s.max_health / 25 + s.spirit / 5);
    s.power_regen = data.player_class == class_id::WARRIOR ? 2 : s.spirit / 4 + 12 + s.max_power / 40;

    if(data.player_class != class_id::WARRIOR)
    {
        s.power_regen += s.power_regen * talent.regen_percent / 100;
    }
    return s;
}

bool stackable(item_id item)
{
    return get_item(item).stack > 1;
}

int bag_row_count()
{
    // Rows are packed, so the first empty one ends the list.
    int low = 0;
    int high = bag_rows;

    while(low < high)
    {
        int middle = (low + high) / 2;

        if(data.bags[middle].item != item_id::NONE)
        {
            low = middle + 1;
        }
        else
        {
            high = middle;
        }
    }

    return low;
}

int add_item(item_id item, int count)
{
    if(item == item_id::NONE || count <= 0)
    {
        return count;
    }

    int rows = bag_row_count();

    if(stackable(item))
    {
        // Tops up its row first.
        for(int row = 0; row < rows && count; ++row)
        {
            item_stack& stack = data.bags[row];

            if(stack.item == item && stack.count < max_stack)
            {
                int added = bn::min(count, max_stack - int(stack.count));
                stack.count += added;
                count -= added;
            }
        }
    }

    while(count && rows < bag_rows)
    {
        int added = stackable(item) ? bn::min(count, max_stack) : 1;
        data.bags[rows].item = item;
        data.bags[rows].count = uint16_t(added);
        count -= added;
        ++rows;
    }

    return count;
}

void remove_from_row(int row, int count)
{
    int rows = bag_row_count();

    if(row < 0 || row >= rows || count <= 0)
    {
        return;
    }

    item_stack& stack = data.bags[row];
    stack.count = uint16_t(bn::max(0, int(stack.count) - count));

    if(stack.count == 0)
    {
        for(int index = row; index < rows - 1; ++index)
        {
            data.bags[index] = data.bags[index + 1];
        }

        data.bags[rows - 1] = item_stack();
    }
}

int remove_item(item_id item, int count)
{
    int removed = 0;

    // From the newest rows, so a stack bought just now goes first.
    for(int row = bag_row_count() - 1; row >= 0 && count; --row)
    {
        item_stack& stack = data.bags[row];

        if(stack.item == item)
        {
            int taken = bn::min(count, int(stack.count));
            count -= taken;
            removed += taken;
            remove_from_row(row, taken);
        }
    }

    return removed;
}

int item_count(item_id item)
{
    int count = 0;

    for(int row = 0, rows = bag_row_count(); row < rows; ++row)
    {
        if(data.bags[row].item == item)
        {
            count += data.bags[row].count;
        }
    }

    return count;
}

void tidy_bags()
{
    int rows = 0;

    for(int index = 0; index < bag_rows; ++index)
    {
        item_stack stack = data.bags[index];
        data.bags[index] = item_stack();

        if(stack.item == item_id::NONE || stack.count == 0 || stack.item >= item_id::COUNT)
        {
            continue;
        }

        if(! stackable(stack.item))
        {
            data.bags[rows++] = item_stack{ stack.item, 1 };
            continue;
        }

        int count = bn::min(int(stack.count), max_stack);

        for(int row = 0; row < rows && count; ++row)
        {
            item_stack& own = data.bags[row];

            if(own.item == stack.item && own.count < max_stack)
            {
                int added = bn::min(count, max_stack - int(own.count));
                own.count += added;
                count -= added;
            }
        }

        if(count && rows < bag_rows)
        {
            data.bags[rows++] = item_stack{ stack.item, uint16_t(count) };
        }
    }
}

bool usable_item(item_id item)
{
    switch(get_item(item).type)
    {

    case item_type::FOOD:
    case item_type::DRINK:
    case item_type::POTION:
    case item_type::HEARTHSTONE:
        return item != item_id::NONE;

    default:
        return false;
    }
}

item_type item_bar_default(int slot)
{
    constexpr item_type defaults[item_slots] = {
        item_type::POTION, item_type::FOOD, item_type::DRINK, item_type::HEARTHSTONE
    };

    return defaults[slot];
}

item_id item_bar_item(int slot)
{
    item_id set = data.item_bar[slot];

    if(set != item_id::NONE && item_count(set) > 0)
    {
        return set;
    }

    item_type type = set != item_id::NONE ? get_item(set).type : item_bar_default(slot);
    item_id best = item_id::NONE;

    // The strongest one the character can use; any, if none is low enough.
    for(int row = 0, rows = bag_row_count(); row < rows; ++row)
    {
        item_id own = data.bags[row].item;
        const item_def& def = get_item(own);

        if(def.type != type)
        {
            continue;
        }

        if(best == item_id::NONE)
        {
            best = own;
            continue;
        }

        const item_def& best_def = get_item(best);
        bool fits = def.level <= data.level;
        bool best_fits = best_def.level <= data.level;

        if(fits != best_fits ? fits : def.min_damage > best_def.min_damage)
        {
            best = own;
        }
    }

    return best;
}

equip_result equip_item(int row)
{
    if(row < 0 || row >= bag_row_count())
    {
        return equip_result::NOT_EQUIPMENT;
    }

    item_id new_item = data.bags[row].item;
    const item_def& item = get_item(new_item);

    if(item.slot == equip_slot::NONE)
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

    item_id old_item = data.equipment[int(item.slot)];
    remove_from_row(row, 1);
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

    if(equipped == item_id::NONE || add_item(equipped) > 0)
    {
        return false;
    }

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
