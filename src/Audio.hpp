/**
 * Copyright (c) 2020 Brandon McGriff
 *
 * Licensed under the MIT license; see the LICENSE-src file at the top level
 * directory for the full text of the license.
 */
#pragma once

#include <string>
#include "SDL_mixer.h"

struct coreState;

namespace Shiro {
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
         * Loads the music from a file. Provide the name of the file, without
         * the ".ogg" or ".wav" extension. Will try to load OGG first,
         * otherwise WAV. Returns true if the file was loaded.
         */
        bool load(std::string filenameNoExt);

        /**
         * Plays the music track. Returns true if the music track was played.
         */
        bool play(coreState* cs);

        int volume;

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
         * Plays the sound effect. Returns true if the sound effect was played.
         */
        bool play(coreState* cs);

        int volume;

    private:
        std::string filename;
        Mix_Chunk *data;
    };
}