#include "gw_quests.h"

#include "bn_math.h"
#include "bn_string.h"

#include "gw_hud.h"
#include "gw_travel.h"
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

    [[nodiscard]] constexpr objective_def tame(int count, const char* name)
    {
        return { o::TAME, uint8_t(count), e::NONE, e::NONE, 0, area_id::NONE, name };
    }

    [[nodiscard]] constexpr objective_def fish(area_id area, int count, int chance, const char* name)
    {
        return { o::FISH, uint8_t(count), e::NONE, e::NONE, uint8_t(chance), area, name };
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
          n::GUARD_WF, n::GUARD_WF, 17, 16, qid::RED_LEATHER_BANDANAS,
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
          n::LEWIS, n::LEWIS, 18, 17, qid::THE_DEFIAS_BROTHERHOOD,
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
          n::GRYAN, n::GRYAN, 19, 18, qid::THE_DEFIAS_BROTHERHOOD,
          { collect(e::VANCLEEF, 1, 100, "Head of VanCleef"), none, none }, xp(19, 200), money(20),
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

        // --- Beast Mastery ----------------------------------------------------------------------------

        { "Taming the Beast",
          "Every Beast Master walks with a companion, and it's time you found yours.\n\n"
          "I'll teach you Tame Beast. Find a wild beast no stronger than you, a wolf, a spider or a boar, "
          "and hold still while it tests you. Win it over and come back to me.",
          "Tame a beast with Tame Beast, then return to Einris Brightspear.",
          "No companion yet? Wolves and spiders roam the forest outside the city.",
          "A fine companion! Now learn to call it to your side, to send it away and to bring it back "
          "when it falls.",
          n::EINRIS, n::EINRIS, 10, 10, qid::NONE, { tame(1, "Beast tamed"), none, none }, xp(10), 0,
          { i::NONE, i::NONE, i::NONE }, subclass_id::BEAST_MASTERY },

        // --- Redridge Mountains ---------------------------------------------------------------------

        { "Lakeshire Needs Aid",
          "A rider came down from Redridge last night. Orcs of the Blackrock clan are raiding "
          "Lakeshire, and Stormwind has sent no one.\n\n"
          "VanCleef can wait in his mine a little longer. Take the east road out of Elwynn and offer "
          "your sword to Magistrate Solomon.",
          "Report to Magistrate Solomon in Lakeshire, east of Elwynn Forest.",
          "",
          "Gryan sent you? Then at least one man in this kingdom remembers us. Welcome to Lakeshire.",
          n::GRYAN, n::SOLOMON, 15, 14, qid::THE_DEFIAS_BROTHERHOOD, { none, none, none }, xp(15, 40),
          money(5), { i::NONE, i::NONE, i::NONE } },

        { "Encroaching Gnolls",
          "Mongrels from the western canyons come down to the road at dusk. They took a cart and its "
          "driver two nights ago.\n\n"
          "Thin the pack before they grow bold enough to try the town.",
          "Kill 10 Redridge Mongrels in the canyons west of Lakeshire.",
          "I can still hear them howling at night.",
          "Quieter already. The carters will thank you.",
          n::BERTON, n::BERTON, 15, 14, qid::NONE,
          { kill(e::REDRIDGE_MONGREL, 10, "Redridge Mongrel slain"), none, none }, xp(15), money(15),
          { i::LAKESHIRE_GAUNTLETS, i::CANYON_WRAPS, i::MONGREL_HIDE_GLOVES } },

        { "Murloc Poachers",
          "Murlocs strip my nets every morning and carry the catch off to their huts on the shore. "
          "Spotted sunfish, the best in the lake!\n\n"
          "Get them back. The inn won't stay open on bread alone.",
          "Bring 8 Spotted Sunfish from the Murloc Flesheaters to Dockmaster Baren.",
          "Still no sunfish? The murlocs eat well, at least.",
          "That's a fine haul. Breanna will have fish on the menu tonight.",
          n::BAREN, n::BAREN, 15, 14, qid::NONE,
          { collect(e::MURLOC_FLESHEATER, 8, 60, "Spotted Sunfish"), none, none }, xp(15), money(15),
          { i::DOCKHAND_BOOTS, i::SHORELINE_SANDALS, i::POACHERS_BOOTS } },

        { "The Fishing Contest",
          "Every summer Lakeshire holds a fishing contest, orcs or no orcs. The prize is my lucky "
          "hat!\n\n"
          "Stand on the shore of Lake Everstill, face the water and press A to cast. Bring me six "
          "Redridge Goldfin and the hat is yours.",
          "Catch 6 Redridge Goldfin in Lake Everstill: face the water and press A.",
          "Patience. The goldfin bite for those who wait.",
          "Six goldfin! You're a natural. Here, wear it with pride.",
          n::BRAY, n::BRAY, 15, 14, qid::NONE,
          { fish(area_id::LAKE_EVERSTILL, 6, 60, "Redridge Goldfin"), none, none }, xp(15, 60), money(10),
          { i::LUCKY_FISHING_HAT, i::NONE, i::NONE } },

        { "Solomon's Law",
          "Shadowhide gnolls hold Alther's Mill in the north. Their mystics speak to things in the "
          "dark, and their warriors take our sheep and our people.\n\n"
          "Lakeshire's law is mine to keep. Go and enforce it.",
          "Kill 8 Shadowhide Gnolls and 6 Shadowhide Mystics at Alther's Mill.",
          "The gnolls still hold the mill.",
          "Justice is done. The mill will grind again.",
          n::SOLOMON, n::SOLOMON, 16, 15, qid::LAKESHIRE_NEEDS_AID,
          { kill(e::SHADOWHIDE_GNOLL, 8, "Shadowhide Gnoll slain"), kill(e::SHADOWHIDE_MYSTIC, 6,
            "Shadowhide Mystic slain"), none }, xp(16), money(16),
          { i::LAKESHIRE_LEGGUARDS, i::MAGISTRATE_TROUSERS, i::SHADOWHIDE_LEGGINGS } },

        { "Blackrock Menace",
          "Blackrock outrunners scout the Lakeridge Highway for the war parties behind them. Each "
          "carries a battleworn axe.\n\n"
          "Bring me their axes. Every one is a scout who won't report back to the keep.",
          "Bring 10 Battleworn Axes from the Blackrock Outrunners to Marshal Marris.",
          "The outrunners are still on the highway.",
          "Ten axes. Their chieftain is blind in the south now.",
          n::MARRIS, n::MARRIS, 16, 15, qid::LAKESHIRE_NEEDS_AID,
          { collect(e::BLACKROCK_OUTRUNNER, 10, 60, "Battleworn Axe"), none, none }, xp(16), money(16),
          { i::LAKESHIRE_LONGSWORD, i::EVERSTILL_STAFF, i::REDRIDGE_RECURVE } },

        { "Redridge Goulash",
          "My Redridge goulash is famous from here to Stormwind, but with the orcs about nobody "
          "dares go hunting.\n\n"
          "Bring me snouts from the great goretusks south of the lake and meat from the tarantulas "
          "in the hills.",
          "Bring 5 Great Goretusk Snouts and 5 Crisp Spider Meat to Chef Breanna.",
          "A goulash without snouts is just soup, dear.",
          "Perfect! Come back tonight for the first bowl.",
          n::BREANNA, n::BREANNA, 17, 15, qid::NONE,
          { collect(e::GREAT_GORETUSK, 5, 60, "Great Goretusk Snout"), collect(e::TARANTULA, 5, 60,
            "Crisp Spider Meat"), none }, xp(17), money(17),
          { i::LAKESHIRE_CHAINMAIL, i::LAKESHIRE_ROBE, i::GORETUSK_HIDE_VEST } },

        { "The Everstill Bridge",
          "The orcs broke the bridge over the lake so their war parties could raid both shores. To "
          "fix it I need iron rivets, and the Blackrock grunts in Render's Valley stole ours.\n\n"
          "Take them back.",
          "Bring 8 Iron Rivets from the Blackrock Grunts in Render's Valley to Foreman Oslow.",
          "No rivets, no bridge.",
          "Good iron, this. We start on the bridge in the morning.",
          n::OSLOW, n::OSLOW, 17, 15, qid::NONE,
          { collect(e::BLACKROCK_GRUNT, 8, 60, "Iron Rivet"), none, none }, xp(17), money(17),
          { i::IRONWORKER_HELM, i::SURVEYOR_HOOD, i::RIGGER_CAP } },

        { "Bellygrub",
          "There's a boar in the south hills the size of a hay cart. Bellygrub, the farmers call "
          "it. It has eaten three fences and a scarecrow.\n\n"
          "Bring me its tusk and I'll know it won't eat a fourth.",
          "Bring Bellygrub's Tusk to Verner Osgood.",
          "Bellygrub still roams the hills, I'd bet.",
          "That's a tusk! I'll hang it over my door.",
          n::OSGOOD, n::OSGOOD, 18, 16, qid::NONE,
          { collect(e::BELLYGRUB, 1, 100, "Bellygrub's Tusk"), none, none }, xp(18, 120), money(18),
          { i::TUSK_CLEAVER, i::OSGOOD_WALKING_STAFF, i::PIGSTICKER_BOW } },

        { "Blackrock Bounty",
          "With their scouts gone, the war party in Render's Valley is ours to break. Grunts and "
          "shadowcasters, camped in the open.\n\n"
          "Strike them before they march on the town.",
          "Kill 10 Blackrock Grunts and 6 Blackrock Shadowcasters in Render's Valley.",
          "The war party still camps in the valley.",
          "Render's Valley is clear. Lakeshire sleeps tonight.",
          n::MARRIS, n::MARRIS, 18, 16, qid::BLACKROCK_MENACE,
          { kill(e::BLACKROCK_GRUNT, 10, "Blackrock Grunt slain"), kill(e::BLACKROCK_SHADOWCASTER, 6,
            "Shadowcaster slain"), none }, xp(18), money(18),
          { i::MARRIS_SABATONS, i::LAKESHIRE_SLIPPERS, i::OUTRUNNER_BOOTS } },

        { "Stonewatch Keep",
          "Stonewatch Keep guarded these mountains for a hundred years. Now the Blackrock hold it, "
          "and their summoners burn fires in its courtyard day and night.\n\n"
          "Climb the road east of the lake and break the garrison.",
          "Kill 8 Blackrock Renegades and 6 Blackrock Summoners at Stonewatch Keep.",
          "The fires still burn at Stonewatch.",
          "The garrison is broken. Only their warlord is left.",
          n::SOLOMON, n::SOLOMON, 19, 17, qid::SOLOMONS_LAW,
          { kill(e::BLACKROCK_RENEGADE, 8, "Blackrock Renegade slain"), kill(e::BLACKROCK_SUMMONER, 6,
            "Blackrock Summoner slain"), none }, xp(19), money(19),
          { i::STONEWATCH_GAUNTLETS, i::SUMMONER_GLOVES, i::RENEGADE_GRIPS } },

        { "Wanted: Gath'Ilzogg",
          "WANTED: Gath'Ilzogg, warlord of the Blackrock in Redridge. He leads the raids from the "
          "top of Stonewatch Keep.\n\n"
          "Lakeshire pays for his head. By order of Magistrate Solomon.",
          "Bring the Head of Gath'Ilzogg to Magistrate Solomon.",
          "Gath'Ilzogg still holds the keep.",
          "The warlord is dead. Redridge is free of the Blackrock, for now. Lakeshire will never "
          "forget this.",
          n::SOLOMON, n::SOLOMON, 20, 18, qid::STONEWATCH_KEEP,
          { collect(e::GATH_ILZOGG, 1, 100, "Head of Gath'Ilzogg"), none, none }, xp(20, 200), money(20),
          { i::REDRIDGE_WARBLADE, i::STAFF_OF_LAKESHIRE, i::EVERSTILL_LONGBOW } },

        { "Return to Sentinel Hill",
          "Gryan Stoutmantle sent you to us, and you gave us back our mountains. Go back to Westfall "
          "and tell him Lakeshire stands.\n\n"
          "He has a debt to settle with VanCleef. Help him settle it.",
          "Return to Gryan Stoutmantle at Sentinel Hill in Westfall.",
          "",
          "Gath'Ilzogg dead, and the Blackrock driven off! Then you're ready for the Deadmines.",
          n::SOLOMON, n::GRYAN, 18, 18, qid::WANTED_GATH_ILZOGG, { none, none, none }, xp(18, 40),
          money(10), { i::NONE, i::NONE, i::NONE } },

        // --- Duskwood ---------------------------------------------------------------------------------

        { "The Road to Darkshire",
          "A letter came from Darkshire, in Duskwood. Lord Ello Ebonlocke writes that the dead walk "
          "out of Raven Hill, worgen hunt in the woods and the night there never lifts.\n\n"
          "Stormwind has no soldiers to spare. It has you. Take the south road out of Elwynn Forest "
          "and offer Lord Ebonlocke your sword.",
          "Report to Lord Ello Ebonlocke in Darkshire, south of Elwynn Forest.",
          "",
          "The Highlord sent a champion? Then Stormwind has not forgotten us after all. Welcome to "
          "Darkshire. Keep to the lamplight.",
          n::BOLVAR, n::ELLO, 21, 20, qid::BAZIL_THREDD, { none, none, none }, xp(21, 40), money(5),
          { i::NONE, i::NONE, i::NONE } },

        { "Wolves at Our Heels",
          "The dire wolves used to keep to the deep woods. Now they come right up to the palisade, and "
          "the rabid ones bite anything that moves.\n\n"
          "My cellar is full of meat I can't salt fast enough, and my guests won't walk the road. "
          "Thin out the packs.",
          "Kill 10 Dire Wolves and 6 Rabid Dire Wolves.",
          "I still hear them at night, scratching at the gate.",
          "That's the first quiet night in a month. Have a drink on the house.",
          n::TRELAYNE, n::TRELAYNE, 21, 19, qid::NONE,
          { kill(e::DIRE_WOLF, 10, "Dire Wolf slain"), kill(e::RABID_DIRE_WOLF, 6, "Rabid Dire Wolf slain"),
            none }, xp(21), money(21),
          { i::WOLFHEAD_HELM, i::DUSKWOOD_COWL, i::DIRE_PELT_CAP } },

        { "The Night Watch",
          "The Night Watch is all that stands between Darkshire and Raven Hill. And there aren't many "
          "of us left.\n\n"
          "Skeletons climb out of the cemetery west of here every night. Break as many as you can "
          "before they reach the road.",
          "Kill 8 Skeletal Warriors and 6 Skeletal Mages at Raven Hill Cemetery.",
          "The dead keep coming. Go back to the cemetery.",
          "Good work. You'd make a fine Watchman, if you ever tire of wandering.",
          n::ALTHEA, n::ALTHEA, 21, 20, qid::NONE,
          { kill(e::SKELETAL_WARRIOR, 8, "Skeletal Warrior slain"), kill(e::SKELETAL_MAGE, 6,
            "Skeletal Mage slain"), none }, xp(21), money(21),
          { i::NIGHT_WATCH_GAUNTLETS, i::WATCHERS_HANDWRAPS, i::DUSKWOOD_GRIPS } },

        { "Worgen in the Woods",
          "The Nightbane worgen have taken Brightwood Grove, north of the road. They were men once, "
          "or so the old tales say.\n\n"
          "Whatever they were, they hunt us now. Cut them down: the runners, the weavers who cast "
          "their shadow magic and the tainted ones who lead them.",
          "Kill 8 Nightbane Dark Runners, 6 Nightbane Shadow Weavers and 4 Nightbane Tainted Ones.",
          "The worgen still howl in the grove.",
          "Fewer howls tonight. I'll forge you something for your trouble.",
          n::CALOR, n::CALOR, 22, 20, qid::NONE,
          { kill(e::NIGHTBANE_DARK_RUNNER, 8, "Nightbane Dark Runner slain"),
            kill(e::NIGHTBANE_SHADOW_WEAVER, 6, "Nightbane Shadow Weaver slain"),
            kill(e::NIGHTBANE_TAINTED_ONE, 4, "Nightbane Tainted One slain") }, xp(22), money(22),
          { i::BRIGHTWOOD_LEGPLATES, i::WEAVERS_LEGGINGS, i::NIGHTBANE_TROUSERS } },

        { "The Legend of Stalvan",
          "Let me tell you a story the cards keep showing me. Stalvan Mistmantle was a tutor in a manor "
          "north of here. He loved a girl he could not have, and it ended in blood.\n\n"
          "The manor stands empty now. Or it should. Go and look, and tell me what you see.",
          "Visit Mistmantle Manor, north of the road, and return to Madame Eva.",
          "You haven't been to the manor yet. The cards know.",
          "So he walks there still. The cards were right, as they always are.",
          n::EVA, n::EVA, 22, 21, qid::NONE,
          { explore(area_id::MISTMANTLE_MANOR, "Mistmantle Manor visited"), none, none }, xp(22, 60),
          money(10), { i::NONE, i::NONE, i::NONE } },

        { "Stalvan Mistmantle",
          "Stalvan's spirit will not rest while his body walks. Go back to the manor and put an end to "
          "him. Be careful: hate keeps the dead strong.",
          "Kill Stalvan Mistmantle in Mistmantle Manor.",
          "He still walks. I can feel it.",
          "It's over, then. Let the poor girl rest too. Take this: it was found in the manor long ago.",
          n::EVA, n::EVA, 24, 22, qid::THE_LEGEND_OF_STALVAN,
          { kill(e::STALVAN_MISTMANTLE, 1, "Stalvan Mistmantle slain"), none, none }, xp(24, 150), money(24),
          { i::MISTMANTLE_BLADE, i::STAFF_OF_THE_MISTS, i::MISTMANTLE_LONGBOW } },

        { "The Night Watch, Part II",
          "Skeletons were only the start. Ghouls now dig through the Tranquil Gardens Cemetery in the "
          "south-east, and the plague spreaders among them make the sick worse.\n\n"
          "Clear the gardens before the sickness reaches Darkshire.",
          "Kill 8 Rotting Ghouls and 6 Plague Spreaders.",
          "The gardens are still crawling with them.",
          "The healers say the fevers are breaking. The Watch owes you again.",
          n::ALTHEA, n::ALTHEA, 23, 21, qid::THE_NIGHT_WATCH,
          { kill(e::ROTTING_GHOUL, 8, "Rotting Ghoul slain"), kill(e::PLAGUE_SPREADER, 6,
            "Plague Spreader slain"), none }, xp(23), money(23),
          { i::RAVEN_HILL_GREAVES, i::GRAVEDIGGER_SLIPPERS, i::CEMETERY_BOOTS } },

        { "Morbent's Bane",
          "Morbent Fel, the necromancer who raises the dead of Tranquil Gardens, cannot be hurt by "
          "steel or spell. Not as he is.\n\n"
          "I can brew a bane that strips his protection away. I need venom: bring me sacs from the "
          "spiders of Twilight Grove.",
          "Bring 6 Venom Web Sacs from the Venom Web Spiders to Sirra Von'Indi.",
          "The bane needs more venom.",
          "Enough venom to kill a horse. Now let me work.",
          n::SIRRA, n::SIRRA, 23, 21, qid::NONE,
          { collect(e::VENOM_WEB_SPIDER, 6, 50, "Venom Web Sac"), none, none }, xp(23), money(23),
          { i::NONE, i::NONE, i::NONE } },

        { "Morbent Fel",
          "The bane is ready, and I've poured it over your weapon. While you carry this task, his "
          "dark armor will not hold against you.\n\n"
          "Find Morbent Fel in the Tranquil Gardens Cemetery and end him.",
          "Kill Morbent Fel in the Tranquil Gardens Cemetery.",
          "Morbent Fel still lives. Don't waste the bane.",
          "You did it! The dead of the gardens will finally lie still.",
          n::SIRRA, n::SIRRA, 25, 23, qid::MORBENTS_BANE,
          { kill(e::MORBENT_FEL, 1, "Morbent Fel slain"), none, none }, xp(25, 200), money(25),
          { i::CRYPTBREAKER, i::STAFF_OF_VON_INDI, i::GRAVEWATCH_LONGBOW } },

        { "The Ogres of Vul'Gol",
          "Splinter Fist ogres came down from the mountains and took the mound at Vul'Gol, south of "
          "the road. They raid our supply carts.\n\n"
          "Darkshire can't hold against the dead and the ogres at once. Drive them out.",
          "Kill 10 Splinter Fist Ogres and 6 Splinter Fist Taskmasters at Vul'Gol.",
          "The ogres still hold the mound.",
          "Our carts reach the town again. Darkshire owes you, champion.",
          n::ELLO, n::ELLO, 24, 22, qid::THE_ROAD_TO_DARKSHIRE,
          { kill(e::SPLINTER_FIST_OGRE, 10, "Splinter Fist Ogre slain"), kill(e::SPLINTER_FIST_TASKMASTER, 6,
            "Splinter Fist Taskmaster slain"), none }, xp(24), money(24),
          { i::OGRE_CLEAVER, i::MOUND_STAFF, i::SPLINTER_BOW } },

        { "The Hermit's Errand",
          "Ah, a visitor! Old Abercrombie doesn't get many. I'm working on something, you see. "
          "Something wonderful.\n\n"
          "But I need parts. Ribs from the ghouls in the gardens, and finger bones from the skeletons on "
          "the hill. Fresh ones! Bring them and I'll make it worth your while.",
          "Bring 6 Ghoul Ribs and 6 Skeleton Fingers to Abercrombie.",
          "Ribs! Fingers! I can't finish without them.",
          "Perfect, perfect! Now off with you. Old Abercrombie has work to do.",
          n::ABERCROMBIE, n::ABERCROMBIE, 23, 21, qid::NONE,
          { collect(e::ROTTING_GHOUL, 6, 50, "Ghoul Rib", e::PLAGUE_SPREADER),
            collect(e::SKELETAL_WARRIOR, 6, 50, "Skeleton Finger", e::SKELETAL_MAGE), none }, xp(23), money(15),
          { i::NONE, i::NONE, i::NONE } },

        { "Stitches",
          "The hermit on the west road was building a monster! The Watch saw it: a thing stitched "
          "together from the dead, as big as a house, and it's walking this way.\n\n"
          "If it reaches the gate, Darkshire falls. Stop it on the road.",
          "Kill Stitches before it reaches Darkshire. It walks the road from Abercrombie's hut.",
          "Stitches is still out there. I can hear the ground shake.",
          "The beast is dead, and the hermit has fled into the night. You saved this town. Darkshire "
          "will not forget it.",
          n::ELLO, n::ELLO, 26, 23, qid::THE_HERMITS_ERRAND,
          { kill(e::STITCHES, 1, "Stitches slain"), none, none }, xp(26, 200), money(26),
          { i::NIGHT_WATCH_SHORTSWORD, i::DARKSHIRE_ROBE, i::GLOOMWOOD_LONGBOW } },

        { "Mor'Ladim",
          "Morgan Ladimore was a hero of the Second War. He came home, found his wife with another man "
          "and killed them both. Then himself.\n\n"
          "He rises from his grave on Raven Hill as Mor'Ladim and cuts down anyone he meets. Bring me "
          "his skull, and Raven Hill can rest.",
          "Bring Mor'Ladim's Skull to Sven Yorgen.",
          "Mor'Ladim still walks the hill.",
          "So he's at peace at last. And so am I, I think. Take this, it was his.",
          n::SVEN, n::SVEN, 25, 23, qid::NONE,
          { collect(e::MOR_LADIM, 1, 100, "Mor'Ladim's Skull"), none, none }, xp(25, 200), money(25),
          { i::LADIMORE_HAUBERK, i::SEXTONS_ROBE, i::RAVEN_HILL_JERKIN } },

        // --- Shadowfang Keep ----------------------------------------------------------------------------

        { "Into Shadowfang",
          "The worgen did not come from nowhere. Archmage Arugal of Dalaran called them into this "
          "world, from his keep in Silverpine Forest. Shadowfang Keep.\n\n"
          "There's no road to it from here. Felicia will fly you to the scouts' camp below the keep. "
          "Find Ranger Valdan there.",
          "Fly from Darkshire to the Scouts' Camp in Silverpine Forest and find Ranger Valdan.",
          "",
          "Ello sent you? Good. I've been watching that keep for weeks, and I could use a blade.",
          n::ELLO, n::VALDAN, 24, 22, qid::WORGEN_IN_THE_WOODS, { none, none, none }, xp(24, 40), money(10),
          { i::NONE, i::NONE, i::NONE } },

        { "The Butchers of Shadowfang",
          "Two of Arugal's beasts run the lower halls. Rethilgore keeps the dungeon where they lock "
          "up prisoners, and Razorclaw the Butcher works the kitchen. I won't say what he cooks.\n\n"
          "Kill them both.",
          "Kill Rethilgore and Razorclaw the Butcher in Shadowfang Keep.",
          "Those two still guard the lower halls.",
          "Good riddance. Take these, the camp can spare them.",
          n::VALDAN, n::VALDAN, 24, 22, qid::INTO_SHADOWFANG,
          { kill(e::RETHILGORE, 1, "Rethilgore slain"), kill(e::RAZORCLAW_THE_BUTCHER, 1,
            "Razorclaw the Butcher slain"), none }, xp(24, 150), money(24),
          { i::SHADOWFANG_GAUNTLETS, i::DALARAN_WRAPS, i::SCOUTS_GLOVES } },

        { "The Fallen Guard",
          "The keep's own guard didn't die when Arugal took it. Baron Silverlaine still holds his hall, "
          "Commander Springvale prays in the chapel to a Light that left him, and Odo the Blindwatcher "
          "keeps the tower stairs.\n\n"
          "Give them the death they were denied.",
          "Kill Baron Silverlaine, Commander Springvale and Odo the Blindwatcher.",
          "The fallen guard still stands.",
          "Silverlaine, Springvale and Odo. I knew them, once. Thank you.",
          n::VALDAN, n::VALDAN, 25, 23, qid::INTO_SHADOWFANG,
          { kill(e::BARON_SILVERLAINE, 1, "Baron Silverlaine slain"), kill(e::COMMANDER_SPRINGVALE, 1,
            "Commander Springvale slain"), kill(e::ODO_THE_BLINDWATCHER, 1, "Odo the Blindwatcher slain") },
          xp(25, 150), money(25), { i::SPRINGVALES_SABATONS, i::CHAPEL_SANDALS, i::BLINDWATCHER_BOOTS } },

        { "Arugal Must Die",
          "Arugal waits at the top of the keep, behind the ramparts. He blinks from ledge to ledge "
          "and calls his worgen to him when he's hurt.\n\n"
          "Bring me his head, and Duskwood's nights will be a little less dark.",
          "Bring the Head of Arugal to Ranger Valdan.",
          "Arugal lives, and his worgen still howl.",
          "It's done. The worgen have lost their master. Darkshire will hear of this tonight, and "
          "Stormwind soon after. Take my bow: you've earned it more than I have.",
          n::VALDAN, n::VALDAN, 26, 23, qid::INTO_SHADOWFANG,
          { collect(e::ARUGAL, 1, 100, "Head of Arugal"), none, none }, xp(26, 250), money(26),
          { i::MOONSTEEL_GREATSWORD, i::STAFF_OF_DALARAN, i::VALDANS_LONGBOW } },
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

    if(quest_state(quest).status != quest_status::NOT_STARTED || character().level < def.min_level ||
       (def.subclass != subclass_id::NONE && def.subclass != character().subclass))
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

    // Taming the Beast teaches what it asks for.
    if(quest == quest_id::TAMING_THE_BEAST)
    {
        learn_ability(ability_id::TAME_BEAST);
    }

    // No road leads to Silverpine: Into Shadowfang marks the scouts' camp on the gryphon masters' maps.
    if(quest == quest_id::INTO_SHADOWFANG)
    {
        (void) discover_flight(flight_id::SCOUTS_CAMP);
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

    if(reward != item_id::NONE && bag_row_count() >= bag_rows && item_count(reward) == 0)
    {
        return false;
    }

    if(reward != item_id::NONE)
    {
        add_item(reward);
    }

    character().money += def.money;
    quest_state(quest).status = quest_status::TURNED_IN;

    // The pet's own abilities.
    if(quest == quest_id::TAMING_THE_BEAST)
    {
        learn_ability(ability_id::CALL_PET);
        learn_ability(ability_id::REVIVE_PET);
        learn_ability(ability_id::PET_PASSIVE);
    }

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

bool quest_wants_kill(enemy_id enemy)
{
    for(int index = 1; index < int(quest_id::COUNT); ++index)
    {
        quest_id quest = quest_id(index);
        const quest_progress& progress = quest_state(quest);

        if(progress.status != quest_status::ACTIVE)
        {
            continue;
        }

        const quest_def& def = get_quest(quest);

        for(int objective_index = 0; objective_index < quest_objectives; ++objective_index)
        {
            const objective_def& objective = def.objectives[objective_index];

            if(objective.type == objective_type::KILL && objective.enemy == enemy &&
               progress.counts[objective_index] < objective.count)
            {
                return true;
            }
        }
    }

    return false;
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

bool quests_on_tame(hud& hud_ref)
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

            if(objective.type == objective_type::TAME && count < objective.count)
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

bool quests_on_fish(const map_info& map, int x, int y, hud& hud_ref)
{
    bool changed = false;
    bool wanted = false;

    // Areas overlap (a town by its lake): any area of the objective's id around the player counts.
    auto inside = [&map, x, y](area_id id)
    {
        for(const area_def& area : map.areas)
        {
            if(area.id == id && x >= area.x && y >= area.y && x < area.x + area.width && y < area.y + area.height)
            {
                return true;
            }
        }

        return false;
    };

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

            if(objective.type == objective_type::FISH && count < objective.count && inside(objective.area))
            {
                wanted = true;

                if(random_chance(objective.chance))
                {
                    ++count;
                    progress_message(hud_ref, objective, count);
                    changed = true;
                }
            }
        }

        if(complete_if_done(quest))
        {
            completed_message(hud_ref, quest);
        }
    }

    if(! changed)
    {
        if(wanted)
        {
            hud_ref.message("It got away", ui::color::WHITE);
        }
        else
        {
            hud_ref.message(random_chance(50) ? "A small fish. You let it go" : "Nothing bites",
                            ui::color::WHITE);
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
