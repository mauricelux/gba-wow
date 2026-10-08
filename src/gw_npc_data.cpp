#include "gw_npc_data.h"

namespace gw
{

namespace
{
    using l = look_id;
    using c = class_id;

    constexpr uint8_t vendor = npc_flag::VENDOR;
    constexpr uint8_t trainer = npc_flag::TRAINER;
    constexpr uint8_t innkeeper = npc_flag::INNKEEPER | npc_flag::VENDOR;

    // Vendor stock lists, see gw_vendors.
    constexpr uint8_t general_goods = 0;
    constexpr uint8_t weapons = 1;
    constexpr uint8_t armor = 2;
    constexpr uint8_t inn = 3;
    constexpr uint8_t militia = 4;

    constexpr npc_info npcs[] = {
        { "", "", l::PEASANT, 0, c::WARRIOR, 0, "" },
        { "Deputy Willem", "", l::GUARD, 0, c::WARRIOR, 0,
          "Stay alert, citizen. The Defias grow bolder every day." },
        { "Marshal McBride", "", l::MARSHAL, 0, c::WARRIOR, 0,
          "The Light watches over Northshire, but it needs strong arms too." },
        { "Llane Beshere", "Warrior Trainer", l::TRAINER_WARRIOR, trainer, c::WARRIOR, 0,
          "Steel and rage, friend. Let me teach you to use both." },
        { "Brother Danil", "General Supplies", l::PRIEST, vendor, c::WARRIOR, general_goods,
          "Welcome, child. Rest a while and take what you need." },
        { "Northshire Guard", "", l::GUARD, 0, c::WARRIOR, 0,
          "The road south leads to Goldshire. Keep your blade ready." },
        { "Marshal Dughan", "", l::MARSHAL, 0, c::WARRIOR, 0,
          "Goldshire is under my protection. Gnolls and kobolds test that every day." },
        { "Lyria Du Lac", "Warrior Trainer", l::TRAINER_WARRIOR_F, trainer, c::WARRIOR, 0,
          "A warrior is never done learning. What do you need?" },
        { "Corina Steele", "Weaponsmith", l::SMITH, vendor, c::WARRIOR, weapons,
          "Finest blades this side of Stormwind. Have a look." },
        { "Innkeeper Farley", "Innkeeper", l::INNKEEPER, innkeeper, c::WARRIOR, inn,
          "Welcome to the Lion's Pride! Something to eat or drink?" },
        { "Remy \"Two Times\"", "", l::MERCHANT, 0, c::WARRIOR, 0,
          "Two times the gold, two times the fun. That's my motto." },
        { "Guard Thomas", "", l::GUARD, 0, c::WARRIOR, 0,
          "Wolves and spiders keep creeping out of the woods." },
        { "Deputy Rainer", "", l::GUARD, 0, c::WARRIOR, 0,
          "West of here lies Westfall. Gnolls lurk in the woods to the south." },
        { "Ma Stonefield", "", l::FARMER_F, 0, c::WARRIOR, 0,
          "Pa's out in the fields. Mind the boars, they're ornery this year." },
        { "Gryan Stoutmantle", "People's Militia", l::MILITIA, 0, c::WARRIOR, 0,
          "Stormwind abandoned Westfall. The militia is all that stands against the Defias." },
        { "Salma Saldean", "", l::FARMER_F2, 0, c::WARRIOR, 0,
          "Our farm was overrun by those metal scarecrows. What a mess." },
        { "Scout Galiaan", "People's Militia", l::MILITIA, 0, c::WARRIOR, 0,
          "I scout the roads for the militia. There are Defias everywhere." },
        { "Khelden Bremen", "Mage Trainer", l::MAGE_TRAINER, trainer, c::MAGE, 0,
          "Magic is a discipline, not a toy. Are you ready to learn?" },
        { "Thorgas Grimson", "Hunter Trainer", l::HUNTER_TRAINER, trainer, c::HUNTER, 0,
          "A steady hand and a keen eye. That's all a hunter needs." },
        { "Eagan Peltskinner", "", l::PEASANT, 0, c::WARRIOR, 0,
          "Wolves keep coming over the ridge, and I'm short on meat." },
        { "Milly Osworth", "", l::FARMER_F2, 0, c::WARRIOR, 0,
          "The vineyard was my family's pride, until the Defias came." },
        { "Andrew Krighton", "Armorer", l::SMITH, vendor, c::WARRIOR, armor,
          "Armor for the road ahead. Cloth, leather, mail, all at fair prices." },
        { "Farmer Furlbrow", "", l::PEASANT, 0, c::WARRIOR, 0,
          "The Defias took our farm, and now gnolls roam the fields." },
        { "Quartermaster Lewis", "Militia Supplies", l::MERCHANT, vendor, c::WARRIOR, militia,
          "The militia needs supplies, and so will you. Take a look." },
        { "Innkeeper Heather", "Innkeeper", l::INNKEEPER, innkeeper, c::WARRIOR, inn,
          "No roof yet, but a warm fire and a hot meal. Rest here as long as you like." },
    };

    static_assert(sizeof(npcs) / sizeof(npcs[0]) == int(npc_id::COUNT));
}

const npc_info& get_npc_info(npc_id npc)
{
    int index = int(npc);
    return npcs[index < int(npc_id::COUNT) ? index : 0];
}

}
