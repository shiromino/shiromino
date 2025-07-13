#include "asset/Sfx.h"
#include "SDL_mixer.h"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include "Settings.h"

Shiro::SfxAssetLoader::SfxAssetLoader(const std::filesystem::path &assetDirectory) :
    assetDirectory(assetDirectory) {}
std::unique_ptr<Shiro::Asset> Shiro::SfxAssetLoader::create(const std::filesystem::path &location) const {
    return std::unique_ptr<Shiro::Asset>(new Shiro::SfxAsset(location));
}
bool Shiro::SfxAssetLoader::load(Shiro::Asset &asset) const {
    assert(asset.getType() == Shiro::AssetType::sfx);
    Shiro::SfxAsset &sfxAsset = static_cast<Shiro::SfxAsset&>(asset);
    sfxAsset.data = Mix_LoadWAV((assetDirectory / sfxAsset.location).concat(".ogg").string().c_str());
    if (sfxAsset.data) {
        sfxAsset.volume = 100.0f;
        return true;
    }
    sfxAsset.data = Mix_LoadWAV((assetDirectory / sfxAsset.location).concat(".wav").string().c_str());
    if (sfxAsset.data) {
        sfxAsset.volume = 100.0f;
        return true;
    }
    std::cerr << "Failed loading sound effect \"" << sfxAsset.location.string() << "\"" << std::endl;
    sfxAsset.volume = 0.0f;
    return false;
}
void Shiro::SfxAssetLoader::unload(Shiro::Asset &asset) const {
    assert(asset.getType() == Shiro::AssetType::sfx);
    Shiro::SfxAsset &sfxAsset = static_cast<Shiro::SfxAsset&>(asset);
    if (sfxAsset.loaded()) {
        Mix_FreeChunk(sfxAsset.data);
        sfxAsset.data = nullptr;
    }
}
Shiro::AssetType Shiro::SfxAssetLoader::getType() const {
    return Shiro::AssetType::sfx;
}
Shiro::SfxAsset::SfxAsset(const std::filesystem::path &location) :
    Shiro::Asset::Asset(location),
    volume(0.0f),
    data(nullptr) {}
Shiro::SfxAsset::~SfxAsset() {}
bool Shiro::SfxAsset::loaded() const {
    return data != nullptr;
}
Shiro::AssetType Shiro::SfxAsset::getType() const {
    return Shiro::AssetType::sfx;
}
bool Shiro::SfxAsset::play(const Shiro::Settings &settings) const {
    if (!loaded()) {
        return false;
    }
    Mix_VolumeChunk(data, std::clamp<int>(MIX_MAX_VOLUME * (volume / 100.0f) * (settings.sfxVolume / 100.0f) * (settings.masterVolume / 100.0f), 0, MIX_MAX_VOLUME));
    Mix_PlayChannel(-1, data, 0);
    return true;
}