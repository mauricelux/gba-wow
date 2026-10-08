#ifndef GW_QUEST_TEXT_H
#define GW_QUEST_TEXT_H

#include "gw_quest_ids.h"

namespace gw
{

class text_page;

// Objective lines; with counts they show progress ("Kobold Vermin slain: 3/8").
void add_quest_objectives(text_page& page, quest_id quest, bool counts);

// Experience, money and reward items. selected marks the chosen item when there is a choice
// (-1 for none).
void add_quest_rewards(text_page& page, quest_id quest, int selected);

// Number of reward items to choose from.
[[nodiscard]] int quest_reward_count(quest_id quest);

}

#endif
