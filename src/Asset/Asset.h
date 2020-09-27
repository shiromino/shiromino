/**
 * Copyright (c) 2020 Brandon McGriff
 *
 * Licensed under the MIT license; see the LICENSE-src file at the top level
 * directory for the full text of the license.
 */
#pragma once
#include "Video/Screen.h"
#include <utility>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <memory>
#include <initializer_list>

namespace Shiro {
    enum class AssetType {
        image,
        font,   // TODO: Change fonts to be AngelCode bitmap fonts, managing all their own files (font definition, images), replacing the current use of SGUIL for fonts.
        music,
        sfx
    };

    class Asset;
    class AssetLoader {
    public:
        AssetLoader() = delete;

        AssetLoader(const std::filesystem::path& basePath, const AssetType type);
        virtual ~AssetLoader();

        virtual bool load(Asset& asset);
        virtual void unload(Asset& asset);
        virtual AssetType getType() const;

    private:
        const std::filesystem::path basePath;
        const AssetType type;
    };

    class Asset {
        friend AssetLoader;

    public:
        Asset() = delete;

        Asset(const std::shared_ptr<AssetLoader> loader, const std::string& name);
        virtual ~Asset();

        virtual AssetType getType() const;

    private:
        const std::shared_ptr<AssetLoader> loader;
        const std::string name;
        bool loaded;
    };

    class AssetManager {
    public:
        AssetManager() = delete;

        AssetManager(const std::initializer_list<std::pair<AssetType, std::shared_ptr<AssetLoader>>>& loaders);

        /**
         * Call with a list of asset types and names to preload them, so the
         * assets are immediately available when requesting them later. Using
         * this isn't required, because the get() function will load assets
         * on-demand if they're not yet loaded. Returns the number of assets
         * successfully loaded; if the number returned is less than the length
         * of the list argument, then some assets weren't successfully loaded.
         * TODO: Come up with a way to provide the preload status to the user of an AssetManager. Might require completely changing how preloading is done, such as no longer having this method.
         */
        std::size_t preload(const std::initializer_list<std::pair<AssetType, std::string>>& assetPairs);

        /**
         * Unloads all currently loaded assets.
         */
        void unload();

        /**
         * Use to get an asset by type and name. The asset will be loaded when
         * requested, if it's not already loaded. If the asset couldn't be made
         * available for some reason, doesn't load anything, and returns an
         * empty shared_ptr. Asset names are the filename without the extension.
         * Will return an empty shared_ptr if there's multiple files with the
         * same name but different supported extensions on disk.
         */
        std::shared_ptr<Asset> operator[](const std::pair<AssetType, std::string>& assetPair);

    private:
        const std::unordered_map<AssetType, std::shared_ptr<AssetLoader>> loaders;
        // TODO: Define hash code function for this map's key type.
        std::unordered_map<std::pair<AssetType, std::string>, std::shared_ptr<Asset>> assets;
    };
}