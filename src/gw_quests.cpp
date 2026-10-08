#include "gw_quests.h"

#include "bn_math.h"
#include "bn_string.h"

#include "gw_hud.h"
#include "gw_types.h"
#include "gw_ui.h"

namespace gw
{

namespace
{
    using o = objective_type;
    using e = enemy_id;
    using n = npc_id;
    using qid = quest_id;
    using i = item_id;

    // Experience for a quest of the level, in percent of a standard quest. Later quests give a little
    // more, so following the quests reaches the Deadmines without grinding.
    [[nodiscard]] constexpr int16_t xp(int level, int percent = 100)
    {
        return int16_t((60 + 90 * level) * (100 + 3 * level) / 100 * percent / 100);
    }

    [[nodiscard]] constexpr int16_t money(int level)
    {
        return int16_t(level * level * 5 + level * 20);
    }

    constexpr objective_def none = { o::NONE, 0, e::NONE, e::NONE, 0, area_id::NONE, "" };

    [[nodiscard]] constexpr objective_def kill(e enemy, int count, const char* name)
    {
        return { o::KILL, uint8_t(count), enemy, e::NONE, 100, area_id::NONE, name };
    }

    [[nodiscard]] constexpr objective_def collect(e enemy, int count, int chance, const char* name,
                                                  e enemy2 = e::NONE)
    {
        return { o::COLLECT, uint8_t(count), enemy, enemy2, uint8_t(chance), area_id::NONE, name };
    }

    [[nodiscard]] constexpr objective_def explore(area_id area, const char* name)
    {
        return { o::EXPLORE, 1, e::NONE, e::NONE, 0, area, name };
    }

    [[nodiscard]] constexpr objective_def treasure(int count, const char* name)
    {
        return { o::TREASURE, uint8_t(count), e::NONE, e::NONE, 0, area_id::NONE, name };
    }

    constexpr quest_def quests[] = {
        { "", "", "", "", "", n::NONE, n::NONE, 0, 0, qid::NONE, { none, none, none }, 0, 0,
          { i::NONE, i::NONE, i::NONE } },

        // --- Northshire -----------------------------------------------------------------------------

        { "A Threat Within",
          "Hold, citizen. You have the look of someone who can handle a blade.\n\n"
          "Marshal McBride wants every able hand he can find. Kobolds in the mine, thieves in the "
          "vineyards... Go inside the abbey and report to him.",
          "Speak with Marshal McBride inside Northshire Abbey.",
          "",
          "So Willem sent you. Good. Northshire needs fighters, and I can use you.",
          n::WILLEM, n::MCBRIDE, 1, 1, qid::NONE, { none, none, none }, xp(1, 30), 0,
          { i::NONE, i::NONE, i::NONE } },

        { "Kobold Camp Cleanup",
          "Kobolds have dug into Echo Ridge Mine, north-west of the abbey. They steal tools, frighten "
          "the miners and breed like rats.\n\n"
          "Go down into the mine and thin their numbers.",
          "Kill 8 Kobold Vermin inside Echo Ridge Mine.",
          "Are the kobolds dealt with? The miners are waiting.",
          "Well done. The miners can work again, at least for a while.",
          n::MCBRIDE, n::MCBRIDE, 2, 1, qid::A_THREAT_WITHIN,
          { kill(e::KOBOLD_VERMIN, 8, "Kobold Vermin slain"), none, none }, xp(2), money(2),
          { i::NONE, i::NONE, i::NONE } },

        { "Wolves at the Border",
          "Wolves keep slinking over the ridge to the east and the abbey's larder is near empty.\n\n"
          "Bring me some meat from those young wolves. Tough as boots, but it makes a fine stew.",
          "Bring 6 Tough Wolf Meat to Eagan Peltskinner.",
          "Any luck with the wolves?",
          "That's plenty. The brothers will eat well tonight. Here, for your trouble.",
          n::EAGAN, n::EAGAN, 2, 1, qid::NONE,
          { collect(e::YOUNG_WOLF, 6, 70, "Tough Wolf Meat"), none, none }, xp(2), money(2),
          { i::NONE, i::NONE, i::NONE } },

        { "Milly's Harvest",
          "Those Defias thugs camped in our vineyard and carried off my grapes, crate by crate.\n\n"
          "They can't have eaten them all. Take back what they stole, please. The abbey is "
          "counting on that harvest.",
          "Recover 6 Crates of Grapes from the Defias Thugs.",
          "The grapes won't last long in their camp.",
          "Oh, thank you! Most of them aren't even bruised.",
          n::MILLY, n::MILLY, 3, 2, qid::NONE,
          { collect(e::DEFIAS_THUG, 6, 60, "Crate of Grapes"), none, none }, xp(3), money(3),
          { i::NONE, i::NONE, i::NONE } },

        { "Brotherhood of Thieves",
          "The thieves in the vineyard wear red burlap bandanas, the mark of the Defias "
          "Brotherhood.\n\n"
          "Bring me their bandanas as proof and you'll be paid from the abbey's purse.",
          "Bring 8 Red Burlap Bandanas to Deputy Willem.",
          "No bandanas, no bounty.",
          "That will teach them. Choose something from the abbey's stores.",
          n::WILLEM, n::WILLEM, 4, 3, qid::A_THREAT_WITHIN,
          { collect(e::DEFIAS_THUG, 8, 70, "Red Burlap Bandana"), none, none }, xp(4), money(4),
          { i::NORTHSHIRE_MAIL_VEST, i::NORTHSHIRE_ROBE, i::NORTHSHIRE_TUNIC } },

        { "Report to Goldshire",
          "You've done Northshire a great service. It is time you saw more of Elwynn.\n\n"
          "Take the road south to Goldshire and report to Marshal Dughan. He has more trouble "
          "than men.",
          "Report to Marshal Dughan in Goldshire.",
          "",
          "McBride speaks well of you. Good, because Goldshire needs the help.",
          n::MCBRIDE, n::DUGHAN, 5, 4, qid::KOBOLD_CAMP_CLEANUP, { none, none, none }, xp(5, 30), 0,
          { i::NONE, i::NONE, i::NONE } },

        // --- Elwynn Forest --------------------------------------------------------------------------

        { "The Fargodeep Mine",
          "Our miners fled Fargodeep Mine to the south. They speak of kobolds digging deeper "
          "tunnels, in numbers we have never seen, all the way down to the old gold vein.\n\n"
          "Go down to the deepest tunnel and see for yourself, then report back.",
          "Find the Deep Vein at the bottom of Fargodeep Mine, south of Goldshire.",
          "What did you find in the mine?",
          "As bad as that? I will send word to Stormwind. Thank you, friend.",
          n::DUGHAN, n::DUGHAN, 7, 5, qid::NONE,
          { explore(area_id::FARGODEEP, "Deep Vein found"), none, none }, xp(7, 60), money(5),
          { i::NONE, i::NONE, i::NONE } },

        { "Gold Dust Exchange",
          "Psst. The kobolds in Fargodeep pocket every speck of gold dust they dig up.\n\n"
          "You bring me their dust, I pay you. Two times what anyone else would. Probably.",
          "Bring 8 Gold Dust to Remy \"Two Times\".",
          "No dust yet? The kobolds won't hand it over themselves.",
          "Beautiful. Just beautiful. Pleasure doing business.",
          n::REMY, n::REMY, 7, 5, qid::NONE,
          { collect(e::KOBOLD_TUNNELER, 8, 60, "Gold Dust"), none, none }, xp(7), money(9),
          { i::NONE, i::NONE, i::NONE } },

        { "Bounty on Murlocs",
          "Murlocs have taken over Crystal Lake, east of the Fargodeep road. Fishermen won't go "
          "near the water anymore.\n\n"
          "The town pays a bounty on every murloc fin. Interested?",
          "Bring 8 Torn Murloc Fins to Innkeeper Farley.",
          "The murlocs are still croaking at the lake, I hear.",
          "Ugh, they smell worse than they look. Here's your bounty.",
          n::FARLEY, n::FARLEY, 8, 6, qid::NONE,
          { collect(e::MURLOC, 8, 60, "Torn Murloc Fin"), none, none }, xp(8), money(8),
          { i::NONE, i::NONE, i::NONE } },

        { "Protect the Frontier",
          "Timber wolves prowl the woods around Goldshire and forest spiders nest in the trees. "
          "Travellers keep going missing.\n\n"
          "Thin out both and the roads will be safe again.",
          "Kill 6 Timber Wolves and 5 Forest Spiders.",
          "The woods are still crawling with beasts.",
          "The roads feel safer already. Well done.",
          n::GUARD_GS, n::GUARD_GS, 7, 5, qid::NONE,
          { kill(e::TIMBER_WOLF, 6, "Timber Wolf slain"), kill(e::FOREST_SPIDER, 5, "Forest Spider slain"),
            none }, xp(7), money(7),
          { i::GUARD_LEGGINGS, i::WOVEN_LEGGINGS, i::FORESTER_PANTS } },

        { "Chunks of Boar Meat",
          "Pa and the boys work the fields all day and they eat like bears. The boars in the "
          "south field are fat this year.\n\n"
          "Bring me some meat and I'll make a stew that will stick to your ribs.",
          "Bring 6 Chunks of Boar Meat to Ma Stonefield.",
          "Hungry mouths are waiting, dear.",
          "Lovely. Now, there's one boar I still need you for...",
          n::MA_STONEFIELD, n::MA_STONEFIELD, 8, 6, qid::NONE,
          { collect(e::BOAR, 6, 70, "Chunk of Boar Meat"), none, none }, xp(8), money(8),
          { i::NONE, i::NONE, i::NONE } },

        { "Princess Must Die!",
          "There's a sow called Princess south of the farm. She was our prize pig until she went "
          "wild and started eating our crops. She's huge and mean.\n\n"
          "Bring me her brass collar. Be careful, she won't go quietly.",
          "Bring Princess's Brass Collar to Ma Stonefield.",
          "Princess still has her collar, I take it?",
          "The poor beast. Still, our crops are safe now. Thank you.",
          n::MA_STONEFIELD, n::MA_STONEFIELD, 10, 8, qid::BOAR_MEAT,
          { collect(e::PRINCESS, 1, 100, "Brass Collar"), none, none }, xp(10, 150), money(10),
          { i::PIG_IRON_GAUNTLETS, i::FARMHAND_GLOVES, i::BOARHIDE_GLOVES } },

        { "The Riverpaw Threat",
          "Riverpaw gnolls have crossed from Westfall into the woods south of this road. They "
          "raid farms and grow bolder each night.\n\n"
          "Drive them back. Kill as many as you can.",
          "Kill 10 Riverpaw Gnolls at Forest's Edge.",
          "The gnolls are still out there.",
          "That will make them think twice. Watch out for their leader, Hogger.",
          n::GUARD_WEST, n::GUARD_WEST, 9, 7, qid::NONE,
          { kill(e::RIVERPAW_GNOLL, 10, "Riverpaw Gnoll slain"), none, none }, xp(9), money(9),
          { i::RIVERPAW_CLEAVER, i::TRIBAL_STAFF, i::GNOLL_HUNTING_BOW } },

        { "Wanted: Hogger",
          "WANTED: Hogger, a huge gnoll leading the Riverpaw raids at Forest's Edge. He has "
          "killed guards and farmers alike.\n\n"
          "A big reward awaits whoever brings back his claw. He is dangerous: do not fight him "
          "unprepared.",
          "Bring Hogger's Huge Gnoll Claw to Marshal Dughan.",
          "Hogger still walks free?",
          "Hogger's claw! You have done what my guards could not. Take your reward, hero.",
          n::DUGHAN, n::DUGHAN, 11, 9, qid::NONE,
          { collect(e::HOGGER, 1, 100, "Huge Gnoll Claw"), none, none }, xp(11, 150), money(12),
          { i::MARSHALS_GREATSWORD, i::STAFF_OF_ELWYNN, i::ELWYNN_LONGBOW } },

        { "Report to Gryan",
          "Westfall, west of Elwynn, has been abandoned by Stormwind. Farmers have formed the "
          "People's Militia under Gryan Stoutmantle at Sentinel Hill.\n\n"
          "They need fighters like you. Take the road west and find him.",
          "Report to Gryan Stoutmantle at Sentinel Hill in Westfall.",
          "",
          "Dughan sent you? Then you're welcome here. We need every sword we can get.",
          n::DUGHAN, n::GRYAN, 10, 10, qid::NONE, { none, none, none }, xp(10, 30), 0,
          { i::NONE, i::NONE, i::NONE } },

        // --- Westfall -------------------------------------------------------------------------------

        { "The People's Militia",
          "The Defias Brotherhood holds Moonbrook, in the south. Their trappers and smugglers "
          "roam the roads, robbing anyone who passes.\n\n"
          "Strike at them. Show them the militia can bite.",
          "Kill 8 Defias Trappers and 6 Defias Smugglers.",
          "Moonbrook is still crawling with Defias.",
          "You fight like ten militiamen. The Brotherhood will feel that.",
          n::GRYAN, n::GRYAN, 13, 11, qid::REPORT_TO_GRYAN,
          { kill(e::DEFIAS_TRAPPER, 8, "Defias Trapper slain"), kill(e::DEFIAS_SMUGGLER, 6,
            "Defias Smuggler slain"), none }, xp(13), money(13),
          { i::MILITIA_CHAINMAIL, i::MILITIA_ROBE, i::MILITIA_JERKIN } },

        { "The Harvest Watchers",
          "Someone set those metal scarecrows loose on our farm. Harvest watchers, they're called. "
          "They walk the fields and attack anything that moves.\n\n"
          "Smash them so we can go home.",
          "Destroy 10 Harvest Watchers on the farms of Westfall.",
          "They still stomp through our wheat.",
          "Scrap metal, all of them! Thank you, thank you.",
          n::SALMA, n::SALMA, 12, 10, qid::NONE,
          { kill(e::HARVEST_WATCHER, 10, "Harvest Watcher destroyed"), none, none }, xp(12), money(12),
          { i::HARVESTER_BOOTS, i::HARVESTER_SLIPPERS, i::HARVESTER_MOCCASINS } },

        { "Red Leather Bandanas",
          "Every Defias trapper and smuggler wears a red leather bandana. The militia pays for "
          "each one: proof of a thief who will rob no more.",
          "Bring 10 Red Leather Bandanas to Scout Galiaan.",
          "I count no bandanas.",
          "Ten fewer thieves on the road. Good work.",
          n::GUARD_WF, n::GUARD_WF, 13, 11, qid::NONE,
          { collect(e::DEFIAS_TRAPPER, 10, 50, "Red Leather Bandana", e::DEFIAS_SMUGGLER), none, none },
          xp(13), money(13),
          { i::MILITIA_HELM, i::SEER_HOOD, i::SCOUT_HOOD } },

        { "Riverpaw Gnoll Bounty",
          "Gnolls drove us off our land, the brutes. Big ones, painted with war colors.\n\n"
          "The militia pays for their painted armbands. Bring some to me and I'll see you get "
          "paid.",
          "Bring 8 Painted Gnoll Armbands to Farmer Furlbrow.",
          "Those gnolls are still out there.",
          "Ha! That's for my barn. Here's your bounty.",
          n::FURLBROW, n::FURLBROW, 15, 13, qid::NONE,
          { collect(e::GNOLL_BRUTE, 8, 60, "Painted Gnoll Armband"), none, none }, xp(15), money(15),
          { i::WESTFALL_WARHAMMER, i::FURLBROW_STAFF, i::WESTFALL_SHORTBOW } },

        { "The Defias Brotherhood",
          "The Defias are led by someone clever, and they hide something in Moonbrook. Our "
          "scouts never came back.\n\n"
          "Go to the ruins of Moonbrook in the south and find their hideout.",
          "Explore Moonbrook and find the Defias hideout.",
          "Have you been to Moonbrook?",
          "A mine under Moonbrook... The Deadmines. So that is where they hide. Their leader, "
          "Edwin VanCleef, must be down there.",
          n::GRYAN, n::GRYAN, 15, 14, qid::THE_PEOPLES_MILITIA,
          { explore(area_id::MOONBROOK, "Defias hideout found"), none, none }, xp(15, 60), money(10),
          { i::NONE, i::NONE, i::NONE } },

        // --- The Deadmines --------------------------------------------------------------------------

        { "Red Silk Bandanas",
          "The Defias deep in their mine wear red silk, not leather. Bring me their bandanas and "
          "I'll make you a rich adventurer.",
          "Bring 8 Red Silk Bandanas from the Deadmines to Scout Galiaan.",
          "The Deadmines are dangerous. Take your time.",
          "Silk! So you made it into their mine. Impressive.",
          n::GUARD_WF, n::GUARD_WF, 17, 15, qid::RED_LEATHER_BANDANAS,
          { collect(e::DEFIAS_MINER, 8, 60, "Red Silk Bandana", e::DEFIAS_PIRATE), none, none },
          xp(17), money(17),
          { i::MILITIA_LEGPLATES, i::SILK_TROUSERS, i::DEFIAS_LEGGINGS } },

        { "Sneed's Shredder",
          "A goblin called Sneed builds machines for the Defias in their mine. His shredder "
          "could tear through our walls in an afternoon.\n\n"
          "Find Sneed in the Deadmines and destroy him and his machine.",
          "Kill Sneed in the Deadmines.",
          "Sneed's still working on his machines, I'd wager.",
          "Without Sneed the Defias will have no more machines. Well fought.",
          n::LEWIS, n::LEWIS, 18, 16, qid::THE_DEFIAS_BROTHERHOOD,
          { kill(e::SNEED, 1, "Sneed slain"), none, none }, xp(18, 200), money(18),
          { i::SHREDDER_GAUNTLETS, i::TINKER_GLOVES, i::MECHANIC_GLOVES } },

        { "Edwin VanCleef",
          "Edwin VanCleef was a stonemason who built Stormwind, then was cheated of his pay. Now "
          "he leads the Defias Brotherhood from a ship hidden at the end of the Deadmines.\n\n"
          "End this. Bring me his head and Westfall will be free.",
          "Bring the Head of VanCleef to Gryan Stoutmantle.",
          "VanCleef still lives. Westfall waits.",
          "So falls Edwin VanCleef. Westfall is free, and every farmer here owes you their life.\n\n"
          "But he carried a letter I don't like the look of. Hear me out before you rest.",
          n::GRYAN, n::GRYAN, 20, 17, qid::THE_DEFIAS_BROTHERHOOD,
          { collect(e::VANCLEEF, 1, 100, "Head of VanCleef"), none, none }, xp(20, 200), money(20),
          { i::CHAUSSES_OF_WESTFALL, i::TUNIC_OF_WESTFALL, i::STAFF_OF_WESTFALL } },

        // --- Stormwind ------------------------------------------------------------------------------

        { "The Road to Stormwind",
          "Stormwind must hear what is happening in Elwynn: thieves in the vineyards, kobolds in "
          "every mine, gnolls at the border.\n\n"
          "Take the road north-west from Goldshire to the city gates and give my report to General "
          "Marcus Jonathan.",
          "Report to General Marcus Jonathan at the gates of Stormwind.",
          "",
          "Dughan's report, at last. Welcome to Stormwind, friend. The Highlord will want to hear "
          "this from you himself.",
          n::DUGHAN, n::MARCUS_JONATHAN, 6, 5, qid::NONE, { none, none, none }, xp(6, 40), money(3),
          { i::NONE, i::NONE, i::NONE } },

        { "An Audience with the Highlord",
          "Highlord Bolvar Fordragon rules in the king's stead, and he asked to see anyone who comes "
          "from the troubled lands of Elwynn.\n\n"
          "Cross the canal and the Trade District to the keep, in the north-west of the city.",
          "Speak with Highlord Bolvar Fordragon in front of Stormwind Keep.",
          "",
          "So the Defias grow bold right under our walls. You did well to come, and Stormwind will "
          "not forget it.",
          n::MARCUS_JONATHAN, n::BOLVAR, 6, 5, qid::THE_ROAD_TO_STORMWIND, { none, none, none }, xp(6, 40),
          money(6), { i::NONE, i::NONE, i::NONE } },

        { "Lost Treasures",
          "Smugglers, miners and adventurers hid chests all over these lands, lad. Behind the trees, "
          "at the end of old mine tunnels, in corners nobody bothers to look.\n\n"
          "Find five of them. Keep what's inside, I only want to know where they were. The "
          "Explorers' League pays well for a good map.",
          "Open 5 hidden treasure chests anywhere in the world.",
          "Found them yet? Look where nobody else would.",
          "Five chests! You have the nose of a true explorer. Take these, they'll carry you far.",
          n::BRANN, n::BRANN, 10, 6, qid::NONE,
          { treasure(5, "Treasure chests opened"), none, none }, xp(10), money(10),
          { i::PATHFINDER_GREAVES, i::WANDERER_SANDALS, i::TRAILBLAZER_BOOTS } },

        // --- The Stockade ---------------------------------------------------------------------------

        { "The Unsent Letter",
          "VanCleef never sent this letter. It is sealed with the Brotherhood's mark and addressed to "
          "one Bazil Thredd... a prisoner in the Stockade, inside Stormwind itself.\n\n"
          "If the Defias have friends in the city's own prison, Stormwind must know. Take it to Warden "
          "Thelwater at the Stockade, in the Mage Quarter.",
          "Take the Unsent Letter to Warden Thelwater at the Stockade in Stormwind.",
          "",
          "A letter from VanCleef to Bazil Thredd? Light help us. The prisoners rose this very "
          "morning, and Thredd is the one leading them.",
          n::GRYAN, n::THELWATER, 20, 18, qid::EDWIN_VANCLEEF, { none, none, none }, xp(20, 40), money(5),
          { i::NONE, i::NONE, i::NONE } },

        { "The Stockade Riots",
          "The prisoners broke out of their cells and hold the whole Stockade. My guards are dead or "
          "locked in with them.\n\n"
          "Go in and put the riot down. And Targorr the Dread is loose in the west hall: he killed "
          "two of my men with his bare hands. Make sure he never does it again.",
          "Kill 10 Defias Convicts, 8 Defias Insurgents and Targorr the Dread in the Stockade.",
          "The riot still rages. I can hear it from out here.",
          "Quiet, at last. You've done what the whole city watch couldn't.",
          n::THELWATER, n::THELWATER, 20, 18, qid::THE_UNSENT_LETTER,
          { kill(e::DEFIAS_CONVICT, 10, "Defias Convict slain"), kill(e::DEFIAS_INSURGENT, 8,
            "Defias Insurgent slain"), kill(e::TARGORR, 1, "Targorr the Dread slain") }, xp(20), money(20),
          { i::RIOTGUARD_HELM, i::JAILERS_COWL, i::TURNKEY_CAP } },

        { "Bazil Thredd",
          "Bazil Thredd planned this riot with VanCleef. He holed up in my own hall at the back of the "
          "Stockade, past the east cells.\n\n"
          "End it. Bring his head to Highlord Bolvar at the keep: the Highlord will want to see with "
          "his own eyes that the last of the Brotherhood is finished.",
          "Bring the Head of Bazil Thredd to Highlord Bolvar Fordragon.",
          "Thredd still lives? Then the Brotherhood still has a head.",
          "So VanCleef's last friend in Stormwind is dead, and the Defias Brotherhood with him. You "
          "have saved this city as surely as Westfall. Kneel, champion: Stormwind will remember your "
          "name.",
          n::THELWATER, n::BOLVAR, 21, 18, qid::THE_UNSENT_LETTER,
          { collect(e::BAZIL_THREDD, 1, 100, "Head of Bazil Thredd"), none, none }, xp(21, 200), money(25),
          { i::LIONHEART_BLADE, i::STAFF_OF_THE_LION, i::LIONHEART_LONGBOW } },
    };

    static_assert(sizeof(quests) / sizeof(quests[0]) == int(quest_id::COUNT));
    static_assert(int(quest_id::COUNT) <= max_quests);

    [[nodiscard]] bool complete_if_done(quest_id quest)
    {
        quest_progress& progress = quest_state(quest);

        if(progress.status == quest_status::ACTIVE && quest_objectives_done(quest))
        {
            progress.status = quest_status::COMPLETE;
            return true;
        }

        return false;
    }

    void progress_message(hud& hud_ref, const objective_def& objective, int count)
    {
        bn::string<48> text = objective.name;

        if(objective.count > 1)
        {
            text += ": ";
            text += bn::to_string<4>(count);
            text += "/";
            text += bn::to_string<4>(objective.count);
        }

        hud_ref.message(text, ui::color::YELLOW);
    }

    void completed_message(hud& hud_ref, quest_id quest)
    {
        bn::string<48> text = get_quest(quest).title;
        text += " complete";
        hud_ref.message(text, ui::color::GREEN);
    }
}

const quest_def& get_quest(quest_id quest)
{
    int index = int(quest);
    return quests[index < int(quest_id::COUNT) ? index : 0];
}

quest_progress& quest_state(quest_id quest)
{
    return character().quests[int(quest)];
}

bool quest_available(quest_id quest)
{
    if(quest == quest_id::NONE)
    {
        return false;
    }

    const quest_def& def = get_quest(quest);

    if(quest_state(quest).status != quest_status::NOT_STARTED || character().level < def.min_level)
    {
        return false;
    }

    return def.previous == quest_id::NONE || quest_state(def.previous).status == quest_status::TURNED_IN;
}

int quest_objective_count(const quest_def& quest)
{
    int result = 0;

    for(const objective_def& objective : quest.objectives)
    {
        if(objective.type != objective_type::NONE)
        {
            ++result;
        }
    }

    return result;
}

bool quest_objectives_done(quest_id quest)
{
    const quest_def& def = get_quest(quest);
    const quest_progress& progress = quest_state(quest);

    for(int index = 0; index < quest_objectives; ++index)
    {
        const objective_def& objective = def.objectives[index];

        if(objective.type != objective_type::NONE && progress.counts[index] < objective.count)
        {
            return false;
        }
    }

    return true;
}

void accept_quest(quest_id quest)
{
    quest_progress& progress = quest_state(quest);
    progress = quest_progress();
    progress.status = quest_status::ACTIVE;

    const quest_def& def = get_quest(quest);

    for(int index = 0; index < quest_objectives; ++index)
    {
        if(def.objectives[index].type == objective_type::TREASURE)
        {
            progress.counts[index] = uint8_t(bn::min(opened_chest_count(), int(def.objectives[index].count)));
        }
    }

    (void) complete_if_done(quest);
}

bool turn_in_quest(quest_id quest, int reward_index)
{
    const quest_def& def = get_quest(quest);
    item_id reward = item_id::NONE;

    if(reward_index >= 0 && reward_index < quest_rewards)
    {
        reward = def.rewards[reward_index];
    }

    if(reward != item_id::NONE && free_bag_slots() == 0 && item_count(reward) == 0)
    {
        return false;
    }

    if(reward != item_id::NONE)
    {
        add_item(reward);
    }

    character().money += def.money;
    quest_state(quest).status = quest_status::TURNED_IN;
    return true;
}

void abandon_quest(quest_id quest)
{
    quest_state(quest) = quest_progress();
}

int quest_log_count()
{
    int result = 0;

    for(int index = 1; index < int(quest_id::COUNT); ++index)
    {
        quest_status status = character().quests[index].status;

        if(status == quest_status::ACTIVE || status == quest_status::COMPLETE)
        {
            ++result;
        }
    }

    return result;
}

quest_id quest_log_at(int log_index)
{
    for(int index = 1; index < int(quest_id::COUNT); ++index)
    {
        quest_status status = character().quests[index].status;

        if(status == quest_status::ACTIVE || status == quest_status::COMPLETE)
        {
            if(log_index == 0)
            {
                return quest_id(index);
            }

            --log_index;
        }
    }

    return quest_id::NONE;
}

int npc_quests(npc_id npc, quest_id* out, int max_count)
{
    int count = 0;

    // Quests to turn in first, then new quests, then quests still in progress.
    for(int pass = 0; pass < 3; ++pass)
    {
        for(int index = 1; index < int(quest_id::COUNT) && count < max_count; ++index)
        {
            quest_id quest = quest_id(index);
            const quest_def& def = get_quest(quest);
            quest_status status = quest_state(quest).status;
            bool match = false;

            if(pass == 0)
            {
                match = def.ender == npc && status == quest_status::COMPLETE;
            }
            else if(pass == 1)
            {
                match = def.giver == npc && quest_available(quest);
            }
            else
            {
                match = def.ender == npc && status == quest_status::ACTIVE;
            }

            if(match)
            {
                out[count++] = quest;
            }
        }
    }

    return count;
}

quest_marker npc_quest_marker(npc_id npc)
{
    quest_marker result = quest_marker::NONE;

    for(int index = 1; index < int(quest_id::COUNT); ++index)
    {
        quest_id quest = quest_id(index);
        const quest_def& def = get_quest(quest);
        quest_status status = quest_state(quest).status;

        if(def.ender == npc && status == quest_status::COMPLETE)
        {
            return quest_marker::COMPLETE;
        }

        if(def.giver == npc && quest_available(quest))
        {
            result = quest_marker::AVAILABLE;
        }
        else if(def.ender == npc && status == quest_status::ACTIVE && result == quest_marker::NONE)
        {
            result = quest_marker::IN_PROGRESS;
        }
    }

    return result;
}

uint8_t level_color(int level)
{
    int difference = level - character().level;

    if(is_gray(level))
    {
        return uint8_t(ui::color::GRAY);
    }

    if(difference >= 3)
    {
        return uint8_t(ui::color::RED);
    }

    return uint8_t(difference >= -2 ? ui::color::YELLOW : ui::color::GREEN);
}

bool quests_on_kill(enemy_id enemy, hud& hud_ref)
{
    bool changed = false;

    for(int index = 1; index < int(quest_id::COUNT); ++index)
    {
        quest_id quest = quest_id(index);
        quest_progress& progress = quest_state(quest);

        if(progress.status != quest_status::ACTIVE)
        {
            continue;
        }

        const quest_def& def = get_quest(quest);

        for(int objective_index = 0; objective_index < quest_objectives; ++objective_index)
        {
            const objective_def& objective = def.objectives[objective_index];
            uint8_t& count = progress.counts[objective_index];

            if(count >= objective.count || (objective.enemy != enemy && objective.enemy2 != enemy) ||
               objective.enemy == enemy_id::NONE)
            {
                continue;
            }

            if(objective.type == objective_type::KILL ||
               (objective.type == objective_type::COLLECT && random_chance(objective.chance)))
            {
                ++count;
                progress_message(hud_ref, objective, count);
                changed = true;
            }
        }

        if(complete_if_done(quest))
        {
            completed_message(hud_ref, quest);
        }
    }

    return changed;
}

bool quests_on_explore(const map_info& map, int x, int y, hud& hud_ref)
{
    bool changed = false;

    for(const area_def& area : map.areas)
    {
        if(area.id == area_id::NONE || x < area.x || y < area.y || x >= area.x + area.width ||
           y >= area.y + area.height)
        {
            continue;
        }

        for(int index = 1; index < int(quest_id::COUNT); ++index)
        {
            quest_id quest = quest_id(index);
            quest_progress& progress = quest_state(quest);

            if(progress.status != quest_status::ACTIVE)
            {
                continue;
            }

            const quest_def& def = get_quest(quest);

            for(int objective_index = 0; objective_index < quest_objectives; ++objective_index)
            {
                const objective_def& objective = def.objectives[objective_index];

                if(objective.type == objective_type::EXPLORE && objective.area == area.id &&
                   progress.counts[objective_index] < objective.count)
                {
                    progress.counts[objective_index] = objective.count;
                    progress_message(hud_ref, objective, objective.count);
                    changed = true;
                }
            }

            if(complete_if_done(quest))
            {
                completed_message(hud_ref, quest);
            }
        }
    }

    return changed;
}

bool quests_on_chest(hud& hud_ref)
{
    bool changed = false;

    for(int index = 1; index < int(quest_id::COUNT); ++index)
    {
        quest_id quest = quest_id(index);
        quest_progress& progress = quest_state(quest);

        if(progress.status != quest_status::ACTIVE)
        {
            continue;
        }

        const quest_def& def = get_quest(quest);

        for(int objective_index = 0; objective_index < quest_objectives; ++objective_index)
        {
            const objective_def& objective = def.objectives[objective_index];
            uint8_t& count = progress.counts[objective_index];

            if(objective.type == objective_type::TREASURE && count < objective.count)
            {
                ++count;
                progress_message(hud_ref, objective, count);
                changed = true;
            }
        }

        if(complete_if_done(quest))
        {
            completed_message(hud_ref, quest);
        }
    }

    return changed;
}

}
