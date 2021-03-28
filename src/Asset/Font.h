/**
* Copyright (c) 2020 Brandon McGriff
*
* Licensed under the MIT license; see the LICENSE-src file at the top level
* directory for the full text of the license.
*/
#pragma once
#include "Asset/Asset.h"
#include "Video/Screen.h"
#define PDBMFONT_TEXT
#define PDBMFONT_BINARY
#define PDBMFONT_XML
#include "PDBMFont.hpp"
#include "SDL.h"
#include <vector>
namespace Shiro {
    class FontAssetLoader : public AssetLoader {
    public:
        FontAssetLoader() = delete;
        FontAssetLoader(const std::filesystem::path &assetDirectory, const Screen &screen);
        virtual std::unique_ptr<Asset> create(const std::filesystem::path &location) const;
        bool load(Asset &asset) const;
        void unload(Asset &asset) const;
        AssetType getType() const;
    private:
        const std::filesystem::path assetDirectory;
        const Screen &screen;
    };
    class FontAsset : public Asset, public AssetCommon<FontAsset, AssetType::font> {
        friend FontAssetLoader;
    public:
        ~FontAsset();
        bool loaded() const;
        AssetType getType() const;
        PDBMFont::BMFont bmFont;
        std::vector<SDL_Texture*> pages;
    protected:
        FontAsset(const std::filesystem::path &location);
    };
}