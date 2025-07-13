#pragma once
#include "SDL_render.h"
#include <filesystem>
#include <memory>
#include "asset/Asset.h"

namespace Shiro {
    struct Screen;

    class ImageAssetLoader : public AssetLoader {
    public:
        ImageAssetLoader() = delete;
        ImageAssetLoader(const std::filesystem::path &assetDirectory, const Screen &screen);
        virtual std::unique_ptr<Asset> create(const std::filesystem::path &location) const;
        bool load(Asset &asset) const;
        void unload(Asset &asset) const;
        AssetType getType() const;
    private:
        const std::filesystem::path assetDirectory;
        const Screen &screen;
    };
    class ImageAsset : public Asset, public AssetCommon<ImageAsset, AssetType::image> {
        friend ImageAssetLoader;
    public:
        ~ImageAsset();
        bool loaded() const;
        AssetType getType() const;
        SDL_Texture *getTexture() const;
    private:
        SDL_Texture *texture;
    protected:
        ImageAsset(const std::filesystem::path &location);
    };
}