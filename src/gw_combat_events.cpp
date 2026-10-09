#include "gw_combat.h"

#include "bn_math.h"
#include "bn_string.h"

#include "gw_audio.h"
#include "gw_enemies.h"
#include "gw_hud.h"
#include "gw_maps.h"
#include "gw_map_blackrock_depths.h"
#include "gw_map_blackrock_spire.h"
#include "gw_map_dire_maul.h"
#include "gw_map_razorfen_downs.h"
#include "gw_map_sunken_temple.h"
#include "gw_map_uldaman.h"
#include "gw_map_zul_farrak.h"
#include "gw_player.h"
#include "gw_types.h"
#include "gw_ui.h"
#include "gw_world.h"

namespace gw
{

// Dungeon events the player starts with A: a gong rung, a cage opened, a prison's field broken, an
// altar woken or a fight in the ring called, inside an area_id::GONG, CAGE, PRISON, ALTAR or ARENA
// area (event_area_at). A sealed event (a prison, the Sunken Temple's altar) holds until every brazier
// of the map is lit (or pylon shut down). An event sends waves of enemies at the hero from its spots,
// then its bosses; the Ring of Law sends one champion picked at random. A gong sends one wave per ring,
// so the hero can eat between them; the cage sends its waves one after another, with a breather in
// between. An event lives only while the map is loaded: dying or running off ends it, and the hero can
// try again; once its bosses are dead it stays done until the map is loaded again, like any dungeon
// boss.

namespace
{
    constexpr int seconds = 60;
    constexpr int first_wave = 2 * seconds;
    constexpr int leash = 360;                      // running this far from the event ends it
    constexpr int wave_size = 3;
    constexpr int max_waves = 3;
    constexpr int minute = 60 * seconds;
    constexpr int baron_minutes = 6;

    struct event_def
    {
        map_id map;
        area_id area;
        const char* start_message;
        const char* wave_message;   // a later wave comes
        const char* beaten_message; // a wave is beaten and the hero rings again for the next
        const char* boss_message;
        const char* done_message;
        point_def spots[4];         // where each wave's enemies come from, in turn
        int spot_count;
        enemy_id waves[max_waves][wave_size];
        int wave_count;
        int pause;                  // frames between a beaten wave and the next; 0: the next waits for a ring
        point_def boss_spot;
        enemy_id bosses[2];
        const char* sealed_message = nullptr;   // while the map's braziers are not all lit
        const char* sealed_hint = nullptr;
        bool champion = false;                  // bosses[0] is one of the champions, picked at random
    };

    namespace brd = map_data::blackrock_depths;
    namespace brs = map_data::blackrock_spire;
    namespace dm = map_data::dire_maul;
    namespace rfd = map_data::razorfen_downs;
    namespace st = map_data::sunken_temple;
    namespace ul = map_data::uldaman;
    namespace zf = map_data::zul_farrak;

    constexpr enemy_id fiend = enemy_id::TOMB_FIEND;
    constexpr enemy_id slave = enemy_id::SANDFURY_SLAVE;
    constexpr enemy_id drudge = enemy_id::SANDFURY_DRUDGE;
    constexpr enemy_id guardian = enemy_id::EARTHEN_GUARDIAN;
    constexpr enemy_id warder = enemy_id::VAULT_WARDER;
    constexpr enemy_id guardsman = enemy_id::ANVILRAGE_GUARDSMAN;
    constexpr enemy_id warden = enemy_id::ANVILRAGE_WARDEN;
    constexpr enemy_id incarcerator = enemy_id::BLACKHAND_INCARCERATOR;
    constexpr enemy_id none = enemy_id::NONE;

    // The Ring of Law's champions.
    constexpr enemy_id champions[] = {
        enemy_id::GOROSH_THE_DERVISH, enemy_id::GRIZZLE, enemy_id::HEDRUM_THE_CREEPER, enemy_id::OK_THOR_THE_BREAKER
    };

    constexpr event_def events[] = {
        // Razorfen Downs: each ring of the gong calls tomb fiends out of their holes; the third calls
        // their mother.
        { map_id::RAZORFEN_DOWNS, area_id::GONG, "The gong echoes in the Downs!", "More crawl out of the dark!",
          "Ring the gong again!", "Tuten'kash crawls out!", "The gong is silent.",
          { rfd::wave_a, rfd::wave_b, rfd::wave_c, rfd::wave_d }, 4,
          { { fiend, fiend, fiend }, { fiend, fiend, fiend }, {} }, 2, 0,
          rfd::event_boss, { enemy_id::TUTEN_KASH, none } },
        // Zul'Farrak: the gong by the pool wakes Gahz'rilla.
        { map_id::ZUL_FARRAK, area_id::GONG, "The gong rings over the pool!", "", "", "Gahz'rilla rises!",
          "The pool is still.",
          { zf::gahzrilla }, 1, {}, 0, 0, zf::gahzrilla, { enemy_id::GAHZ_RILLA, none } },
        // Zul'Farrak: opening the cage on the pyramid brings the Sandfury up the stairs, then their
        // priests.
        { map_id::ZUL_FARRAK, area_id::CAGE, "The trolls storm the pyramid!", "More trolls climb the stairs!", "",
          "Nekrum and Sezz'ziz attack!", "The cage stands open.",
          { zf::wave_a, zf::wave_b, zf::wave_c }, 3,
          { { slave, drudge, none }, { slave, slave, drudge }, { slave, drudge, slave } }, 3, 15 * seconds,
          zf::event_boss, { enemy_id::NEKRUM_GUTCHEWER, enemy_id::SHADOWPRIEST_SEZZ_ZIZ } },
        // Dire Maul: with the four pylons shut down, Immol'thar's field falls and the demon comes out.
        { map_id::DIRE_MAUL, area_id::PRISON, "The force field falls!", "", "", "Immol'thar is free!",
          "The prison is empty.", { dm::event_boss }, 1, {}, 0, 0, dm::event_boss,
          { enemy_id::IMMOL_THAR, none }, "The force field holds", "Shut down the four pylons" },
        // Uldaman: touching the altar wakes the vault's guardians, then Archaedas himself.
        { map_id::ULDAMAN, area_id::ALTAR, "The vault's guardians wake!", "More guardians wake!", "",
          "Archaedas wakes!", "The altar is still.",
          { ul::wave_a, ul::wave_b, ul::wave_c, ul::wave_d }, 4,
          { { guardian, guardian, none }, { warder, guardian, warder }, {} }, 2, 12 * seconds,
          ul::event_boss, { enemy_id::ARCHAEDAS, none } },
        // The Sunken Temple: with the six statues lit in order, the altar wakes Atal'alarion.
        { map_id::SUNKEN_TEMPLE, area_id::ALTAR, "The altar flares!", "", "", "Atal'alarion rises!",
          "The altar is still.", { st::event_boss }, 1, {}, 0, 0, st::event_boss,
          { enemy_id::ATAL_ALARION, none }, "The altar is cold", "Light the six statues in order" },
        // Blackrock Depths: the Ring of Law sends the guards in, then a champion of the crowd's choice.
        { map_id::BLACKROCK_DEPTHS, area_id::ARENA, "The crowd roars for blood!", "The gates open again!", "",
          "A champion enters the ring!", "The ring is empty.",
          { brd::wave_a, brd::wave_b, brd::wave_c, brd::wave_d }, 4,
          { { guardsman, warden, none }, { warden, guardsman, warden }, {} }, 2, 12 * seconds,
          brd::event_boss, { enemy_id::GOROSH_THE_DERVISH, none }, nullptr, nullptr, true },
        // Blackrock Spire: with the seven altars lit, touching the runes breaks Emberseer's bonds. His
        // jailers come first.
        { map_id::BLACKROCK_SPIRE, area_id::PRISON, "The runes crack!", "More jailers rush in!", "",
          "Pyroguard Emberseer is free!", "The Hall of Binding is still.",
          { brs::wave_a, brs::wave_b, brs::wave_c, brs::wave_d }, 4,
          { { incarcerator, incarcerator, none }, { incarcerator, incarcerator, incarcerator }, {} }, 2,
          12 * seconds, brs::event_boss, { enemy_id::PYROGUARD_EMBERSEER, none }, "The seal holds",
          "Light the seven altars" },
    };

    constexpr int event_count = sizeof(events) / sizeof(events[0]);
    static_assert(event_count <= 8, "combat::_events_done has a bit per event");

    [[nodiscard]] bool event_boss(const event_def& event, enemy_id id)
    {
        if(event.champion)
        {
            for(enemy_id other : champions)
            {
                if(other == id)
                {
                    return true;
                }
            }

            return false;
        }

        return id != enemy_id::NONE && (id == event.bosses[0] || id == event.bosses[1]);
    }

    [[nodiscard]] bool event_enemy(const event_def& event, enemy_id id)
    {
        for(int wave = 0; wave < event.wave_count; ++wave)
        {
            for(enemy_id other : event.waves[wave])
            {
                if(other == id)
                {
                    return true;
                }
            }
        }

        return false;
    }
}

bool combat::start_event(area_id area, bool sealed)
{
    // Not while enemies fight the hero (a lingering disease is fine): then A attacks as usual.
    if(_enemies.any_in_combat())
    {
        return false;
    }

    map_id map = character().map;

    for(int index = 0; index < event_count; ++index)
    {
        const event_def& event = events[index];

        if(event.map != map || event.area != area)
        {
            continue;
        }

        if(_events_done & (1 << index))
        {
            _hud.message(event.done_message, ui::color::WHITE);
            return true;
        }

        if(event.sealed_message && sealed)
        {
            _hud.message(event.sealed_message, ui::color::RED);
            _hud.message(event.sealed_hint, ui::color::YELLOW);
            return true;
        }

        if(_event < 0)
        {
            // The first ring, or the cage opened: everything for a cage, one wave for a gong.
            _event = int8_t(index);
            _event_wave = 0;
            _event_rings = int8_t(event.pause ? event.wave_count + 1 : 1);
            _event_timer = first_wave;
            _hud.message(event.start_message, ui::color::RED);
        }
        else if(_event == index && _event_rings == _event_wave && _event_rings <= event.wave_count)
        {
            // The gong again, once its last wave is beaten.
            ++_event_rings;
            _event_timer = first_wave;
            _hud.message(event.start_message, ui::color::RED);
        }
        else
        {
            return true;
        }

        play_sound(sound_id::SPELL);
        _effects.burst(_player.position(), projectile_kind::ARCANE);
        return true;
    }

    return false;
}

void combat::_update_event()
{
    if(_event < 0)
    {
        return;
    }

    const event_def& event = events[_event];
    bn::fixed_point center(event.boss_spot.x, event.boss_spot.y);

    // Running off ends it: the enemies it sent go home and vanish.
    if(distance_squared(_player.position(), center) > leash * leash)
    {
        _event = -1;
        return;
    }

    bool waves_alive = false;
    bool bosses_alive = false;

    for(int index = 0, limit = _enemies.count(); index < limit; ++index)
    {
        const enemy& item = _enemies.at(index);

        if(item.summoned && item.alive())
        {
            waves_alive |= event_enemy(event, item.id);
            bosses_alive |= event_boss(event, item.id);
        }
    }

    if(_event_wave > event.wave_count)
    {
        // The bosses are out: when none is left standing, they died (_event_boss_killed marks the
        // event done) or went home.
        if(! bosses_alive)
        {
            _event = -1;
        }

        return;
    }

    // The next wave waits until the hero has beaten this one, and for a gong, until it rings again.
    if(waves_alive)
    {
        _event_timer = event.pause;
        _event_beaten = false;
        return;
    }

    if(_event_wave >= _event_rings)
    {
        if(! _event_beaten)
        {
            _event_beaten = true;
            _hud.message(event.beaten_message, ui::color::WHITE);
        }

        return;
    }

    if(--_event_timer > 0)
    {
        return;
    }

    if(_event_wave < event.wave_count)
    {
        for(int slot = 0; slot < wave_size; ++slot)
        {
            enemy_id id = event.waves[_event_wave][slot];
            const point_def& spot = event.spots[(_event_wave + slot) % event.spot_count];
            bn::fixed_point position(spot.x + (slot - 1) * 12, spot.y);

            if(id != enemy_id::NONE && _enemies.summon(id, position) >= 0)
            {
                _effects.burst(position, projectile_kind::SHADOW);
            }
        }

        if(_event_wave > 0)
        {
            _hud.message(event.wave_message, ui::color::RED);
        }

        _event_beaten = true;
    }
    else
    {
        for(int slot = 0; slot < 2; ++slot)
        {
            enemy_id boss = event.bosses[slot];

            if(event.champion && slot == 0)
            {
                boss = champions[random_range(0, int(sizeof(champions) / sizeof(champions[0])) - 1)];
            }

            if(boss != enemy_id::NONE)
            {
                bn::fixed_point position(event.boss_spot.x + slot * 28, event.boss_spot.y);
                _enemies.summon(boss, position);
                _effects.burst(position, projectile_kind::SHADOW);
            }
        }

        _hud.message(event.boss_message, ui::color::RED);
    }

    ++_event_wave;
}

void combat::_event_boss_killed(const enemy& boss)
{
    if(_event < 0)
    {
        return;
    }

    const event_def& event = events[_event];

    if(! event_boss(event, boss.id))
    {
        return;
    }

    for(int index = 0, limit = _enemies.count(); index < limit; ++index)
    {
        const enemy& item = _enemies.at(index);

        if(item.summoned && item.alive() && event_boss(event, item.id))
        {
            return;
        }
    }

    _events_done |= uint8_t(1 << _event);
    _event = -1;
}

void combat::_update_baron_clock()
{
    // Stratholme: stepping into the gauntlet starts the Baron's clock. Ysida Harmon is saved if
    // Rivendare dies before it runs out (_boss_killed); one try per visit.
    if(_baron_over || character().map != map_id::STRATHOLME)
    {
        return;
    }

    if(_baron_frames == 0)
    {
        int x = _player.position().x().floor_integer();
        int y = _player.position().y().floor_integer();

        if(event_area_at(world::map(), x, y) == area_id::GAUNTLET)
        {
            _baron_frames = baron_minutes * minute;
            _hud.message("Rivendare: Ysida dies soon!", ui::color::RED);
            _hud.message("Ysida has 6 minutes left", ui::color::YELLOW);
            play_sound(sound_id::SPELL);
        }

        return;
    }

    --_baron_frames;

    if(_baron_frames == 0)
    {
        _baron_over = true;
        _hud.message("Ysida Harmon is lost...", ui::color::RED);
    }
    else if(_baron_frames % minute == 0)
    {
        bn::string<32> text = "Ysida has ";
        int left = _baron_frames / minute;
        text += bn::to_string<4>(left);
        text += left == 1 ? " minute left" : " minutes left";
        _hud.message(text, ui::color::YELLOW);
    }
}

}
