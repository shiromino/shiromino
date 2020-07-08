/**
 * Copyright (c) 2020 Brandon McGriff
 *
 * Licensed under the MIT license; see the LICENSE-src file at the top level
 * directory for the full text of the license.
 */
#include "Audio.h"
#include "SDL_mixer.h"
#include "Config.h"
#include <filesystem>
namespace Shiro {
    Music::Music() : volume(0.0f), data(nullptr) {
    }

    Music::~Music() {
        if(data) {
            Mix_FreeMusic(data);
        }
    }

    bool Music::load(std::filesystem::path&& path) {
        data = nullptr;
        volume = 100.0f;
        data = Mix_LoadMUS(path.concat(".ogg").c_str());
        if(data) {
            return true;
        }
        data = Mix_LoadMUS(path.concat(".wav").c_str());
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

    Sfx::Sfx() : volume(0.0f), data(nullptr) {}

    Sfx::~Sfx() {
        if (data) {
            Mix_FreeChunk(data);
        }
    }

    bool Sfx::load(std::filesystem::path&& filenameWithoutExtension) {
        data = nullptr;
        volume = 100.0f;
        data = Mix_LoadWAV(filenameWithoutExtension.concat(".wav").c_str());
        return data != nullptr;
    }

    bool Sfx::play(Settings& settings) {
        if(!data) {
            return false;
        }

        Mix_VolumeChunk(data, static_cast<int>(MIX_MAX_VOLUME * (volume / 100.0f) * (settings.sfxVolume / 100.0f) * (settings.masterVolume / 100.0f)));
        Mix_PlayChannel(-1, data, 0);

        return true;
    }
}