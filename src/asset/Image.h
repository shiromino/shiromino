/**
* Copyright (c) 2020 Brandon McGriff
*
* Licensed under the MIT license; see the LICENSE-src file at the top level
* directory for the full text of the license.
*/
#pragma once
#include "asset/Asset.h"
#include "video/Screen.h"
#include <filesystem>
#include <memory>
namespace Shiro {
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