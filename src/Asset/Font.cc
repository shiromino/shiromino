/**
 * Copyright (c) 2020 Brandon McGriff
 *
 * Licensed under the MIT license; see the LICENSE-src file at the top level
 * directory for the full text of the license.
 */
#include "Asset/Font.h"
#define PDBMFONT_DEFINE
#define PDBMFONT_TEXT
#define PDBMFONT_BINARY
#define PDBMFONT_XML
#include "PDBMFont.hpp"
#include "SDL_image.h"
#include <cassert>

namespace Shiro {
    FontAssetLoader::FontAssetLoader(const std::filesystem::path& basePath, const Screen& screen) :
        basePath(basePath),
        screen(screen) {}

    std::unique_ptr<Asset> FontAssetLoader::create(const std::filesystem::path& location) const {
        return std::unique_ptr<Shiro::Asset>(new FontAsset(location));
    }

    bool FontAssetLoader::load(Asset& asset) const {
        assert(asset.getType() == AssetType::font);
        FontAsset& fontAsset = static_cast<FontAsset&>(asset);

        auto fontLocation = (basePath / fontAsset.location).replace_extension(".fnt");
        if (!fontAsset.bmFont.read(fontLocation.string())) {
            fontAsset.bmFont = PDBMFont::BMFont();
            std::cerr << "Failed loading font \"" << fontAsset.location.string() << "\"" << std::endl;
            return false;
        }

        fontLocation.remove_filename();
        bool loadSuccess = true;
        fontAsset.pages.resize(fontAsset.bmFont.pages.size(), nullptr);
        for (std::size_t i = 0u; i < fontAsset.bmFont.pages.size(); i++) {
            SDL_Surface* surface = IMG_Load((fontLocation / fontAsset.bmFont.pages[i]).string().c_str());
            if (!surface) {
                loadSuccess = false;
                std::cerr << "Failed loading font page image \"" << fontAsset.bmFont.pages[i] << "\"" << std::endl;
                break;
            }
            fontAsset.pages[i] = SDL_CreateTextureFromSurface(screen.renderer, surface);
            SDL_FreeSurface(surface);
            if (!fontAsset.pages[i]) {
                fontAsset.bmFont = PDBMFont::BMFont();
                for (const auto page : fontAsset.pages) {
                    if (page) {
                        SDL_DestroyTexture(page);
                    }
                }
                fontAsset.pages.clear();
                loadSuccess = false;
                std::cerr << "Failed creating texture for font page image \"" << fontAsset.bmFont.pages[i] << "\"" << std::endl;
                break;
            }
        }

        return loadSuccess;
    }

    void FontAssetLoader::unload(Asset& asset) const {
        assert(asset.getType() == AssetType::font);
        FontAsset& fontAsset = static_cast<FontAsset&>(asset);

        if (fontAsset.loaded()) {
            fontAsset.bmFont = PDBMFont::BMFont();
            for (const auto page : fontAsset.pages) {
                SDL_DestroyTexture(page);
            }
            fontAsset.pages.clear();
        }
    }

    AssetType FontAssetLoader::getType() const {
        return AssetType::font;
    }

    FontAsset::FontAsset(const std::filesystem::path& location) : Asset(location) {}

    FontAsset::~FontAsset() {}

    bool FontAsset::loaded() const {
        return pages.size() > 0u;
    }

    AssetType FontAsset::getType() const {
        return AssetType::font;
    }
}