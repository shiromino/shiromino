/**
 * Copyright (c) 2020 Brandon McGriff
 *
 * Licensed under the MIT license; see the LICENSE-src file at the top level
 * directory for the full text of the license.
 */
#include "Asset/Sfx.h"
#include <cassert>

Shiro::SfxAssetLoader::SfxAssetLoader(const std::filesystem::path& basePath) :
    basePath(basePath) {}

Shiro::Asset* Shiro::SfxAssetLoader::create(const std::filesystem::path& location) const {
    return new Shiro::SfxAsset(location);
}

bool Shiro::SfxAssetLoader::load(Shiro::Asset& asset) const {
    assert(asset.getType() == Shiro::AssetType::sfx);
    Shiro::SfxAsset& sfxAsset = static_cast<Shiro::SfxAsset&>(asset);

    sfxAsset.data = Mix_LoadWAV((basePath / sfxAsset.location).string().append(".ogg").c_str());
    if (sfxAsset.data) {
        sfxAsset.volume = 100.0f;
        return true;
    }

    sfxAsset.data = Mix_LoadWAV((basePath / sfxAsset.location).string().append(".wav").c_str());
    if (sfxAsset.data) {
        sfxAsset.volume = 100.0f;
        return true;
    }

    return false;
}

void Shiro::SfxAssetLoader::unload(Shiro::Asset& asset) const {
    assert(asset.getType() == Shiro::AssetType::sfx);
    Shiro::SfxAsset& sfxAsset = static_cast<Shiro::SfxAsset&>(asset);

    if (sfxAsset.data) {
        Mix_FreeChunk(sfxAsset.data);
        sfxAsset.data = nullptr;
    }
}

Shiro::AssetType Shiro::SfxAssetLoader::getType() const {
    return Shiro::AssetType::sfx;
}

Shiro::SfxAsset::SfxAsset(const std::filesystem::path& location) :
    Shiro::Asset::Asset(location),
    volume(0.0f),
    data(nullptr) {}

Shiro::SfxAsset::~SfxAsset() {}

Shiro::AssetType Shiro::SfxAsset::getType() const {
    return Shiro::AssetType::sfx;
}

bool Shiro::SfxAsset::play(Shiro::Settings& settings) const {
    if (!data) {
        return false;
    }

    Mix_VolumeChunk(data, static_cast<int>(MIX_MAX_VOLUME * (volume / 100.0f) * (settings.sfxVolume / 100.0f) * (settings.masterVolume / 100.0f)));
    Mix_PlayChannel(-1, data, 0);

    return true;
}