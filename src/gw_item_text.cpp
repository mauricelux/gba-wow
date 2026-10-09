#include "gw_item_text.h"

#include "bn_string.h"

#include "gw_character.h"
#include "gw_homes.h"
#include "gw_items.h"
#include "gw_text_page.h"

namespace gw
{

namespace
{
    void add_stat(bn::string<64>& text, int value, const char* name)
    {
        if(value)
        {
            if(! text.empty())
            {
                text += " ";
            }

            text += value > 0 ? "+" : "";
            text += bn::to_string<4>(value);
            text += " ";
            text += name;
        }
    }

    // The subclasses that want the item's stats, a bit per subclass_id: strength for warriors,
    // agility for hunters and Fury, intellect for mages, shields for Protection.
    [[nodiscard]] int suited_subclasses(const item_def& def)
    {
        auto bit = [](subclass_id subclass)
        {
            return 1 << int(subclass);
        };

        int result = 0;

        if(def.type == item_type::SHIELD)
        {
            result |= bit(subclass_id::PROTECTION);
        }

        if(def.strength > 0)
        {
            result |= bit(subclass_id::ARMS) | bit(subclass_id::FURY) | bit(subclass_id::PROTECTION);
        }

        if(def.agility > 0)
        {
            result |= bit(subclass_id::FURY) | bit(subclass_id::BEAST_MASTERY) | bit(subclass_id::MARKSMANSHIP) |
                      bit(subclass_id::SURVIVAL);
        }

        if(def.intellect > 0)
        {
            result |= bit(subclass_id::ARCANE) | bit(subclass_id::FIRE) | bit(subclass_id::FROST);
        }

        return result;
    }

    [[nodiscard]] const char* slot_name(equip_slot slot)
    {
        switch(slot)
        {

        case equip_slot::HEAD:
            return "head";

        case equip_slot::CHEST:
            return "chest";

        case equip_slot::HANDS:
            return "hands";

        case equip_slot::LEGS:
            return "legs";

        case equip_slot::FEET:
            return "feet";

        default:
            return "";
        }
    }
}

const char* item_kind_name(item_id item)
{
    const item_def& def = get_item(item);

    switch(def.type)
    {

    case item_type::JUNK:
        return "Junk";

    case item_type::QUEST:
        return "Quest item";

    case item_type::REAGENT:
        return "Reagent";

    case item_type::HEARTHSTONE:
        return "Use: return home";

    case item_type::FOOD:
        return "Food";

    case item_type::DRINK:
        return "Drink";

    case item_type::POTION:
        return "Potion";

    case item_type::SHIELD:
        return "Shield";

    case item_type::SWORD:
        return "Sword";

    case item_type::AXE:
        return "Axe";

    case item_type::MACE:
        return "Mace";

    case item_type::DAGGER:
        return "Dagger";

    case item_type::STAFF:
        return "Staff, two-hand";

    case item_type::TWO_HANDED:
        return "Two-hand weapon";

    case item_type::BOW:
        return "Bow";

    case item_type::GUN:
        return "Gun";

    case item_type::WAND:
        return "Wand";

    default:
        break;
    }

    return "Armor";
}

void add_item_details(text_page& page, item_id item)
{
    if(item == item_id::NONE)
    {
        return;
    }

    const item_def& def = get_item(item);
    const character_data& data = character();
    bn::string<64> text;

    switch(def.type)
    {

    case item_type::CLOTH:
    case item_type::LEATHER:
    case item_type::MAIL:
        text = def.type == item_type::CLOTH ? "Cloth " : def.type == item_type::LEATHER ? "Leather " : "Mail ";
        text += slot_name(def.slot);
        text += ", ";
        text += bn::to_string<6>(def.armor);
        text += " armor";
        break;

    case item_type::SHIELD:
        text = "Shield, ";
        text += bn::to_string<6>(def.armor);
        text += " armor";
        break;

    case item_type::FOOD:
    case item_type::DRINK:
    case item_type::POTION:
        text = item_kind_name(item);
        text += ": ";
        text += bn::to_string<6>(def.min_damage);
        text += def.type == item_type::DRINK ? " mana" : " health";
        break;

    case item_type::JUNK:
    case item_type::QUEST:
    case item_type::REAGENT:
        text = item_kind_name(item);
        break;

    case item_type::HEARTHSTONE:
    {
        text = "Takes you to ";
        text += get_home(home_id(data.home)).name;
        page.add_copy(text, ui::color::WHITE);

        if(data.play_frames < data.hearthstone_ready)
        {
            int minutes = int((data.hearthstone_ready - data.play_frames) / 3600) + 1;
            text = "Ready in ";
            text += bn::to_string<4>(minutes);
            text += " min";
            page.add_copy(text, ui::color::RED);
        }
        else
        {
            page.add("Ready", ui::color::GREEN);
        }

        page.add("Talk to an innkeeper to change your home.", ui::color::GRAY);
        return;
    }

    default:
        text = item_kind_name(item);
        text += " ";
        text += bn::to_string<6>(def.min_damage);
        text += "-";
        text += bn::to_string<6>(def.max_damage);
        text += " ";
        text += bn::to_string<4>(def.speed / 10);
        text += ".";
        text += bn::to_string<4>(def.speed % 10);
        text += "s";
        break;
    }

    bool usable = def.slot == equip_slot::NONE || can_equip(data.player_class, def);
    page.add_copy(text, usable ? ui::color::WHITE : ui::color::RED);

    bn::string<64> stats;
    add_stat(stats, def.strength, "Str");
    add_stat(stats, def.agility, "Agi");
    add_stat(stats, def.stamina, "Sta");
    add_stat(stats, def.intellect, "Int");
    add_stat(stats, def.spirit, "Spi");

    if(! stats.empty())
    {
        page.add_copy(stats, ui::color::GREEN);
    }

    // Which of the class's subclasses want it, green when the character's own does.
    if(def.slot != equip_slot::NONE && usable)
    {
        int suited = suited_subclasses(def);
        bn::string<64> suits = "Suits ";
        int count = 0;

        for(int index = 0; index < subclasses_per_class; ++index)
        {
            subclass_id subclass = class_subclass(data.player_class, index);

            if(suited & (1 << int(subclass)))
            {
                suits += count ? ", " : "";
                suits += subclass_name(subclass);
                ++count;
            }
        }

        if(count == subclasses_per_class)
        {
            suits = "Suits every ";
            suits += class_name(data.player_class);
        }

        if(count)
        {
            page.add_copy(suits, suited & (1 << int(data.subclass)) ? ui::color::GREEN : ui::color::GRAY);
        }
    }

    if(def.level > 1)
    {
        bn::string<32> level = "Requires level ";
        level += bn::to_string<4>(def.level);
        page.add_copy(level, data.level >= def.level ? ui::color::GRAY : ui::color::RED);
    }

    if(def.slot != equip_slot::NONE)
    {
        item_id equipped = data.equipment[int(def.slot)];

        if(equipped != item_id::NONE && equipped != item)
        {
            bn::string<64> now = "Now: ";
            now += get_item(equipped).name;
            page.add_copy(now, ui::color::GRAY);
        }
    }
}

}
