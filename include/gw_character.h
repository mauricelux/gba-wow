#ifndef GW_CHARACTER_H
#define GW_CHARACTER_H

#include "gw_abilities.h"
#include "gw_ids.h"
#include "gw_items.h"
#include "gw_look_ids.h"

namespace gw
{

constexpr int max_level = 60;
constexpr int bag_rows = 400;     // different stacks the bags hold: more than a whole game's loot
constexpr int max_stack = 999;
constexpr int action_slots = 7;     // A, B, L, up, right, down, left (L only on the Combat bar)
constexpr int item_slots = 4;       // up, right, down, left
constexpr int max_quests = 256;
constexpr int max_talents = 24;
constexpr int quest_objectives = 3;
constexpr int max_chests = 256;
constexpr int max_story_flags = 256;

// Rested experience: every rest_frames_per_step frames played since the last rest at an inn add
// rest_step_percent of a level, up to rest_max_percent of a level.
constexpr int rest_frames_per_step = 6 * 60 * 60;
constexpr int rest_step_percent = 5;
constexpr int rest_max_percent = 150;

struct item_stack
{
    item_id item = item_id::NONE;
    uint16_t count = 0;
};

// How the Bags page lists the bags. Saves store it by value: only append.
enum class bag_sort : uint8_t
{
    TYPE,
    QUALITY,
    LEVEL,
    NEWEST,
    COUNT
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

// Everything about the player that is saved. The save file stores it field by field (gw_save.cpp), so
// arrays can grow without breaking old saves.
struct character_data
{
    race_id race = race_id::HUMAN;
    class_id player_class = class_id::WARRIOR;
    subclass_id subclass = subclass_id::NONE;
    uint8_t level = 1;
    uint8_t talent_points_spent = 0;
    int32_t xp = 0;
    int32_t money = 0;              // copper
    int16_t health = 1;
    int16_t power = 0;              // rage or mana
    map_id map = map_id::ELWYNN;
    int16_t x = 0;
    int16_t y = 0;
    item_stack bags[bag_rows];      // the rows in use come first, oldest first; no gaps
    item_id equipment[int(equip_slot::COUNT)] = {};
    uint8_t ability_ranks[ability_count] = {};  // the rank known of each ability, 0 = not known
    ability_id action_bars[bar_count][action_slots] = {};   // by bar_id
    item_id item_bar[item_slots] = {};  // NONE: the slot's usual kind (see item_bar_default)
    uint8_t talents[max_talents] = {};  // the rank of each talent of the subclass's tree
    quest_progress quests[max_quests];  // indexed by quest_id
    uint32_t flags[max_story_flags / 32] = {};      // bit per story_flag
    uint32_t play_frames = 0;
    uint8_t home = 0;               // home_id the hearthstone returns to
    uint32_t hearthstone_ready = 0; // play_frames when it can be used again
    uint32_t chests_opened[max_chests / 32] = {};   // bit per chest_def::id
    int32_t rest_xp = 0;            // kills give this much extra experience before it runs out
    uint32_t last_rest = 0;         // play_frames at the last rest at an inn
    bag_sort sort = bag_sort::TYPE;
};

// One-off story events. Saves store them by value: only append.
enum class story_flag : uint8_t
{
    HOGGER_KILLED,
    PRINCESS_KILLED,
    VANCLEEF_KILLED,
    SNEED_KILLED,
    SEEN_INTRO,
    BAZIL_KILLED
};

[[nodiscard]] character_data& character();

// Back to a blank character, in place (the data is too large to copy through the stack).
void reset_character();

[[nodiscard]] const char* race_name(race_id race);

[[nodiscard]] const char* class_name(class_id player_class);

[[nodiscard]] const char* subclass_name(subclass_id subclass);

// A line about how the subclass fights, for the creation screen.
[[nodiscard]] const char* subclass_description(subclass_id subclass);

// Warriors fight in the stance of their subclass; nullptr for the other classes.
[[nodiscard]] const char* stance_name(subclass_id subclass);

// The races each class is open to: humans can be warriors or mages, dwarves and night elves
// warriors or hunters.
[[nodiscard]] bool class_allowed(race_id race, class_id player_class);

[[nodiscard]] look_id player_look(race_id race, class_id player_class);

// Resets the character to a fresh level 1 of the race, class and subclass, at the start of the game.
void new_character(race_id race, class_id player_class, subclass_id subclass);

// For characters from before subclasses: takes the subclass, refunds every talent point, keeps the
// known abilities of its kit at rank 1 (and forgets the rest), and teaches its starting abilities.
void choose_subclass(subclass_id subclass);

[[nodiscard]] bool has_flag(story_flag flag);

void set_flag(story_flag flag);

[[nodiscard]] bool chest_opened(int chest);

void set_chest_opened(int chest);

[[nodiscard]] int opened_chest_count();

[[nodiscard]] bool knows_ability(ability_id ability);

// The rank known, 0 if the ability isn't known.
[[nodiscard]] int ability_rank(ability_id ability);

// Learns the rank (and the ones below it). A new ability goes on a free slot of its default bar,
// or of another bar if that one is full.
void learn_ability(ability_id ability, int rank = 1);

// Forgets the ability and takes it off the bars (unlearning talents).
void forget_ability(ability_id ability);

// Whether the bar has the slot: L is the Combat bar's own, since L opens the other two.
[[nodiscard]] constexpr bool bar_has_slot(bar_id bar, int slot)
{
    return bar == bar_id::COMBAT || slot != 2;
}

// Puts the ability on the slot, taking it off wherever it was before. A slot of -1 only takes it off.
void set_bar_slot(bar_id bar, int slot, ability_id ability);

// For saves from before the Utility and Buffs bars: moves buffs and utility abilities off the
// Combat bar to their own bars, and puts known abilities that were on no bar on one.
void arrange_bars();

// The ability's value at the known rank and the character's level.
[[nodiscard]] int ability_value(ability_id ability);

// What the known rank costs to use, in rage or mana.
[[nodiscard]] int ability_cost(ability_id ability);

// The next rank a trainer could teach now (level reached, in the subclass's kit, talent and quest
// abilities only after the talent or the quest), or 0.
[[nodiscard]] int trainable_rank(ability_id ability);

// How many abilities have a rank waiting at a trainer of the class (any_class: riding trainers).
[[nodiscard]] int trainable_count(class_id trainer_class);

// Whether trainers of the class teach the character: their own class's trainers and riding trainers.
[[nodiscard]] bool teaches(class_id trainer_class);

[[nodiscard]] bool has_shield();

// Experience needed to go from level to level + 1 (0 at the level cap).
[[nodiscard]] int xp_for_level(int level);

// Experience for killing an enemy of enemy_level (elites give double).
[[nodiscard]] int kill_xp(int enemy_level, bool elite);

// The part of a kill's experience paid out of rested experience (as much again, while it lasts).
// Takes it from the rested pool.
int use_rest_xp(int kill_xp);

// Rested experience waiting at an inn: the time played since the last rest, up to the cap.
[[nodiscard]] int pending_rest_xp();

// Rests at an inn: moves the pending rested experience into the pool. Returns what it added.
int rest_at_inn();

// Gray enemies are too low to give experience.
[[nodiscard]] bool is_gray(int enemy_level);

[[nodiscard]] bool uses_mana();

// Temporary bonuses from buffs, applied on top of gear, talents and level.
struct stat_bonus
{
    int attack_power = 0;
    int ranged_attack_power = 0;
    int armor = 0;
    int intellect = 0;
    int health_percent = 0;
    int damage_percent = 0;
    int damage_taken_percent = 0;
    int haste_percent = 0;
    int ranged_haste_percent = 0;
    int crit = 0;
    int spell_crit = 0;
    int dodge = 0;
    int block = 0;
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
    int block;              // percent, with a shield: a blocked hit does half damage
    int damage_percent;     // all damage dealt, 100 = normal
    int damage_taken_percent;   // 100 = normal
    int crit_percent;       // damage of a critical hit, 200 = double
    int rage_percent;       // rage gained, 100 = normal
    int spell_power;        // added to spell damage
    int health_regen;       // per tick out of combat
    int power_regen;        // mana per tick, or rage lost per tick out of combat
};

[[nodiscard]] stats compute_stats(const stat_bonus& bonus = stat_bonus());

// Adds items to the bags: stackable items to their row (up to max_stack), the others to a new row
// each. Returns how many did not fit, which only happens with all bag_rows in use.
int add_item(item_id item, int count = 1);

// Removes up to count items from the bags. Returns how many were removed.
int remove_item(item_id item, int count = 1);

// Removes up to count items from one row; the rows after it move up if it empties.
void remove_from_row(int row, int count);

[[nodiscard]] int item_count(item_id item);

// The rows of the bags in use.
[[nodiscard]] int bag_row_count();

// Merges rows of the same stackable item and closes gaps (after loading a save).
void tidy_bags();

[[nodiscard]] bool stackable(item_id item);

// Items with a use from the bags or the Items bar: potions, food, drink and the hearthstone.
[[nodiscard]] bool usable_item(item_id item);

// What an Items bar slot holds when it was never set: healing potion up, food right, drink down and
// the hearthstone left.
[[nodiscard]] item_type item_bar_default(int slot);

// The item an Items bar slot uses now: the one set if it is still in the bags, otherwise the best
// of the same kind the character can use. NONE if there is nothing to use.
[[nodiscard]] item_id item_bar_item(int slot);

enum class equip_result : uint8_t
{
    OK,
    NOT_EQUIPMENT,
    WRONG_CLASS,
    LEVEL_TOO_LOW
};

// Puts on the item in the bag row; whatever it replaces goes back to the bags. Two-handed weapons
// also take off the shield, and a shield takes off a two-handed weapon.
equip_result equip_item(int row);

// Takes the item off into the bags. Returns false if nothing is in the slot.
bool unequip_item(equip_slot slot);

// Vendors sell for four times what they pay.
[[nodiscard]] int buy_price(item_id item);

[[nodiscard]] int sell_price(item_id item);

}

#endif
