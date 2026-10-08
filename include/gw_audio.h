#ifndef GW_AUDIO_H
#define GW_AUDIO_H

#include "gw_ids.h"

namespace gw
{

enum class sound_id : uint8_t
{
    HIT,
    SPELL,
    LEVEL_UP,
    QUEST,
    COIN,
    SELECT,
    DEATH
};

// Starts a looping tune; does nothing if it's already playing. music_id::NONE stops the music.
void play_music(music_id music);

[[nodiscard]] music_id current_music();

void play_sound(sound_id sound);

}

#endif
