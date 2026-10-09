# GBA WoW: the road to level 60

This plan takes the game from its current cap of level 20 to level 60. It builds on what is already in
the repo (milestones M0 to M11, with the Stockade in PR #4) and on the first roadmap. Nothing in the
game code has changed yet.

**What Maurice asked for**

- Levels 1 to 60, with one zone for every 5 levels.
- 20 dungeons.
- Subclasses for the 3 existing classes, chosen at character creation. A Fire Mage only gets fire
  spells, a Frost Mage only frost spells, and so on.
- Buffs you rarely cast go on harder button combinations.
- Some enemies get abilities.
- Bags with unlimited space, sorted by type.
- Ability ranks.
- No professions for now.
- Everything inspired by WoW's quests, zones, talents and abilities.

One note on the numbers: "one dungeon for each 5 levels" gives 12, not 20. This plan keeps the 20
dungeons and spreads them over the 12 zones. The first three zones have no dungeon (the Echo Ridge
and Fargodeep caves stay there as starter caves), and the later zones have two or three each, the way
WoW's own dungeon list gets denser near the top.

---

## 1. Where the game is today

| What | Today | In the code |
| --- | --- | --- |
| Level cap | 20 | `max_level` in `include/gw_character.h`, `xp_table` in `src/gw_character.cpp` |
| Zones | Northshire, Elwynn, Stormwind, Westfall, plus the two mines | `include/gw_map_*.h`, `tools/gen_world.py` |
| Dungeons | Deadmines, Stockade (PR #4) | `gw_map_deadmines.h`, `gw_map_stockade.h` |
| Classes | Warrior, Mage, Hunter, each with three talent trees of 8 talents | `gw_abilities.cpp`, `gw_talents.cpp` |
| Races | Human (Warrior, Mage), Dwarf and Night Elf (Warrior, Hunter) | `class_allowed()` |
| Abilities | 30 in total, 10 per class; damage grows a little with every level (`value_per_level`) | `ability_def` |
| Action bar | 7 slots: hold R, then A, B, L or a D-pad direction | `action_slots = 7` |
| Bags | 16 slots, stacks of up to `item_def::stack` | `bag_slots = 16` |
| Enemies | 29 types; only Princess, Hogger, Sneed, VanCleef, Targorr, Kam and Bazil have special moves, hard-coded in `src/gw_combat.cpp` | `enemy_def`, `gw_combat.cpp` around line 1700 |
| Quests | 29 | `gw_quests.cpp` |
| Save | One SRAM slot, version 3, about 300 bytes | `gw_save.cpp` |
| ROM | 1.1 MB of the GBA's 32 MB | |

Several limits have to grow before any new content goes in:

- `item_id`, `enemy_id`, `npc_id` and `map_id` are all `uint8_t`, which allows at most 255 of each.
  Levels 1 to 60 need about 900 items, 250 enemy types and 300 NPCs, so these become `uint16_t`.
- `known_abilities` is a 32-bit mask, which allows 32 abilities. Ranks and subclasses need about 150.
- `max_talents = 24` and `max_quests = 32` are too small.
- The save layout changes, so the save version goes to 4. Old saves get migrated, see section 9.

---

## 2. The world: 12 zones

The route follows the Alliance leveling path from WoW Classic. The first half stays in the Eastern
Kingdoms. At 40 a boat goes from Booty Bay to Kalimdor, and at 50 the hero sails home for the
endgame. Each zone is one streamed outdoor map, about the size of Westfall (1024×1024) or Elwynn
(2048×2048), with a quest hub, an inn, a flight master and a few small interiors or caves.

| # | Levels | Zone | Hub(s) | Main story | Dungeons |
| --- | --- | --- | --- | --- | --- |
| 1 | 1–5 | Northshire Valley ✓ | Northshire Abbey | Kobolds and the Blackrock worgs | Echo Ridge Mine (cave) ✓ |
| 2 | 5–10 | Elwynn Forest ✓ | Goldshire, **Stormwind** ✓ | Defias in the farms, Hogger, Princess | Fargodeep Mine (cave) ✓ |
| 3 | 10–15 | Westfall ✓ | Sentinel Hill | The People's Militia vs the Defias | (Deadmines entrance is here) |
| 4 | 15–20 | Redridge Mountains ✓ | Lakeshire ✓ | Blackrock orcs and the Redridge gnolls | **Deadmines** ✓, **Stockade** ✓ |
| 5 | 20–25 | Duskwood ✓ | Darkshire ✓ | Worgen, Stitches, Morbent Fel, the Night Watch | **Shadowfang Keep** ✓ |
| 6 | 25–30 | Wetlands ✓ | Menethil Harbor ✓, **Ironforge** ✓ | Dark Iron dwarves, Dragonmaw orcs, Grim Batol | **Blackfathom Deeps** ✓, **Gnomeregan** ✓ |
| 7 | 30–35 | Hillsbrad Foothills ✓ | Southshore ✓ | Syndicate, Forsaken, Alterac ogres | **SM Graveyard** ✓, **SM Library** ✓ |
| 8 | 35–40 | Stranglethorn Vale | Rebel Camp, Booty Bay | The tiger and raptor hunts, Bloodsail pirates, trolls | **SM Armory**, **SM Cathedral** |
| 9 | 40–45 | Tanaris and Thousand Needles | Gadgetzan | Wastewander bandits, Sandfury trolls, the quilboar | **Razorfen Kraul**, **Razorfen Downs**, **Zul'Farrak** |
| 10 | 45–50 | Feralas | Feathermoon Stronghold | Gordok ogres, the Grimtotem, the Emerald Dream portal | **Maraudon**, **Dire Maul** |
| 11 | 50–55 | Burning Steppes, with Searing Gorge and the Badlands edge | Morgan's Vigil, Thorium Point | The Dark Iron empire and the Blackrock clan | **Uldaman**, **Sunken Temple**, **Blackrock Depths** |
| 12 | 55–60 | Western and Eastern Plaguelands | Chillwind Camp, Light's Hope Chapel | The Scourge, the Scarlet Crusade, the Argent Dawn | **Blackrock Spire**, **Scholomance**, **Stratholme** |

✓ means it already exists. Redridge pulls Westfall's last levels forward, so Westfall gets retuned
to 10–15 and the Deadmines to 16–19 (section 8).

### The zones in a few lines each

Each zone gets about 12 to 15 quests, one or two elites, a rare spawn and two or three hidden
chests, which means about 150 new quests across all zones.

**4. Redridge Mountains (15–20).** Lakeshire on the lake, its bridge broken by the orcs. Quests:
Blackrock Menace, Shadowhide gnolls, Murloc Poachers, *Solomon's Law* for Magistrate Solomon,
Bellygrub the pig, and the Redridge fishing contest at the lake. Elite: **Gath'Ilzogg** at
Stonewatch Keep. Rare: **Ribchaser**. The dungeon bridge: VanCleef's
letter in the existing story already leads to the Stockade, so the Stockade moves from "game ending"
to "end of the 15–20 chapter".

**5. Duskwood (20–25).** Always night: a dark palette and a light circle around the hero. Darkshire,
Raven Hill cemetery, the Twilight Grove. Quests: *The Legend of Stalvan*, *Wolves at Our Heels*,
*The Night Watch*, *Morbent Fel*, *Mor'Ladim*. Elite: **Stitches**, who walks the road to Darkshire
and has to be stopped at the town gate. The worgen story points to Arugal, so the hero flies to
Silverpine Forest for Shadowfang Keep.

**6. Wetlands (25–30), with Ironforge.** Ironforge becomes the second capital, reached from
Stormwind through the Deeprun Tram (a short train ride). Menethil Harbor is the Wetlands hub and the
port for the boats. Quests: Dark Iron dwarves at Dun Modr, Dragonmaw orcs, the Thandol Span, the
young crocolisks, *Digging Up the Past* at the excavation. A boat to Auberdine leads to Blackfathom
Deeps, and Mekkatorque in Ironforge sends the hero to Gnomeregan.

**7. Hillsbrad Foothills (30–35).** Southshore, the Alterac foothills and Durnholde Keep. Quests:
Syndicate thieves, Alterac ogres, the Forsaken plague farms, *Bloodfang* the yeti. Elite: **Gravis
Slipknot** at the Syndicate camp. Southshore's paladin sends the hero to Tirisfal for the Scarlet
Monastery.

**8. Stranglethorn Vale (35–40).** Jungle, two hubs. Hemet Nesingwary's *Big Game Hunter*
chain (panthers, raptors, tigers, then King Bangalash), the Bloodsail pirates at Booty Bay, and the
pages of the *Green Hills of Stranglethorn* as a collection quest. Rare: **Mogh the Undying**. The
Scarlet Monastery's last two wings are this bracket's dungeons.

**9. Tanaris and Thousand Needles (40–45).** The first Kalimdor zone, reached by boat from Booty
Bay. Gadgetzan's goblins, the Wastewander bandits, the Steamwheedle pirates, the Shimmering Flats
race track. The Thousand Needles canyons lead to Razorfen Kraul and Downs. Zul'Farrak is the
chapter's dungeon finale.

**10. Feralas (45–50).** A rainforest of huge trees. Feathermoon Stronghold on its island, the
Twin Colossals, Gordok ogres, the wildkin, and the Emerald Dream portal. Desolace's Maraudon and
Feralas's own Dire Maul are its two dungeons.

**11. Burning Steppes (50–55).** Back in the Eastern Kingdoms, a red sky and lava. Morgan's Vigil,
Thorium Point in the Searing Gorge, and the Badlands dig sites. Quests: the Blackrock orcs, *Ragnaros*
foreshadowing, the Dark Iron dwarves, Marshal Windsor's story. Its dungeons are Uldaman in the
Badlands, the Sunken Temple in the Swamp of Sorrows (a flight away) and Blackrock Depths.

**12. Western and Eastern Plaguelands (55–60).** Grey grass, dead trees, the Scourge. Chillwind
Camp, Andorhal and the farms in the west, Light's Hope Chapel, Tyr's Hand and Stratholme in the
east. The Argent Dawn reputation is replaced by a simple "Argent Dawn tokens" vendor so no new
system is needed.

### Getting around

Twelve zones need faster travel. All of these are new and small:

- **Flight masters** in every hub ✓. Talking to one opens a list of discovered flight paths. The hero
  rides a gryphon across the world map screen, then fades into the destination.
- **Boats and the Deeprun Tram** ✓ as door warps with a short travel scene.
- **A mount at level 30** ✓ (WoW's level 40, moved down because GBA zones are smaller): a horse, ram
  or nightsaber depending on race. Bought in Stormwind, Ironforge or from a nightsaber trainer, it
  lets you move 60% faster outside combat. Mounting is on the buff bar (section 5).
- **The hearthstone** already exists and keeps working.
- **The world map page** ✓ already exists. It gets a second level: Eastern Kingdoms and Kalimdor,
  then the zone.

M16 built these on the current world: gryphon masters in Stormwind and at Sentinel Hill, the riding
trainer in Stormwind, the continent level of the world map (zones of later chapters are on it,
marked "Coming later"), and the Deeprun Tram from Stormwind to an Ironforge Station whose lift stays
shut until M19 opens Ironforge. The boat scene is in the engine (a warp with `ride='boat'`); the
first boats arrive with Menethil Harbor in M19 and Booty Bay in M21. Each hub's chapter adds its
flight master to `src/gw_travel.cpp` and its zone to the continent pictures in `gen_travel.py`.
M19 opened the station's stairs to Tinker Town and put the first boat on Menethil's pier (to
Auberdine, on Kalimdor's continent picture, with its own hippogryph master). M20 added Southshore's
gryphons and the Argent Watch in Tirisfal, which a quest marks on the map like the Scouts' Camp.

---

## 3. 20 dungeons

Every dungeon is its own map, like the Deadmines and the Stockade, with 3 to 5 bosses, trash packs
of 2 or 3 enemies, a hidden chest, a quest chain that starts in the zone hub, and a blue item from
each boss. Because there are no groups, dungeon enemies are tuned for one hero, and most bosses
telegraph their big attacks (red circles, cast bars) so a solo player can dodge or interrupt them.

| # | Dungeon | Levels | Entrance | Bosses (last one in bold) | Signature mechanics |
| --- | --- | --- | --- | --- | --- |
| 1 | The Deadmines ✓ | 16–19 | Westfall | Sneed ✓, **Edwin VanCleef** ✓; new: Rhahk'Zor, Gilnid, Mr. Smite | Shredder phase, VanCleef calls his guards |
| 2 | The Stockade ✓ | 19–21 | Stormwind | Targorr ✓, Kam Deepfury ✓, **Bazil Thredd** ✓; new: Hamhock | Smoke bombs, adds, frenzy |
| 3 | Shadowfang Keep | 22–26 | Silverpine (flight from Darkshire) | Rethilgore, Razorclaw the Butcher, Baron Silverlaine, Commander Springvale, Odo the Blindwatcher, **Archmage Arugal** | Arugal teleports between ledges and turns his worgen loose; Springvale heals himself and must be interrupted |
| 4 | Blackfathom Deeps ✓ | 24–28 | Ashenvale coast (boat to Auberdine) | Ghamoo-ra, Lady Sarevess, Gelihast, Twilight Lord Kelris, **Aku'mai** | Light the four braziers to open Aku'mai's door; Sarevess's Forked Lightning |
| 5 | Gnomeregan ✓ | 26–30 | Dun Morogh (from Ironforge) | Grubbis, Viscous Fallout, Electrocutioner 6000, Crowd Pummeler 9-60, **Mekgineer Thermaplugg** | Radiation pools hurt over time; Thermaplugg's bomb bots must be dodged or killed (the wall buttons are left out for now) |
| 6 | Scarlet Monastery: Graveyard ✓ | 30–33 | Tirisfal (from Southshore) | Interrogator Vishas, Azshir the Sleepless, Bloodmage Thalnos, **Ironspine** | Short wing; Thalnos casts Flame Spike circles |
| 7 | Scarlet Monastery: Library ✓ | 33–35 | Tirisfal | Houndmaster Loksey, **Arcanist Doan** | Doan's Detonation: get out of range or it hits hard; Loksey's hounds |
| 8 | Scarlet Monastery: Armory | 35–37 | Tirisfal | **Herod** | Whirlwind charge-up, then a wave of trainees when he falls |
| 9 | Scarlet Monastery: Cathedral | 37–40 | Tirisfal | High Inquisitor Fairbanks, Scarlet Commander Mograine, **High Inquisitor Whitemane** | Whitemane resurrects Mograine and the fight goes on; Deep Sleep puts the hero to sleep |
| 10 | Razorfen Kraul | 40–41 | Southern Barrens edge | Aggem Thorncurse, Death Speaker Jargba, Overlord Ramtusk, Agathelos the Raging, **Charlga Razorflank** | Thorn walls shrink the paths; Charlga heals her guards |
| 11 | Razorfen Downs | 41–43 | Thousand Needles | Tuten'kash, Mordresh Fire Eye, Glutton, **Amnennar the Coldbringer** | The gong event summons spider waves before Tuten'kash; Amnennar's Frost Nova and spectral adds |
| 12 | Zul'Farrak | 43–45 | Tanaris | Antu'sul, Theka the Martyr, Witch Doctor Zum'rah, Gahz'rilla, Nekrum and Sezz'ziz, **Chief Ukorz Sandscalp** | The pyramid event: waves of trolls climb the stairs; Gahz'rilla is summoned by ringing the gong at her pool |
| 13 | Maraudon | 46–48 | Desolace (from Feralas) | Noxxion, Razorlash, Lord Vyletongue, Celebras the Cursed, Landslide, Tinkerer Gizlock, Rotgrip, **Princess Theradras** | Poison pools; Theradras's Boulder throws and knockback |
| 14 | Dire Maul | 48–50 | Feralas | Lethtendris, Hydrospawn, Zevrim Thornhoof, Alzzin, Tendris Warpwood, Immol'thar, King Gordok, **Prince Tortheldrin** | Ogre "tribute" option: kill King Gordok and the ogres become friendly; pylons to drop Immol'thar's shield |
| 15 | Uldaman | 50–51 | Badlands | Revelosh, Ironaya, Obsidian Sentinel, Ancient Stone Keeper, Galgann Firehammer, Grimlok, **Archaedas** | Archaedas wakes his stone guardians in waves; Ironaya knocks you back |
| 16 | Sunken Temple | 51–53 | Swamp of Sorrows | Atal'alarion, Jammal'an the Prophet, Weaver and Dreamscythe, **Shade of Eranikus** | Light the six statues in order to summon Atal'alarion; dragon breath cones |
| 17 | Blackrock Depths | 53–55 | Burning Steppes | Lord Roccor, High Interrogator Gerstahn, the Ring of Law, Golem Lord Argelmach, General Angerforge, Ambassador Flamelash, Magmus, **Emperor Dagran Thaurissan** | The Ring of Law arena picks a random boss; the Grim Guzzler bar; free Marshal Windsor |
| 18 | Blackrock Spire | 55–57 | Burning Steppes | Highlord Omokk, War Master Voone, Mother Smolderweb, Overlord Wyrmthalak, Pyroguard Emberseer, The Beast, **General Drakkisath** | Lower and upper halves in one map; Emberseer's seal is broken at the seven altars |
| 19 | Scholomance | 57–59 | Western Plaguelands | Jandice Barov, Rattlegore, Ras Frostwhisper, Instructor Malicia, Lord Alexei Barov, Lady Illucia Barov, **Darkmaster Gandling** | Jandice's mirror images; Gandling teleports the hero into a sealed room full of risen students |
| 20 | Stratholme | 59–60 | Eastern Plaguelands | Timmy the Cruel, Malor the Zealous, Balnazzar, Maleki the Pallid, Ramstein the Gorger, **Baron Rivendare** | Two halves (Scarlet and Scourge); a timer to save the captive before Rivendare's countdown ends |

### The finale

The current game ends when Bazil Thredd dies and Bolvar thanks you. At 60 the ending moves, and it
reuses Bolvar: in WoW the Alliance's real enemy is **Lady Katrana Prestor**, Bolvar's advisor, who
is the black dragon Onyxia in disguise. Marshal Windsor's chain (Blackrock Depths → Blackrock Spire →
Stormwind Keep) unmasks her in front of Bolvar, and the last fight is **Onyxia** in her lair in
Dustwallow Marsh, as a one-room encounter with three phases: on the ground, in the air with fire
breath lanes and whelp waves, then back on the ground. That is the game's last boss and the new
ending. It is a single room, not counted as one of the 20 dungeons.

---

## 4. Subclasses

You pick a class, then one of its three subclasses, on the character creation screen. They match the
three talent trees each class already has, so the names stay familiar. The choice is permanent for
that character, like the race.

| Class | Subclass | Role on GBA | Resource |
| --- | --- | --- | --- |
| Warrior | **Arms** | Big, slow hits and bleeds, two-handed weapons | Rage |
| Warrior | **Fury** | Fast hits, dual wielding, self-healing | Rage |
| Warrior | **Protection** | Shield, high armor, outlasts anything | Rage |
| Mage | **Fire** | Burst and burning damage over time | Mana |
| Mage | **Frost** | Slows, freezes and shatter combos, safest mage | Mana |
| Mage | **Arcane** | Mana-hungry burst, blinks, best area damage | Mana |
| Hunter | **Beast Mastery** | Fights with a tamed pet | Mana |
| Hunter | **Marksmanship** | Long-range shots and big aimed hits | Mana |
| Hunter | **Survival** | Melee and traps, hardest to kill | Mana |

How it works:

- Each class has a **small shared kit** (4 to 6 utility abilities every subclass gets) and a
  **subclass kit** (8 to 10 abilities only that subclass gets). A Fire Mage never sees Frostbolt and a
  Frost Mage never sees Fireball. The trainer only lists your subclass's spells.
- **Warrior stances are automatic**: Arms fights in Battle Stance, Fury in Berserker Stance,
  Protection in Defensive Stance. No stance dancing, so no extra buttons.
- **Talents**: you only get your subclass's tree. It grows from 8 to 18 talents and 51 points (one per
  level from 10 to 60), with seven tiers, one every 5 points as in WoW. The last tier is a capstone
  ability, the way Mortal Strike, Bloodthirst and Shield Slam were.
- **Gear** shows which subclass it suits in its tooltip (strength or agility, intellect, shield), but
  any item your class can wear stays wearable.
- **Respec**: not planned. If wanted later, a trainer could offer a costly subclass change.

### What happens to the existing 30 abilities

All of them stay, sorted into the kits below. Today's talent-only abilities (Mortal Strike,
Bloodthirst, Last Stand, Pyroblast, Ice Barrier, Arcane Power, Aimed Shot, Counterattack, Bestial
Wrath) become the capstones or mid-tree talents of their subclass.

---

## 5. Abilities and ranks

### How ranks work

Today an ability gets a little stronger every level (`value_per_level`). That is replaced by
**ranks**, as in WoW:

- Each ability has a list of ranks. Each rank has its own required level, trainer cost, damage or
  effect, and resource cost. Higher ranks cost more mana or rage.
- You buy new ranks at your class trainer, who already exists in every town. The trainer screen marks
  abilities with a new rank available, and the HUD shows a small "!" on the trainer's head.
- The action bar always uses your highest rank. The spellbook shows "Fireball (Rank 5)".
- Rotation spells get about 10 ranks (one every 6 levels). Utility spells get 2 to 5. Big
  cooldowns get 1 to 3. Talent abilities get further ranks at the trainer after you take the talent.
- At level 60 everything is at its top rank.

The rank levels in the tables below follow WoW Classic, squeezed or shifted where the GBA version
introduces an ability earlier.

### Warrior: shared kit

| Ability | What it does | Ranks (learned at level) | Bar |
| --- | --- | --- | --- |
| Heroic Strike ✓ | Next swing hits harder | 1, 8, 16, 24, 32, 40, 48, 56 | Combat |
| Charge ✓ | Rush in, stun, gain rage | 4, 26, 46 | Combat |
| Battle Shout ✓ | Attack power buff | 1, 12, 22, 32, 42, 52, 60 | Buffs |
| Hamstring ✓ | Slow the target | 8, 32, 54 | Utility |
| Execute ✓ | Finisher under 20% health | 10, 24, 32, 40, 48, 56 | Combat |
| Pummel / Shield Bash | Interrupt a cast (Pummel for Arms and Fury, Shield Bash for Protection) | 12, 38 | Utility |
| Intimidating Shout | Fear nearby enemies for a few seconds | 22 | Utility |

### Arms (Battle Stance)

| Ability | What it does | Ranks |
| --- | --- | --- |
| Rend ✓ | Bleed over time | 4, 10, 20, 30, 40, 50, 60 |
| Overpower | Instant strike after the enemy dodges, can't be dodged | 12, 28, 44, 60 |
| Thunder Clap ✓ | Area damage and slow | 6, 18, 28, 38, 48, 58 |
| Sweeping Strikes (talent) | Next 5 swings also hit a second enemy | 30 |
| Mortal Strike ✓ (talent capstone) | Big strike, cuts enemy healing | 40, 48, 54, 60 |
| Retaliation | For 15 s, counter every melee hit | 20 |
| Deep Wounds (talent) | Crits make the target bleed | passive |
| Whirling Blades (talent, inspired by Bladestorm) | Spin for 6 s hitting everything | 50 |

### Fury (Berserker Stance)

| Ability | What it does | Ranks |
| --- | --- | --- |
| Cleave | Next swing hits the target and one more | 6, 20, 30, 40, 50 |
| Slam | Short-cast heavy hit | 10, 20, 34, 46, 54 |
| Whirlwind | Hit every nearby enemy | 24 |
| Berserker Rage | Gain rage, immune to fear for 10 s | 22 |
| Demoralizing Shout | Lowers nearby enemies' attack | 14, 24, 34, 44, 54 |
| Recklessness | Every hit crits for 15 s, you take more damage | 50 |
| Death Wish (talent) | +20% damage for 30 s | 30 |
| Bloodthirst ✓ (talent capstone) | Strike and heal yourself | 40, 48, 54, 60 |

### Protection (Defensive Stance, needs a shield)

| Ability | What it does | Ranks |
| --- | --- | --- |
| Sunder Armor | Stack armor reduction on the target | 6, 18, 30, 42, 54 |
| Revenge | Strong counter after you block, dodge or parry | 10, 20, 30, 40, 50, 60 |
| Shield Block | Block the next hits for 5 s | 16 |
| Demoralizing Shout | Lowers nearby enemies' attack | 14, 24, 34, 44, 54 |
| Disarm | Take the target's weapon for 10 s | 18 |
| Shield Wall | Take 75% less damage for 10 s | 28 |
| Last Stand ✓ (talent) | +30% maximum health for 20 s | 20 |
| Concussion Blow (talent) | Stun the target | 30 |
| Shield Slam (talent capstone) | Big shield hit that strips an enemy buff | 40, 48, 54, 60 |

### Mage: shared kit

| Ability | What it does | Ranks | Bar |
| --- | --- | --- | --- |
| Arcane Intellect | Raises intellect for 30 minutes | 1, 14, 28, 42, 56 | Buffs |
| Conjure Water | Makes drinks (no more buying water) | 4, 10, 20, 30, 40, 50, 60 | Buffs |
| Conjure Food | Makes food | 6, 12, 22, 32, 42, 52 | Buffs |
| Blink | Teleport 4 tiles forward | 20 | Utility |
| Counterspell | Interrupt a cast and silence its school | 24 | Utility |
| Polymorph | Turn one enemy into a sheep for 20 s; damage breaks it | 8, 20, 40, 60 | Utility |
| Teleport: Stormwind / Ironforge | Like a second hearthstone, costs a rune | 20, 30 | Buffs |

### Fire

| Ability | What it does | Ranks |
| --- | --- | --- |
| Fireball ✓ | Cast, then burns over 4 s | 1, 6, 12, 18, 24, 30, 36, 42, 48, 54, 60 |
| Fire Blast ✓ | Instant fire hit | 6, 14, 22, 30, 38, 46, 54 |
| Scorch | Short cast, stacks fire vulnerability | 22, 28, 34, 40, 46, 52, 58 |
| Flamestrike | Fire area at the target after a cast, then burns the ground | 16, 24, 32, 40, 48, 56 |
| Fire Ward | Absorb fire damage | 20, 30, 40, 50, 60 |
| Molten Armor | Armor buff that burns attackers (inspired by TBC) | 10, 30, 50 |
| Pyroblast ✓ (talent) | Huge slow fireball | 20, 24, 30, 36, 42, 48, 54, 60 |
| Blast Wave (talent) | Fire ring around you, knocks back and slows | 30, 36, 44, 52, 60 |
| Combustion (talent capstone) | Your fire spells crit more and more for 15 s | 40 |

### Frost

| Ability | What it does | Ranks |
| --- | --- | --- |
| Frostbolt ✓ | Damage and slow | 1, 8, 14, 20, 26, 32, 38, 44, 50, 56 |
| Frost Armor ✓ / Ice Armor | Armor buff, slows attackers (Ice Armor from 30) | 1, 10, 20, 30, 40, 50, 60 |
| Frost Nova ✓ | Freeze nearby enemies in place | 10, 26, 40, 54 |
| Cone of Cold | Frost cone in front of you | 26, 34, 42, 50, 58 |
| Blizzard | Channel ice on an area | 20, 28, 36, 44, 52, 60 |
| Ice Lance | Instant, triple damage to frozen targets (shatter combo, inspired by TBC) | 16, 32, 48 |
| Ice Barrier ✓ (talent) | Absorb shield | 30, 40, 46, 52, 58 |
| Cold Snap (talent) | Reset every frost cooldown | 30 |
| Ice Block | Encase yourself: immune for 10 s, can't act | 30 |
| Summon Water Elemental ✓ (talent capstone, inspired by TBC) | A frost pet for 45 s that casts Frostbolt and Freeze | 40 |

### Arcane

| Ability | What it does | Ranks |
| --- | --- | --- |
| Arcane Missiles ✓ | Channel missiles | 1, 8, 16, 24, 32, 40, 48, 56 |
| Arcane Explosion ✓ | Area around you | 14, 22, 30, 38, 46, 54 |
| Arcane Blast | Fast cast, each one stronger and pricier than the last (inspired by TBC) | 10, 30, 50 |
| Mana Shield | Damage drains mana instead of health | 20, 28, 36, 44, 52, 60 |
| Mage Armor | Mana regeneration buff | 34, 46, 58 |
| Slow | Slow the target's movement and casting | 20 |
| Evocation | Channel to restore most of your mana | 20 |
| Presence of Mind (talent) | Next spell is instant | 30 |
| Arcane Power ✓ (talent capstone) | +30% spell damage for 15 s | 40 |

### Hunter: shared kit

| Ability | What it does | Ranks | Bar |
| --- | --- | --- | --- |
| Hunter's Mark ✓ | Target takes more damage | 6, 22, 40, 58 | Combat |
| Aspect of the Hawk ✓ | Ranged attack power buff | 10, 18, 28, 38, 48, 58 | Buffs |
| Aspect of the Monkey | Dodge buff | 4 | Buffs |
| Aspect of the Cheetah | Run 30% faster out of combat, dazed when hit | 20 | Buffs |
| Concussive Shot ✓ | Daze and slow | 8 | Utility |
| Wing Clip | Melee slow | 12, 38, 60 | Utility |
| Feign Death | Drop combat; enemies walk away | 30 | Utility |
| Raptor Strike ✓ | Melee attack | 1, 8, 16, 24, 32, 40, 48, 56 | Combat |

### Beast Mastery (with a pet)

This is the one new system the subclass brings: a **pet** that follows you, attacks your target and
can be told to stay passive. At level 10 you get the quest *Taming the Beast* and tame a wolf, boar,
cat or spider out in the world (any beast type the game already draws). The pet has its own small
health bar under yours and levels with you. Pet commands go on the utility bar. Since M16 the pet
also holds the attention of what it bites: enemies keep a threat for the hunter and one for the pet
and swing at the higher. Pet Passive is the "stay passive" command, and the first pet is a young wolf
or any other beast up to the hunter's level.

| Ability | What it does | Ranks |
| --- | --- | --- |
| Call / Dismiss Pet ✓ | | 10 |
| Revive Pet ✓ | | 10 |
| Mend Pet ✓ | Heal the pet over time | 12, 20, 28, 36, 44, 52, 60 |
| Serpent Sting ✓ | Poison over 15 s | 4, 10, 18, 26, 34, 42, 50, 58 |
| Arcane Shot ✓ | Instant shot | 6, 12, 20, 28, 36, 44, 52, 60 |
| Kill Command ✓ | The pet hits hard (inspired by TBC) | 20, 40, 60 |
| Intimidation ✓ (talent) | The pet stuns the target | 30 |
| Aspect of the Beast ✓ | You and your pet hit harder (replaces Hawk for BM) | 30 |
| Bestial Wrath ✓ (talent capstone) | Pet goes berserk, you attack 40% faster for 15 s | 40 |

### Marksmanship

| Ability | What it does | Ranks |
| --- | --- | --- |
| Arcane Shot ✓ | Instant shot | 6, 12, 20, 28, 36, 44, 52, 60 |
| Serpent Sting ✓ | Poison over 15 s | 4, 10, 18, 26, 34, 42, 50, 58 |
| Multi-Shot ✓ | Hit the target and two more | 14, 26, 38, 50, 60 |
| Aimed Shot ✓ (now trained) | Slow aimed shot | 20, 28, 36, 44, 52, 60 |
| Rapid Fire | Shoot 40% faster for 15 s | 26 |
| Volley | Rain arrows on an area | 40, 50, 58 |
| Scatter Shot (talent) | Disorient the target and step back | 30 |
| Trueshot Aura (talent capstone) | Permanent attack power aura | 40, 50, 60 |

### Survival (melee and traps)

Traps are dropped at your feet and trigger when an enemy walks over them, so they work well with
free movement: lay one, step back, let the enemy run into it.

| Ability | What it does | Ranks |
| --- | --- | --- |
| Raptor Strike ✓ | Melee attack | 1, 8, 16, 24, 32, 40, 48, 56 |
| Mongoose Bite | Strong counter after you dodge | 16, 30, 44, 58 |
| Serpent Sting ✓ | Poison over 15 s | 4, 10, 18, 26, 34, 42, 50, 58 |
| Immolation Trap | Burns the enemy that steps on it | 16, 26, 36, 46, 56 |
| Freezing Trap | Freezes the enemy that steps on it | 20, 40, 60 |
| Frost Trap | Ice patch that slows | 28 |
| Explosive Trap | Fire burst on an area | 34, 44, 54 |
| Deterrence | Dodge everything for 10 s | 20 |
| Counterattack ✓ (talent) | Strike and pin after a parry | 30, 42, 54 |
| Wyvern Sting (talent capstone) | Puts the target to sleep, then poisons it | 40, 50, 60 |

### Talent trees

Each subclass tree has 18 talents in 7 tiers (tier 2 opens at 5 points, tier 3 at 10, and so on).
The existing 8 talents per tree stay as the lower tiers. Signature talents taken from WoW Classic:

- **Arms**: Deflection, Improved Rend, Tactical Mastery, Deep Wounds, Impale, Two-Handed Weapon
  Specialization, Sweeping Strikes, Mortal Strike.
- **Fury**: Cruelty, Unbridled Wrath, Improved Battle Shout, Dual Wield Specialization, Flurry,
  Enrage, Death Wish, Bloodthirst.
- **Protection**: Shield Specialization, Anticipation, Toughness, Improved Revenge, Last Stand,
  Concussion Blow, One-Handed Weapon Specialization, Shield Slam.
- **Fire**: Improved Fireball, Ignite, Impact, Incinerate, Pyroblast, Blast Wave, Critical Mass,
  Fire Power, Combustion.
- **Frost**: Improved Frostbolt, Frostbite, Ice Shards, Permafrost, Shatter, Arctic Reach, Cold
  Snap, Ice Barrier, Winter's Chill.
- **Arcane**: Arcane Subtlety, Arcane Focus, Clearcasting, Improved Arcane Missiles, Arcane Mind,
  Presence of Mind, Arcane Instability, Arcane Power.
- **Beast Mastery**: Improved Aspect of the Hawk, Endurance Training, Unleashed Fury, Ferocity,
  Intimidation, Bestial Discipline, Frenzy, Bestial Wrath.
- **Marksmanship**: Lethal Shots, Efficiency, Improved Hunter's Mark, Mortal Shots, Scatter Shot,
  Barrage, Ranged Weapon Specialization, Trueshot Aura.
- **Survival**: Monster Slaying, Savage Strikes, Deflection, Clever Traps, Surefooted, Killer
  Instinct, Counterattack, Lightning Reflexes, Wyvern Sting.

---

## 6. Controls and keybinds

The GBA has A, B, L, R, Start, Select and the D-pad. Today, holding R turns A, B, L and the four
D-pad directions into 7 ability slots. That stays exactly the same, and two more bars are added on
harder combinations for the abilities you use less often.

| Bar | How to use it | Slots | Meant for |
| --- | --- | --- | --- |
| **Combat** | Hold **R**, then A, B, L, ↑, ↓, ← or → | 7 | Your rotation: Fireball, Mortal Strike, Arcane Shot… |
| **Utility** | Hold **L**, then A, B, ↑, ↓, ← or → | 6 | Interrupts, crowd control, cooldowns, pet commands |
| **Buffs** | Hold **L and R** together, then A, B, ↑, ↓, ← or → | 6 | Long buffs and travel: Battle Shout, Arcane Intellect, armors, aspects, conjuring, mount, teleports |
| **Items** | Hold **Select**, then ↑, ↓, ← or → | 4 | Health potion, mana potion, food, drink (or any usable item) |

What changes for buttons you already know:

- **L alone** still switches target, but on release, and only if you didn't press anything while
  holding it and let go quickly. A quick tap of L feels the same as today.
- **Tapping Select** still uses the best consumable for the moment, as today.
- **B** still runs when nothing else is held.
- While a bar is held, the HUD shows that bar's icons around a small cross so you can see which
  button does what, as it does now for the R bar.
- In the Spells page of the menu, A puts a spell on a bar: first pick the bar with L/R, then the
  slot.

**Buff reminder.** Since long buffs live on the hardest bar, a small icon blinks next to the health
bar when one of your long buffs (shout, intellect, armor, aspect) is missing outside combat, so
you know to press L+R.

That gives 19 ability slots and 4 item slots. Every subclass kit fits with room left, so nothing
has to be dropped from the bars.

---

## 7. Enemy abilities

Today enemies only auto-attack, and seven bosses have hard-coded special moves. The plan is a shared
table of enemy abilities that any enemy can use, so new enemies get abilities by data instead of new
code.

- Each `enemy_def` gets up to **two ability slots** and an "AI style" (melee, caster, ranged,
  healer, runner).
- About **one in three normal enemy types** gets one ability. **Every elite** gets two. **Bosses**
  keep their scripted phases and can also use table abilities.
- Casts show a **cast bar over the enemy**, and the player's interrupts (Pummel, Shield Bash,
  Counterspell, Scatter Shot, Wyvern Sting) can stop them. Ground attacks show a **red circle**
  first, as Bazil's smoke bombs already do.
- Enemies that **flee** at low health run off to bring friends, as murlocs and gnolls do in WoW.

The shared list (about 40 abilities, each with a short example):

| Kind | Abilities (who uses them) |
| --- | --- |
| Caster | Fireball (Defias Wizards), Frostbolt (Dark Iron mages), Lightning Bolt and Healing Wave (gnoll and troll shamans), Shadow Bolt (cultists, Scholomance), Holy Smite and Heal (Scarlet priests) |
| Melee | Rend (Defias Thugs), Sunder Armor (orcs), Thrash (gnolls), Mortal Strike (Blackrock elites), Cleave (ogres), Knockdown (Dark Irons) |
| Control | Net (Defias Trappers), Frost Nova (wizards), Fear / Howl (worgen, banshees), Sleep (Scarlet priests), Polymorph (Syndicate mages), Web (spiders) |
| Over time | Poison (spiders, scorpids), Disease (ghouls, Plaguelands), Curse of Weakness (Shadowfang, cultists), Burning (fire elementals) |
| Movement | Charge (orcs, boars), Leap (worgen, raptors), Knockback / War Stomp (ogres, tauren), Blink (Arcane casters) |
| Support | Call for Help (murlocs, quilboar), Enrage at 30% (wolves, worgen), Battle Shout (orc packs), Shield Wall (guards), Summon Skeleton (necromancers), Flee at 15% (murlocs, gnolls, kobolds) |
| Defense | Shield Block (Scarlet defenders), Evasion (rogue-like Defias), Stoneskin (golems), Mana Shield (casters) |

Existing enemies that get one in the first pass: Kobold Tunneler (Candle Throw, a small fire hit),
Murloc (Call for Help, Flee), Riverpaw Gnoll (Thrash), Defias Trapper (Net), Defias Thug (Rend),
Forest Spider (Poison), Gnoll Brute (Enrage), Defias Pirate (Cleave). M15 also gave Rockhide Boars
Charge, and the elites two each: Princess (Charge, Enrage), Hogger (Thrash, Knockdown), Targorr
(Charge, Mortal Strike) and Kam Deepfury (Knockdown, Shield Wall). Kobolds, murlocs and gnolls flee.
Sneed, VanCleef and Bazil keep only their scripted phases for now. Summon Skeleton waits for
Duskwood's skeletons, and Knockback is War Stomp's stun until the game has a way to push the player.

---

## 8. Bags, items and leveling

### Bags with no limit

- The 16-slot bag becomes **one list that never fills up**. Each item takes one row with a count
  (×1 to ×999). Equipment that can't stack still takes one row each.
- "Bags full" disappears everywhere: loot, quest rewards, chests and vendors stop checking for it.
- On the GBA, "no limit" means room for 400 different rows, which a whole game's worth of loot
  will not fill. It costs about 1.6 KB of the 32 KB save memory.

### Sorting by type

In the Bags page, **Select cycles the sort order** and the order is remembered:

1. **By type** (default): Equipment grouped by slot (head, chest, hands, legs, feet, weapons,
   off-hand, ranged), then consumables (potions, food, drink), then quest items, then junk.
2. **By quality**: epic, rare, uncommon, common, junk.
3. **By level**: highest required level first.
4. **Newest first**.

Headings ("Weapons", "Consumables", "Junk") show between groups, and the list scrolls with
up/down, with left/right on the D-pad jumping a whole group (L and R still switch menu pages).
Dropping an item moves from Select to a small menu on A (Use / Equip / Item bar / Drop). Vendors
keep "sell all junk".

### Items for 60 levels

- About 900 items in all: greens from quests and enemies in every zone, a blue from every dungeon
  boss, and a few purples from the final bosses of Blackrock Spire, Scholomance, Stratholme and
  Onyxia.
- Each dungeon's last boss drops a piece of a dungeon set, inspired by WoW's Dungeon Set 1: Battlegear
  of Valor (Warrior), Magister's Regalia (Mage), Beaststalker Armor (Hunter). Collecting pieces from
  different dungeons gives set bonuses.
- Item names, stats and rarity follow WoW items of the same level where one fits.
- More equipment slots become possible but are not required: shoulders, back, rings and a trinket
  would fit the menu. They are listed as optional in milestone M18.

### Leveling curve

- `max_level` goes from 20 to 60, and the XP table extends with the same shape as WoW's, scaled so
  each 5-level zone takes about 1.5 hours with its quests, plus 20 to 30 minutes per dungeon.
- That makes roughly 25 to 30 hours from level 1 to 60, compared to about 4 today.
- **Rest XP**: sleeping in an inn (or saving and quitting there) gives double XP for a while, as in
  WoW. Cheap to add and it softens the longer curve.
- Westfall and the Deadmines are retuned so Redridge fits: the Westfall quests end at about level 15,
  Redridge covers 15 to 18, and the Deadmines and Stockade finish the chapter at about 20.

---

## 9. Engine work before the content

These changes come first, because every later milestone builds on them.

| Change | Why | Files |
| --- | --- | --- |
| `max_level` 60, new XP table, rest XP | Level cap | `gw_character.*` |
| `item_id`, `enemy_id`, `npc_id`, `map_id`, `quest_id` to `uint16_t` | More than 255 of each | `gw_ids.h`, `gw_item_ids.h`, `gw_quest_ids.h` |
| Ability ranks table, `known_abilities` as a rank per ability instead of a 32-bit mask | Ranks and about 150 abilities | `gw_abilities.*`, `gw_trainer_screen.cpp`, `gw_menu_spells.cpp` |
| `subclass_id` on the character, subclass kits, trainer filter, creation screen step | Subclasses | `gw_character.*`, `gw_character_creation.cpp` |
| One talent tree per subclass, 18 talents, 51 points, tiers every 5 points | Talents | `gw_talents.*`, `gw_menu_talents.cpp` |
| Three action bars plus item bar, held-button logic, HUD cross, buff reminder | Keybinds | `gw_game.cpp`, `gw_combat.cpp`, `gw_hud.cpp` |
| Enemy ability table, cast bars, interrupts, AI styles, flee and call for help | Enemy abilities | `gw_enemy_data.*`, `gw_enemies.*`, `gw_combat.cpp` |
| Bags as a counted list with sorting and group headings | Bags | `gw_character.*`, `gw_menu_bags.cpp`, `gw_vendor_screen.cpp`, `gw_chests.cpp`, `gw_loot.cpp` |
| Hunter pet: follow, attack, passive, health bar, taming quest | Beast Mastery | new `gw_pet.*` |
| Traps, ground-target spells (Flamestrike, Blizzard, Volley), summons (Water Elemental) | Subclass kits | `gw_combat.cpp`, `gw_effects.*` |
| Flight masters, boats, tram, mount speed | Travel | new `gw_travel.*`, `gw_world.cpp`, `gw_menu_map.cpp` |
| Save version 4 and migration from version 3 | Keep old saves | `gw_save.cpp` |

**Old saves.** A version 3 save loads into version 4. The character keeps its level, gear, gold,
quests and position. Because old saves have no subclass, the first time one loads the game asks you
to pick one (the talent tree with the most points is preselected), refunds the talent points, and
maps known abilities to rank 1 of each ability that subclass keeps, then lets the trainer sell the
rest.

**Memory and size.** The ROM grows from 1.1 MB to an estimated 10 to 14 MB with twelve zones and
twenty dungeons, well under the cartridge's 32 MB. The save grows from about 300 bytes to about
3 KB, inside the 32 KB of SRAM. The rule of 1024 unique tiles per background still applies, so each
zone gets its own tileset, and large zones like the Plaguelands may be split into two connected maps.

---

## 10. Milestones

Each milestone ends with a playable ROM, CI green, and screenshots, as before. Content milestones
are one bracket each so they can be played and tuned one at a time.

| Milestone | What it adds | Depends on |
| --- | --- | --- |
| **M12 Systems for 60** ✓ | Level cap 60, XP table, rest XP, wider ids, save version 4 and migration | |
| **M13 Subclasses and ranks** ✓ | Subclass choice at creation, 9 kits, ranks for every existing ability, trainer and spellbook changes, one 18-talent tree per subclass (levels 10 to 20 filled in, higher tiers stubbed) | M12 |
| **M14 Keybinds and bags** ✓ | Combat, Utility, Buffs and Items bars, HUD cross, buff reminder; unlimited bags with sorting | M12 |
| **M15 Enemy abilities** ✓ | Shared enemy ability table, cast bars, interrupts, flee and call for help; abilities on the existing Elwynn and Westfall enemies | M12 |
| **M16 Travel and the pet** ✓ | Flight masters, boats, Deeprun Tram, mount at 30, two-level world map; Beast Mastery pet and taming | M12 |
| **M17 Redridge** (15–20) ✓ | Lakeshire, about 12 quests, retuned Westfall, Deadmines and Stockade; the old ending becomes the end of the chapter | M13–M16 |
| **M18 Duskwood and Shadowfang Keep** (20–25) ✓ | Night palette, Darkshire, Stitches, Silverpine entrance, SFK. Optional new gear slots (left out for now) | M17 |
| **M19 Wetlands, Ironforge, BFD, Gnomeregan** (25–30) ✓ | Second capital, two dungeons; Dun Morogh and Darkshore as the ways in | M18 |
| **M20 Hillsbrad and Scarlet Monastery 1** (30–35) ✓ | Southshore, Graveyard, Library, mount | M19 |
| **M21 Stranglethorn and Scarlet Monastery 2** (35–40) | Booty Bay, Nesingwary, Armory, Cathedral | M20 |
| **M22 Tanaris and the Razorfens** (40–45) | Kalimdor by boat, Gadgetzan, Razorfen Kraul, Razorfen Downs, Zul'Farrak | M21 |
| **M23 Feralas** (45–50) | Feathermoon, Maraudon, Dire Maul | M22 |
| **M24 Burning Steppes** (50–55) | Morgan's Vigil, Uldaman, Sunken Temple, Blackrock Depths | M23 |
| **M25 Plaguelands** (55–60) | Light's Hope, Blackrock Spire, Scholomance, Stratholme, all talent tiers and top ranks | M24 |
| **M26 Onyxia and the ending** | Prestor unmasked in Stormwind Keep, Onyxia's three-phase fight, new ending, dungeon set bonuses, full balance pass 1 to 60 | M25 |

M12 to M16 are engine work and can be played on the current 1–20 content: by the end of M16, the
existing world already has subclasses, ranks, the new bars, sorted bags, smarter enemies, flight
paths and a pet. From M17 on, every milestone is a new chapter of world.

---

## 11. Not in this plan

- **Professions** (asked to leave them out). Loot that would be crafting materials is replaced by
  junk you sell or by quest items.
- **Groups, raids and other players.** Dungeons are tuned for one hero. Onyxia is the only "raid"
  boss and is built as a solo fight.
- **Reputation.** Where WoW used it (Argent Dawn, Thorium Brotherhood), quests and token vendors
  stand in for it.
- **Horde zones and races.** The route stays Alliance; Horde places (Silverpine, Tirisfal, the
  Barrens edge) only appear as dungeon entrances.
- **New races or classes.** Paladin, Priest, Rogue, Warlock and Druid could come later; the subclass
  system is built so each new class is three kits and three trees.

## 12. Open questions for later

These don't block anything; the plan above uses the default in bold.

- Mount level: **30** (smaller GBA zones) or WoW's 40.
- New gear slots (shoulders, back, rings, trinket): **optional in M18**, or never.
- Subclass change at a trainer: **not planned**, or allowed for a large fee.
