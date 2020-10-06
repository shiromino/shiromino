/**
 * Copyright (c) 2020 Brandon McGriff
 *
 * Licensed under the MIT license; see the LICENSE-src file at the top level
 * directory for the full text of the license.
 */
#include "Audio.h"
#include "definitions.h"
#include "SDL_mixer.h"
#include <filesystem>
#include <iostream>

namespace Shiro {
    Music::Music() : volume(0.0f), data(nullptr) {
    }

    Music::~Music() {
        if(data) {
            Mix_FreeMusic(data);
        }
    }

    bool Music::load(const std::filesystem::path& path) {
        data = nullptr;
        volume = 100.0f;
        data = Mix_LoadMUS(path.string().append(".ogg").c_str());
        if(data) {
            return true;
        }
        data = Mix_LoadMUS(path.string().append(".wav").c_str());
        return data != nullptr;
    }

    bool Music::play(Settings& settings) {
        if (!data) {
            return false;
        }

        Mix_VolumeMusic(static_cast<int>(MIX_MAX_VOLUME * (volume / 100.0f) * (settings.musicVolume / 100.0f) * (settings.masterVolume / 100.0f)));
        Mix_PlayMusic(data, -1);

        return true;
    }
}