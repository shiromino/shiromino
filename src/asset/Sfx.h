#pragma once
#include <filesystem>
#include <memory>
#include "SDL_mixer.h"
#include "asset/Asset.h"

namespace Shiro {
    struct Settings;

    class SfxAssetLoader : public AssetLoader {
    public:
        SfxAssetLoader() = delete;
        SfxAssetLoader(const std::filesystem::path &assetDirectory);
        virtual std::unique_ptr<Asset> create(const std::filesystem::path &location) const;
        bool load(Asset &asset) const;
        void unload(Asset &asset) const;
        AssetType getType() const;
    private:
        const std::filesystem::path assetDirectory;
    };
    class SfxAsset : public Asset, public AssetCommon<SfxAsset, AssetType::sfx> {
        friend SfxAssetLoader;
    public:
        ~SfxAsset();
        /**
        * Plays the sound effect. Returns true if the sound effect was played;
        * playback only fails if no sound effect is loaded. The master and
        * sound effect volume settings will scale the volume the sound effect
        * plays at; if they're both 100%, SfxAsset::volume will be the played
        * volume.
        */
        bool play(const Settings &settings) const;
        bool loaded() const;
        AssetType getType() const;
        /**
        * The volume of the sound effect when played. This is a percentage,
        * with 100.0f being max volume.
        */
        float volume;
    private:
        Mix_Chunk *data;
    protected:
        SfxAsset(const std::filesystem::path &location);
    };
}