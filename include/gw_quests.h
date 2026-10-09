#ifndef GW_QUESTS_H
#define GW_QUESTS_H

#include "gw_character.h"
#include "gw_ids.h"
#include "gw_item_ids.h"
#include "gw_maps.h"
#include "gw_quest_ids.h"

namespace gw
{

class hud;

enum class objective_type : uint8_t
{
    NONE,
    KILL,       // kill count enemies
    COLLECT,    // enemies drop the quest item with chance percent (kept with the quest, not in bags)
    EXPLORE,    // walk into the area
    TREASURE,   // open count treasure chests (chests opened before count too)
    TAME,       // tame count beasts
    FISH        // each Fishing cast from inside the area catches one with chance percent
};

struct objective_def
{
    objective_type type;
    uint8_t count;
    enemy_id enemy;
    enemy_id enemy2;        // a second enemy that also counts, or NONE
    uint8_t chance;
    area_id area;
    const char* name;       // what the quest log shows: "Kobold Vermin slain", "Tough Wolf Meat"
};

constexpr int quest_rewards = 3;

struct quest_def
{
    const char* title;
    const char* text;           // the story told when the quest is offered
    const char* goal;           // the short summary shown under the story: "Kill 8 Kobold Vermin."
    const char* progress;       // said by the ender while the quest is not done
    const char* completion;     // said by the ender when it is turned in
    npc_id giver;
    npc_id ender;
    uint8_t level;
    uint8_t min_level;
    quest_id previous;           // must be turned in first
    objective_def objectives[quest_objectives];
    int16_t xp;
    int16_t money;              // copper
    item_id rewards[quest_rewards];   // the player picks one
    subclass_id subclass = subclass_id::NONE;   // only offered to this subclass
};

[[nodiscard]] const quest_def& get_quest(quest_id quest);

[[nodiscard]] quest_progress& quest_state(quest_id quest);

// Not started, level high enough and the previous quest turned in.
[[nodiscard]] bool quest_available(quest_id quest);

[[nodiscard]] bool quest_objectives_done(quest_id quest);

[[nodiscard]] int quest_objective_count(const quest_def& quest);

// Starts the quest. Quests without objectives are complete at once.
void accept_quest(quest_id quest);

// Marks the quest turned in and hands out its money and the chosen reward item (experience is
// given by the caller through combat so level ups show). Returns false if every bag row is in use.
bool turn_in_quest(quest_id quest, int reward_index);

// Gives up the quest: its progress is lost and it can be picked up again.
void abandon_quest(quest_id quest);

// Number of quests in the log (active or complete).
[[nodiscard]] int quest_log_count();

// The index-th quest of the log in quest order, or NONE.
[[nodiscard]] quest_id quest_log_at(int index);

enum class quest_marker : uint8_t
{
    NONE,
    IN_PROGRESS,    // gray question mark: the npc will take a quest that is not done yet
    AVAILABLE,      // yellow exclamation mark: the npc has a quest to give
    COMPLETE,       // yellow question mark: a quest can be turned in
    TRAINER         // blue exclamation mark: the class trainer has new ranks to teach
};

[[nodiscard]] quest_marker npc_quest_marker(npc_id npc);

// The quests the npc offers or takes, most important first. Returns the number written.
int npc_quests(npc_id npc, quest_id* out, int max_count);

// Text color for a level compared to the player's: gray, green, yellow or red.
[[nodiscard]] uint8_t level_color(int level);

// Events. Each returns true if a quest changed, writing progress messages to the hud.
bool quests_on_kill(enemy_id enemy, hud& hud_ref);

bool quests_on_explore(const map_info& map, int x, int y, hud& hud_ref);

bool quests_on_chest(hud& hud_ref);

bool quests_on_tame(hud& hud_ref);

// A Fishing cast from (x, y) ended: rolls each fishing objective of the area the player stands in, and
// tells what was caught.
bool quests_on_fish(const map_info& map, int x, int y, hud& hud_ref);

}

#endif
