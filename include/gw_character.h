#ifndef GW_CHARACTER_H
#define GW_CHARACTER_H

#include "gw_abilities.h"
#include "gw_ids.h"
#include "gw_items.h"

namespace gw
{

constexpr int max_level = 20;
constexpr int bag_slots = 16;
constexpr int action_slots = 7;
constexpr int max_quests = 32;
constexpr int max_talents = 24;
constexpr int quest_objectives = 3;

struct item_stack
{
    item_id item = item_id::NONE;
    uint8_t count = 0;
};

enum class quest_status : uint8_t
{
    NOT_STARTED,
    ACTIVE,
    COMPLETE,       // objectives done, waiting to be turned in
    TURNED_IN
};

struct quest_progress
{
    quest_status status = quest_status::NOT_STARTED;
    uint8_t counts[quest_objectives] = {};
};

// Everything about the player that is saved.
struct character_data
{
    race_id race = race_id::HUMAN;
    class_id player_class = class_id::WARRIOR;
    uint8_t level = 1;
    uint8_t talent_points_spent = 0;
    int32_t xp = 0;
    int32_t money = 0;              // copper
    int16_t health = 1;
    int16_t power = 0;              // rage or mana
    map_id map = map_id::ELWYNN;
    int16_t x = 0;
    int16_t y = 0;
    item_stack bags[bag_slots];
    item_id equipment[int(equip_slot::COUNT)] = {};
    uint32_t known_abilities = 0;   // bit per ability_id
    ability_id action_bar[action_slots] = {};
    uint8_t talents[max_talents] = {};
    quest_progress quests[max_quests];
    uint32_t flags = 0;             // story flags, see story_flag
    uint32_t play_frames = 0;
    uint8_t home = 0;               // home_id the hearthstone returns to
    uint32_t hearthstone_ready = 0; // play_frames when it can be used again
};

// One-off story events.
enum class story_flag : uint8_t
{
    HOGGER_KILLED,
    PRINCESS_KILLED,
    VANCLEEF_KILLED,
    SNEED_KILLED,
    SEEN_INTRO
};

[[nodiscard]] character_data& character();

// Resets the character to a fresh level 1 of the race and class, at the start of the game.
void new_character(race_id race, class_id player_class);

[[nodiscard]] bool has_flag(story_flag flag);

void set_flag(story_flag flag);

[[nodiscard]] bool knows_ability(ability_id ability);

// Learns the ability and puts it on the first free action slot.
void learn_ability(ability_id ability);

// Forgets the ability and takes it off the action bar (unlearning talents).
void forget_ability(ability_id ability);

// Experience needed to go from level to level + 1 (0 at the level cap).
[[nodiscard]] int xp_for_level(int level);

// Experience for killing an enemy of enemy_level (elites give double).
[[nodiscard]] int kill_xp(int enemy_level, bool elite);

// Gray enemies are too low to give experience.
[[nodiscard]] bool is_gray(int enemy_level);

[[nodiscard]] bool uses_mana();

// Temporary bonuses from buffs, applied on top of gear, talents and level.
struct stat_bonus
{
    int attack_power = 0;
    int ranged_attack_power = 0;
    int armor = 0;
    int health_percent = 0;
    int damage_percent = 0;
    int haste_percent = 0;
};

struct stats
{
    int strength;
    int agility;
    int stamina;
    int intellect;
    int spirit;
    int max_health;
    int max_power;
    int armor;
    int attack_power;
    int ranged_attack_power;
    int melee_min;
    int melee_max;
    int melee_speed;        // frames between swings
    int ranged_min;
    int ranged_max;
    int ranged_speed;
    bool has_ranged;
    int crit;               // percent
    int spell_crit;         // percent
    int dodge;              // percent
    int damage_percent;     // all damage dealt, 100 = normal
    int crit_percent;       // damage of a critical hit, 200 = double
    int rage_percent;       // rage gained, 100 = normal
    int spell_power;        // added to spell damage
    int health_regen;       // per tick out of combat
    int power_regen;        // mana per tick, or rage lost per tick out of combat
};

[[nodiscard]] stats compute_stats(const stat_bonus& bonus = stat_bonus());

// Adds items to the bags. Returns how many did not fit.
int add_item(item_id item, int count = 1);

// Removes up to count items from the bags. Returns how many were removed.
int remove_item(item_id item, int count = 1);

[[nodiscard]] int item_count(item_id item);

[[nodiscard]] int free_bag_slots();

enum class equip_result : uint8_t
{
    OK,
    NOT_EQUIPMENT,
    WRONG_CLASS,
    LEVEL_TOO_LOW,
    BAGS_FULL
};

// Puts on the item in the bag slot; whatever it replaces goes back to the bags. Two-handed weapons
// also take off the shield, and a shield takes off a two-handed weapon.
equip_result equip_item(int bag_index);

// Takes the item off into the bags. Returns false if the bags are full.
bool unequip_item(equip_slot slot);

// Vendors sell for four times what they pay.
[[nodiscard]] int buy_price(item_id item);

[[nodiscard]] int sell_price(item_id item);

}

#endif
