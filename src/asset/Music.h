#pragma once
#include <filesystem>
#include <memory>
#include "SDL_mixer.h"
#include "asset/Asset.h"

namespace Shiro {
    struct Settings;

    // TODO: Allow selecting playback channel?
    class MusicAssetLoader : public AssetLoader {
    public:
        MusicAssetLoader() = delete;
        MusicAssetLoader(const std::filesystem::path &path);
        virtual std::unique_ptr<Asset> create(const std::filesystem::path &location) const;
        bool load(Asset &asset) const;
        void unload(Asset &asset) const;
        AssetType getType() const;
    private:
        const std::filesystem::path assetDirectory;
    };
    class MusicAsset : public Asset, public AssetCommon<MusicAsset, AssetType::music> {
        friend MusicAssetLoader;
    public:
        ~MusicAsset();
        /**
        * Plays the music track. Returns true if the music track was played;
        * playback only fails if no music track is loaded. The master and
        * music volume settings will scale the volume the music track plays
        * at; if they're both 100%, Music::volume will be the played volume.
        */
        bool play(const Settings &settings) const;
        bool loaded() const;
        AssetType getType() const;
        /**
        * The volume of the music track when played. This is a percentage,
        * with 100.0f being max volume.
        */
        float volume;
    private:
        Mix_Music *data;
    protected:
        MusicAsset(const std::filesystem::path &location);
    };
}