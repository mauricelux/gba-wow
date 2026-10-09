#include "gw_audio.h"

#include "bn_music.h"
#include "bn_music_items.h"
#include "bn_sound_items.h"

namespace gw
{

namespace
{
    constexpr bn::fixed music_volume = 0.45;
    constexpr bn::fixed sound_volume = 0.7;

    music_id playing = music_id::NONE;

    [[nodiscard]] bn::optional<bn::music_item> music_item(music_id music)
    {
        switch(music)
        {

        case music_id::TITLE:
            return bn::music_items::title;

        case music_id::ELWYNN:
            return bn::music_items::elwynn;

        case music_id::TOWN:
            return bn::music_items::town;

        case music_id::WESTFALL:
            return bn::music_items::westfall;

        case music_id::DUNGEON:
            return bn::music_items::dungeon;

        case music_id::BOSS:
            return bn::music_items::boss;

        case music_id::REDRIDGE:
            return bn::music_items::redridge;

        case music_id::DUSKWOOD:
            return bn::music_items::duskwood;

        case music_id::IRONFORGE:
            return bn::music_items::ironforge;

        case music_id::WETLANDS:
            return bn::music_items::wetlands;

        case music_id::HILLSBRAD:
            return bn::music_items::hillsbrad;

        case music_id::MONASTERY:
            return bn::music_items::monastery;

        case music_id::STRANGLETHORN:
            return bn::music_items::stranglethorn;

        case music_id::TANARIS:
            return bn::music_items::tanaris;

        default:
            return bn::nullopt;
        }
    }

    [[nodiscard]] bn::sound_item sound_item(sound_id sound)
    {
        switch(sound)
        {

        case sound_id::SPELL:
            return bn::sound_items::sfx_spell;

        case sound_id::LEVEL_UP:
            return bn::sound_items::sfx_level_up;

        case sound_id::QUEST:
            return bn::sound_items::sfx_quest;

        case sound_id::COIN:
            return bn::sound_items::sfx_coin;

        case sound_id::SELECT:
            return bn::sound_items::sfx_select;

        case sound_id::DEATH:
            return bn::sound_items::sfx_death;

        default:
            return bn::sound_items::sfx_hit;
        }
    }
}

void play_music(music_id music)
{
    if(music == playing)
    {
        return;
    }

    playing = music;

    if(bn::optional<bn::music_item> item = music_item(music))
    {
        item->play(music_volume);
    }
    else if(bn::music::playing())
    {
        bn::music::stop();
    }
}

music_id current_music()
{
    return playing;
}

void play_sound(sound_id sound)
{
    sound_item(sound).play(sound_volume);
}

}
