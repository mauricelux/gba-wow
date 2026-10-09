#include "gw_enemy_data.h"

namespace gw
{

namespace
{
    using l = look_id;
    using f = enemy_family;
    using a = enemy_ability_id;
    using namespace enemy_flag;

    constexpr ai_style MELEE = ai_style::MELEE;
    constexpr ai_style CASTER = ai_style::CASTER;
    constexpr ai_style RUNNER = ai_style::RUNNER;
    constexpr ai_style HEALER = ai_style::HEALER;
    constexpr uint8_t DUNGEON_BOSS = ELITE | BOSS | NO_RESPAWN;

    // name, look, min level, max level, flags, health %, damage %, attack speed, scale %, respawn s, loot,
    // family, AI style, abilities
    constexpr enemy_def enemies[] = {
        { "", l::YOUNG_WOLF, 1, 1, PASSIVE, 100, 100, 20, 100, 60, 0, f::BEAST, MELEE, {} },
        { "Young Wolf", l::YOUNG_WOLF, 1, 2, FAST, 90, 90, 20, 85, 45, 1, f::BEAST, MELEE, {} },
        { "Kobold Vermin", l::KOBOLD_VERMIN, 2, 3, 0, 100, 100, 20, 100, 45, 2, f::KOBOLD, RUNNER, {} },
        { "Defias Thug", l::DEFIAS_THUG, 3, 4, 0, 105, 100, 20, 100, 60, 3, f::DEFIAS, MELEE, { a::REND } },
        { "Timber Wolf", l::TIMBER_WOLF, 5, 6, FAST, 100, 100, 20, 100, 60, 1, f::BEAST, MELEE, {} },
        { "Forest Spider", l::FOREST_SPIDER, 5, 7, 0, 95, 105, 18, 90, 60, 4, f::BEAST, MELEE, { a::POISON } },
        { "Kobold Tunneler", l::KOBOLD_TUNNELER, 6, 8, 0, 100, 100, 20, 100, 60, 5,
          f::KOBOLD, RUNNER, { a::CANDLE_THROW } },
        { "Murloc Forager", l::MURLOC, 7, 9, 0, 100, 100, 20, 100, 60, 6, f::MURLOC, RUNNER, { a::CALL_FOR_HELP } },
        { "Rockhide Boar", l::BOAR, 7, 8, PASSIVE, 110, 100, 20, 100, 60, 7, f::BEAST, MELEE, { a::CHARGE } },
        { "Princess", l::PRINCESS, 9, 9, ELITE, 260, 140, 20, 140, 240, 8, f::BEAST, MELEE, { a::CHARGE, a::ENRAGE } },
        { "Riverpaw Gnoll", l::RIVERPAW_GNOLL, 8, 10, 0, 105, 100, 22, 100, 60, 9, f::GNOLL, RUNNER, { a::THRASH } },
        { "Hogger", l::HOGGER, 11, 11, ELITE, 300, 160, 22, 145, 240, 10,
          f::GNOLL, MELEE, { a::THRASH, a::KNOCKDOWN } },
        { "Harvest Watcher", l::HARVEST_WATCHER, 12, 14, 0, 115, 110, 26, 110, 60, 11, f::CONSTRUCT, MELEE, {} },
        { "Defias Trapper", l::DEFIAS_TRAPPER, 11, 13, 0, 100, 100, 20, 100, 60, 12, f::DEFIAS, MELEE, { a::NET } },
        { "Defias Smuggler", l::DEFIAS_SMUGGLER, 12, 14, 0, 100, 105, 20, 100, 60, 12, f::DEFIAS, MELEE, {} },
        { "Riverpaw Brute", l::GNOLL_BRUTE, 13, 15, 0, 115, 110, 24, 110, 60, 9, f::GNOLL, MELEE, { a::ENRAGE } },
        { "Defias Miner", l::DEFIAS_MINER, 16, 17, 0, 130, 105, 20, 100, 90, 13, f::DEFIAS, MELEE, {} },
        { "Goblin Engineer", l::GOBLIN_ENGINEER, 17, 18, 0, 125, 110, 20, 100, 90, 14, f::DEFIAS, MELEE, {} },
        { "Sneed", l::SNEED, 19, 19, ELITE | BOSS | NO_RESPAWN, 240, 130, 20, 160, 0, 15, f::DEFIAS, MELEE, {} },
        { "Defias Pirate", l::DEFIAS_PIRATE, 18, 19, 0, 130, 110, 20, 100, 90, 13, f::DEFIAS, MELEE, { a::CLEAVE } },
        { "Edwin VanCleef", l::VANCLEEF, 20, 20, ELITE | BOSS | NO_RESPAWN, 300, 110, 18, 140, 0, 16,
          f::DEFIAS, MELEE, {} },
        { "Blackguard", l::DEFIAS_BLACKGUARD, 19, 19, NO_RESPAWN, 70, 75, 20, 100, 0, 0, f::DEFIAS, MELEE, {} },
        { "Defias Prisoner", l::DEFIAS_PRISONER, 19, 20, 0, 120, 100, 20, 100, 90, 17, f::DEFIAS, MELEE, {} },
        { "Defias Convict", l::DEFIAS_CONVICT, 19, 20, 0, 130, 105, 20, 100, 90, 17, f::DEFIAS, MELEE, {} },
        { "Defias Insurgent", l::DEFIAS_INSURGENT, 20, 21, 0, 135, 110, 20, 100, 90, 17, f::DEFIAS, MELEE, {} },
        { "Targorr", l::TARGORR, 20, 20, ELITE | NO_RESPAWN, 260, 135, 22, 135, 0, 18,
          f::DEFIAS, MELEE, { a::CHARGE, a::MORTAL_STRIKE } },
        { "Kam Deepfury", l::KAM_DEEPFURY, 21, 21, ELITE | NO_RESPAWN, 280, 125, 22, 115, 0, 19,
          f::DEFIAS, MELEE, { a::KNOCKDOWN, a::SHIELD_WALL } },
        { "Bazil Thredd", l::BAZIL_THREDD, 21, 21, ELITE | BOSS | NO_RESPAWN, 250, 95, 18, 140, 0, 20,
          f::DEFIAS, MELEE, {} },
        { "Defias Rioter", l::DEFIAS_CONVICT, 19, 19, NO_RESPAWN, 60, 70, 20, 100, 0, 0, f::DEFIAS, MELEE, {} },
        // Redridge
        { "Redridge Mongrel", l::REDRIDGE_MONGREL, 15, 16, 0, 105, 100, 22, 100, 60, 25, f::GNOLL, RUNNER,
          { a::THRASH } },
        { "Shadowhide Gnoll", l::SHADOWHIDE_GNOLL, 16, 17, 0, 115, 105, 22, 105, 60, 25, f::GNOLL, MELEE,
          { a::SUNDER_ARMOR } },
        { "Shadowhide Mystic", l::SHADOWHIDE_MYSTIC, 16, 17, 0, 90, 90, 22, 100, 60, 25, f::GNOLL, CASTER,
          { a::SHADOW_BOLT, a::CURSE_OF_WEAKNESS } },
        { "Ribchaser", l::RIBCHASER, 18, 18, RARE, 190, 125, 22, 120, 255, 23, f::GNOLL, MELEE,
          { a::THRASH, a::ENRAGE } },
        { "Murloc Flesheater", l::MURLOC_FLESHEATER, 15, 16, 0, 100, 100, 20, 100, 60, 6, f::MURLOC, RUNNER,
          { a::CALL_FOR_HELP } },
        { "Great Goretusk", l::GREAT_GORETUSK, 16, 17, 0, 115, 100, 20, 110, 60, 7, f::BEAST, MELEE, { a::CHARGE } },
        { "Bellygrub", l::BELLYGRUB, 19, 19, 0, 230, 120, 24, 150, 180, 22, f::BEAST, MELEE,
          { a::CHARGE, a::ENRAGE } },
        { "Tarantula", l::TARANTULA, 15, 17, 0, 95, 105, 18, 95, 60, 4, f::BEAST, MELEE, { a::POISON } },
        { "Blackrock Outrunner", l::BLACKROCK_OUTRUNNER, 15, 17, FAST, 100, 100, 20, 100, 60, 21, f::ORC, MELEE,
          { a::CHARGE } },
        { "Blackrock Grunt", l::BLACKROCK_GRUNT, 17, 18, 0, 120, 105, 22, 105, 60, 21, f::ORC, MELEE,
          { a::SUNDER_ARMOR, a::BATTLE_SHOUT } },
        { "Blackrock Shadowcaster", l::BLACKROCK_SHADOWCASTER, 17, 18, 0, 95, 95, 22, 100, 60, 21, f::ORC, CASTER,
          { a::SHADOW_BOLT, a::CURSE_OF_WEAKNESS } },
        { "Blackrock Renegade", l::BLACKROCK_RENEGADE, 18, 19, 0, 125, 110, 22, 105, 75, 21, f::ORC, MELEE,
          { a::MORTAL_STRIKE } },
        { "Blackrock Summoner", l::BLACKROCK_SUMMONER, 18, 19, 0, 100, 100, 22, 100, 75, 21, f::ORC, CASTER,
          { a::FIREBALL, a::BURNING } },
        { "Gath'Ilzogg", l::GATH_ILZOGG, 20, 20, ELITE, 320, 150, 22, 140, 240, 24, f::ORC, MELEE,
          { a::CLEAVE, a::WAR_STOMP } },
        // Duskwood
        { "Dire Wolf", l::DIRE_WOLF, 20, 21, FAST, 105, 100, 20, 105, 60, 26, f::BEAST, MELEE, {} },
        { "Rabid Dire Wolf", l::RABID_DIRE_WOLF, 22, 23, FAST, 110, 105, 20, 110, 60, 26, f::BEAST, MELEE,
          { a::DISEASE } },
        { "Venom Web Spider", l::VENOM_WEB_SPIDER, 21, 22, 0, 100, 105, 18, 100, 60, 27, f::BEAST, MELEE,
          { a::POISON, a::WEB } },
        { "Nightbane Dark Runner", l::NIGHTBANE_DARK_RUNNER, 21, 22, FAST, 115, 105, 20, 105, 60, 28, f::WORGEN, MELEE,
          { a::LEAP, a::ENRAGE } },
        { "Nightbane Shadow Weaver", l::NIGHTBANE_SHADOW_WEAVER, 22, 23, 0, 95, 95, 22, 105, 60, 28, f::WORGEN,
          CASTER, { a::SHADOW_BOLT, a::CURSE_OF_WEAKNESS } },
        { "Nightbane Tainted One", l::NIGHTBANE_TAINTED_ONE, 23, 24, 0, 125, 110, 22, 115, 60, 28, f::WORGEN, MELEE,
          { a::LEAP, a::REND } },
        { "Skeletal Warrior", l::SKELETAL_WARRIOR, 21, 22, 0, 115, 100, 22, 100, 60, 29, f::UNDEAD, MELEE,
          { a::SUNDER_ARMOR } },
        { "Skeletal Mage", l::SKELETAL_MAGE, 21, 22, 0, 90, 95, 22, 100, 60, 29, f::UNDEAD, CASTER,
          { a::FROSTBOLT, a::FROST_NOVA } },
        { "Skeletal Servant", l::SKELETAL_SERVANT, 22, 22, NO_RESPAWN, 70, 80, 22, 90, 0, 0, f::UNDEAD, MELEE, {} },
        { "Rotting Ghoul", l::ROTTING_GHOUL, 22, 23, 0, 120, 105, 22, 105, 60, 30, f::UNDEAD, MELEE, { a::DISEASE } },
        { "Plague Spreader", l::PLAGUE_SPREADER, 24, 25, 0, 125, 110, 22, 110, 60, 30, f::UNDEAD, MELEE,
          { a::DISEASE, a::THRASH } },
        { "Splinter Fist Ogre", l::SPLINTER_FIST_OGRE, 23, 24, 0, 140, 115, 26, 130, 75, 31, f::OGRE, MELEE,
          { a::CLEAVE } },
        { "Splinter Fist Taskmaster", l::SPLINTER_FIST_TASKMASTER, 24, 25, 0, 150, 120, 26, 135, 75, 31, f::OGRE,
          MELEE, { a::WAR_STOMP, a::BATTLE_SHOUT } },
        { "Mor'Ladim", l::MOR_LADIM, 25, 25, ELITE, 320, 150, 22, 130, 240, 32, f::UNDEAD, MELEE,
          { a::MORTAL_STRIKE, a::KNOCKDOWN } },
        { "Stalvan Mistmantle", l::STALVAN_MISTMANTLE, 24, 24, ELITE, 280, 135, 20, 120, 240, 33, f::UNDEAD, MELEE,
          { a::SHADOW_BOLT, a::CURSE_OF_WEAKNESS } },
        { "Morbent Fel", l::MORBENT_FEL, 25, 25, ELITE, 300, 135, 20, 130, 240, 34, f::UNDEAD, CASTER,
          { a::SHADOW_BOLT, a::SUMMON_SKELETON } },
        { "Stitches", l::STITCHES, 26, 26, ELITE | PATROL | QUEST, 450, 140, 26, 160, 255, 35, f::UNDEAD, MELEE,
          { a::CLEAVE, a::DISEASE } },
        // Silverpine Forest and Shadowfang Keep
        { "Bleak Worg", l::BLEAK_WORG, 21, 22, FAST, 110, 105, 20, 105, 60, 26, f::BEAST, MELEE, { a::REND } },
        { "Shadowfang Moonwalker", l::SHADOWFANG_MOONWALKER, 22, 23, 0, 130, 105, 22, 110, 90, 36, f::WORGEN,
          MELEE, { a::LEAP, a::THRASH } },
        { "Shadowfang Darkcaster", l::SHADOWFANG_DARKCASTER, 22, 23, 0, 100, 95, 22, 110, 90, 36, f::WORGEN, CASTER,
          { a::SHADOW_BOLT, a::FEAR } },
        { "Shadowfang Wolfguard", l::SHADOWFANG_WOLFGUARD, 23, 24, 0, 140, 110, 22, 115, 90, 36, f::WORGEN, MELEE,
          { a::SUNDER_ARMOR, a::ENRAGE } },
        { "Haunted Servitor", l::HAUNTED_SERVITOR, 23, 24, 0, 115, 105, 22, 105, 90, 37, f::UNDEAD, MELEE,
          { a::CURSE_OF_WEAKNESS } },
        { "Wailing Guardsman", l::WAILING_GUARDSMAN, 24, 25, 0, 130, 110, 22, 110, 90, 37, f::UNDEAD, MELEE,
          { a::SHIELD_BLOCK, a::FEAR } },
        { "Rethilgore", l::RETHILGORE, 23, 23, ELITE | BOSS | NO_RESPAWN, 280, 130, 22, 140, 0, 38, f::WORGEN, MELEE,
          { a::KNOCKDOWN, a::REND } },
        { "Razorclaw the Butcher", l::RAZORCLAW_THE_BUTCHER, 24, 24, ELITE | BOSS | NO_RESPAWN, 300, 135, 24, 140, 0,
          39, f::WORGEN, MELEE, { a::CLEAVE, a::REND } },
        { "Baron Silverlaine", l::BARON_SILVERLAINE, 24, 24, ELITE | BOSS | NO_RESPAWN, 300, 130, 20, 135, 0, 40,
          f::UNDEAD, MELEE, { a::MORTAL_STRIKE, a::CURSE_OF_WEAKNESS } },
        { "Commander Springvale", l::COMMANDER_SPRINGVALE, 25, 25, ELITE | BOSS | NO_RESPAWN, 320, 130, 20, 135, 0, 41,
          f::UNDEAD, MELEE, { a::HEAL, a::KNOCKDOWN } },
        { "Odo the Blindwatcher", l::ODO_THE_BLINDWATCHER, 25, 25, ELITE | BOSS | NO_RESPAWN, 330, 140, 22, 150, 0,
          42, f::WORGEN, MELEE, { a::FEAR, a::ENRAGE } },
        { "Archmage Arugal", l::ARCHMAGE_ARUGAL, 26, 26, ELITE | BOSS | NO_RESPAWN, 320, 120, 20, 135, 0, 43,
          f::WORGEN, CASTER, { a::SHADOW_BOLT, a::BLINK } },
        { "Lupine Horror", l::BLEAK_WORG, 24, 24, NO_RESPAWN, 90, 90, 20, 120, 0, 0, f::WORGEN, MELEE, { a::LEAP } },
        // The Wetlands
        { "Young Crocolisk", l::YOUNG_CROCOLISK, 24, 25, 0, 110, 100, 22, 95, 60, 44, f::BEAST, MELEE, {} },
        { "Giant Crocolisk", l::GIANT_CROCOLISK, 26, 27, 0, 135, 110, 24, 120, 60, 44, f::BEAST, MELEE,
          { a::THRASH, a::REND } },
        { "Mottled Raptor", l::MOTTLED_RAPTOR, 25, 26, FAST, 105, 105, 20, 100, 60, 45, f::BEAST, MELEE,
          { a::REND } },
        { "Mottled Screecher", l::MOTTLED_SCREECHER, 26, 27, FAST, 100, 105, 20, 100, 60, 45, f::BEAST, MELEE,
          { a::CALL_FOR_HELP } },
        { "Sarltooth", l::SARLTOOTH, 28, 28, RARE, 220, 130, 20, 125, 255, 50, f::BEAST, MELEE,
          { a::REND, a::ENRAGE } },
        { "Mosshide Gnoll", l::MOSSHIDE_GNOLL, 25, 26, 0, 115, 105, 22, 100, 60, 46, f::GNOLL, RUNNER, { a::REND } },
        { "Mosshide Mystic", l::MOSSHIDE_MYSTIC, 25, 26, 0, 95, 95, 22, 100, 60, 46, f::GNOLL, HEALER,
          { a::HEALING_WAVE, a::LIGHTNING_BOLT } },
        { "Bluegill Murloc", l::BLUEGILL_MURLOC, 24, 25, 0, 105, 100, 20, 100, 60, 47, f::MURLOC, RUNNER,
          { a::CALL_FOR_HELP } },
        { "Dark Iron Dwarf", l::DARK_IRON_DWARF, 26, 27, 0, 130, 105, 22, 95, 75, 48, f::DWARF, MELEE,
          { a::SUNDER_ARMOR, a::SHIELD_BLOCK } },
        { "Dark Iron Saboteur", l::DARK_IRON_SABOTEUR, 26, 27, 0, 110, 105, 22, 95, 75, 48, f::DWARF, MELEE,
          { a::THROW_DYNAMITE } },
        { "Balgaras the Foul", l::BALGARAS_THE_FOUL, 28, 28, ELITE, 320, 145, 22, 115, 240, 51, f::DWARF, MELEE,
          { a::CLEAVE, a::KNOCKDOWN } },
        { "Dragonmaw Grunt", l::DRAGONMAW_GRUNT, 27, 28, 0, 130, 110, 22, 105, 75, 49, f::ORC, MELEE,
          { a::CHARGE, a::SUNDER_ARMOR } },
        { "Dragonmaw Shadowwarder", l::DRAGONMAW_SHADOWWARDER, 27, 28, 0, 100, 100, 22, 100, 75, 49, f::ORC, CASTER,
          { a::SHADOW_BOLT, a::CURSE_OF_WEAKNESS } },
        { "Nek'rosh", l::NEK_ROSH, 30, 30, ELITE, 340, 150, 22, 135, 240, 52, f::ORC, MELEE,
          { a::MORTAL_STRIKE, a::BATTLE_SHOUT } },
        // Blackfathom Deeps
        { "Blackfathom Myrmidon", l::BLACKFATHOM_MYRMIDON, 24, 25, 0, 135, 110, 22, 110, 90, 53, f::NAGA, MELEE,
          { a::CLEAVE, a::SUNDER_ARMOR } },
        { "Blackfathom Tide Priestess", l::BLACKFATHOM_TIDE_PRIESTESS, 24, 25, 0, 105, 100, 22, 105, 90, 53, f::NAGA,
          HEALER, { a::HEALING_WAVE, a::FROSTBOLT } },
        { "Aku'mai Snapjaw", l::AKU_MAI_SNAPJAW, 24, 25, 0, 150, 100, 26, 115, 90, 0, f::BEAST, MELEE,
          { a::STONESKIN, a::THRASH } },
        { "Blindlight Murloc", l::BLINDLIGHT_MURLOC, 25, 26, 0, 110, 105, 20, 100, 90, 47, f::MURLOC, RUNNER,
          { a::NET } },
        { "Twilight Acolyte", l::TWILIGHT_ACOLYTE, 26, 27, 0, 110, 100, 22, 100, 90, 54, f::CULTIST, HEALER,
          { a::HEAL, a::SHADOW_BOLT } },
        { "Twilight Reaver", l::TWILIGHT_REAVER, 26, 27, 0, 140, 110, 22, 105, 90, 54, f::CULTIST, MELEE,
          { a::CLEAVE, a::BATTLE_SHOUT } },
        { "Aku'mai Servant", l::AKU_MAI_SERVANT, 27, 27, 0, 140, 110, 22, 110, 90, 0, f::BEAST, MELEE,
          { a::POISON } },
        { "Ghamoo-ra", l::GHAMOO_RA, 25, 25, DUNGEON_BOSS, 320, 120, 26, 160, 0, 55, f::BEAST, MELEE,
          { a::STONESKIN, a::KNOCKDOWN } },
        { "Lady Sarevess", l::LADY_SAREVESS, 26, 26, DUNGEON_BOSS, 280, 125, 22, 135, 0, 56, f::NAGA, CASTER,
          { a::FORKED_LIGHTNING, a::FROST_NOVA } },
        { "Gelihast", l::GELIHAST, 26, 26, DUNGEON_BOSS, 300, 130, 20, 145, 0, 57, f::MURLOC, MELEE,
          { a::NET, a::THRASH } },
        { "Twilight Lord Kelris", l::TWILIGHT_LORD_KELRIS, 27, 27, DUNGEON_BOSS, 300, 125, 22, 135, 0, 58,
          f::CULTIST, CASTER, { a::SHADOW_BOLT, a::SLEEP } },
        { "Aku'mai", l::AKU_MAI, 28, 28, DUNGEON_BOSS, 360, 140, 24, 170, 0, 59, f::BEAST, MELEE,
          { a::POISON, a::ENRAGE } },
        // Gnomeregan and Dun Morogh
        { "Leper Gnome", l::LEPER_GNOME, 26, 27, 0, 105, 100, 20, 90, 75, 61, f::GNOME, MELEE, { a::DISEASE } },
        { "Irradiated Pillager", l::IRRADIATED_PILLAGER, 27, 28, 0, 130, 110, 22, 105, 90, 60, f::TROGG, MELEE,
          { a::POISON, a::KNOCKDOWN } },
        { "Caverndeep Burrower", l::CAVERNDEEP_BURROWER, 27, 28, 0, 125, 105, 22, 105, 90, 60, f::TROGG, MELEE,
          { a::THRASH } },
        { "Irradiated Slime", l::IRRADIATED_SLIME, 27, 28, 0, 120, 100, 24, 100, 90, 63, f::OOZE, MELEE,
          { a::POISON } },
        { "Dark Iron Agent", l::DARK_IRON_AGENT, 28, 29, 0, 125, 110, 22, 95, 90, 48, f::DWARF, MELEE,
          { a::THROW_DYNAMITE, a::SHIELD_BLOCK } },
        { "Mechano-Tank", l::MECHANO_TANK, 28, 29, 0, 150, 110, 26, 115, 90, 62, f::CONSTRUCT, MELEE,
          { a::KNOCKDOWN, a::SHIELD_BLOCK } },
        { "Arcane Nullifier", l::ARCANE_NULLIFIER, 28, 29, 0, 120, 100, 24, 110, 90, 62, f::CONSTRUCT, CASTER,
          { a::MANA_SHIELD, a::FROST_NOVA } },
        { "Walking Bomb", l::WALKING_BOMB, 28, 28, NO_RESPAWN, 30, 120, 20, 90, 0, 0, f::CONSTRUCT, MELEE,
          { a::SELF_DESTRUCT } },
        { "Grubbis", l::GRUBBIS, 28, 28, DUNGEON_BOSS, 320, 130, 24, 150, 0, 64, f::TROGG, MELEE,
          { a::CLEAVE, a::WAR_STOMP } },
        { "Viscous Fallout", l::VISCOUS_FALLOUT, 28, 28, DUNGEON_BOSS, 340, 120, 26, 160, 0, 65, f::OOZE, MELEE,
          { a::TOXIC_VOLLEY, a::DISEASE } },
        { "Electrocutioner 6000", l::ELECTROCUTIONER_6000, 29, 29, DUNGEON_BOSS, 320, 130, 22, 150, 0, 66,
          f::CONSTRUCT, MELEE, { a::LIGHTNING_BOLT, a::KNOCKDOWN } },
        { "Crowd Pummeler 9-60", l::CROWD_PUMMELER, 29, 29, DUNGEON_BOSS, 350, 135, 24, 160, 0, 67, f::CONSTRUCT,
          MELEE, { a::CROWD_PUMMEL, a::THRASH } },
        { "Mekgineer Thermaplugg", l::MEKGINEER_THERMAPLUGG, 30, 30, DUNGEON_BOSS, 380, 135, 22, 140, 0, 68,
          f::GNOME, MELEE, { a::KNOCKDOWN } },
    };

    static_assert(sizeof(enemies) / sizeof(enemies[0]) == int(enemy_id::COUNT));
}

const enemy_def& get_enemy_def(enemy_id enemy)
{
    return enemies[int(enemy)];
}

int enemy_base_health(int level)
{
    return (30 + 10 * level + level * level) * 65 / 100;
}

int enemy_base_damage(int level)
{
    return 3 + level * 3 / 2;
}

}
