/**
 * Copyright (c) 2020 Brandon McGriff
 *
 * Licensed under the MIT license; see the LICENSE-src file at the top level
 * directory for the full text of the license.
 */
#pragma once

#include "Settings.hpp"
#include "Path.hpp"
#include <string>
#include "SDL_mixer.h"

struct CoreState;

namespace Shiro {
    // TODO: Allow selecting playback channel?

    /**
     * Manages loading and playback of a music track.
     */
    class Music {
    public:
        Music();
        Music(const Music&) = delete;
        Music& operator=(const Music&) = delete;
        ~Music();

        /**
         * Loads the music from a file. directory is the directory where the
         * audio file is. name is the name of the file, without the ".ogg" or
         * ".wav" extension. Will try to load OGG first, otherwise WAV. Returns
         * true if the file was loaded.
         */
        bool load(std::string directory, std::string name);

        /**
         * Plays the music track. Returns true if the music track was played;
         * playback only fails if no music track is loaded. The master and
         * music volume settings will scale the volume the music track plays
         * at; if they're both 100%, Music::volume will be the played volume.
         */
        bool play(Settings& settings);

        /**
         * The volume of the music track when played. This is a percentage,
         * with 100.0f being max volume.
         */
        float volume;

    private:
        Mix_Music *data;
    };

    /**
     * Manages loading and playback of a sound effect.
     */
    class Sfx {
    public:
        Sfx();
        Sfx(const Sfx&) = delete;
        Sfx& operator=(const Sfx&) = delete;
        ~Sfx();

        /**
         * Loads the sound effect from a file. Provide the name of the file,
         * without the ".wav" extension. Returns true if the file was loaded.
         */
        bool load(std::string filenameNoExt);

        /**
         * Plays the sound effect. Returns true if the sound effect was played;
         * playback only fails if no sound effect is loaded. The master and
         * sound effect volume settings will scale the volume the sound effect
         * plays at; if they're both 100%, Sfx::volume will be the played
         * volume.
         */
        bool play(Settings& settings);

        /**
         * The volume of the sound effect when played. This is a percentage,
         * with 100.0f being max volume.
         */
        float volume;

    private:
        Mix_Chunk *data;
    };
}