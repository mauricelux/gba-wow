#include "gw_quest_text.h"

#include "bn_string.h"

#include "gw_items.h"
#include "gw_quests.h"
#include "gw_text_page.h"

namespace gw
{

void add_quest_objectives(text_page& page, quest_id quest, bool counts)
{
    const quest_def& def = get_quest(quest);
    const quest_progress& progress = quest_state(quest);

    page.add("Objectives", ui::color::YELLOW);
    page.add(def.goal);

    if(! counts)
    {
        return;
    }

    for(int index = 0; index < quest_objectives; ++index)
    {
        const objective_def& objective = def.objectives[index];

        if(objective.type == objective_type::NONE)
        {
            continue;
        }

        int count = progress.counts[index];
        bool done = count >= objective.count;
        bn::string<64> text = "- ";
        text += objective.name;

        if(objective.count > 1)
        {
            text += ": ";
            text += bn::to_string<4>(count);
            text += "/";
            text += bn::to_string<4>(objective.count);
        }

        page.add_copy(text, done ? ui::color::GREEN : ui::color::WHITE);
    }
}

int quest_reward_count(quest_id quest)
{
    int result = 0;

    for(item_id reward : get_quest(quest).rewards)
    {
        if(reward != item_id::NONE)
        {
            ++result;
        }
    }

    return result;
}

void add_quest_rewards(text_page& page, quest_id quest, int selected)
{
    const quest_def& def = get_quest(quest);
    int rewards = quest_reward_count(quest);

    page.add("Rewards", ui::color::YELLOW);

    if(rewards > 1)
    {
        page.add("Choose one with < and >:", ui::color::GRAY);
    }

    for(int index = 0; index < rewards; ++index)
    {
        const item_def& item = get_item(def.rewards[index]);
        bn::string<64> text = rewards > 1 && index == selected ? "> " : "  ";
        text += item.name;
        // Red for gear the class can't wear, like WoW.
        bool usable = can_equip(character().player_class, item);
        page.add_copy(text, usable ? ui::color(quality_color(item.quality)) : ui::color::RED);
    }

    if(def.xp > 0 && character().level < max_level)
    {
        bn::string<32> text = "Experience: ";
        text += bn::to_string<8>(def.xp);
        page.add_copy(text);
    }

    if(def.money > 0)
    {
        page.add_money("Money:", def.money);
    }
}

}
