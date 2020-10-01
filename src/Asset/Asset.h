/**
 * Copyright (c) 2020 Brandon McGriff
 *
 * Licensed under the MIT license; see the LICENSE-src file at the top level
 * directory for the full text of the license.
 */
#pragma once
#include <filesystem>
#include <utility>
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
        virtual ~AssetLoader();

        virtual Asset* create(const std::filesystem::path& location) const = 0;
        virtual bool load(Asset& asset) const = 0;
        virtual void unload(Asset& asset) const = 0;
        virtual AssetType getType() const = 0;
    };

    class Asset {
        friend AssetLoader;

    public:
        virtual ~Asset();

        virtual AssetType getType() const = 0;

        const std::filesystem::path location;

    protected:
        /**
         * The `location` argument must be a relative path, so it can be
         * properly appended to a base path used by the asset's loader.
         */
        Asset(const std::filesystem::path& location);
    };

    class AssetManager {
    public:
        AssetManager() = delete;

        AssetManager(const std::initializer_list<std::pair<const Shiro::AssetType, Shiro::AssetLoader*>>& loaders);
        ~AssetManager();

        /**
         * Call with a list of asset types and names to preload them, so the
         * assets are immediately available when requesting them later. Using
         * this isn't required, because operator[] will load assets on-demand if
         * they're not yet loaded. Returns the number of assets successfully
         * loaded; if the number returned is less than the length of the list
         * argument, then some assets weren't successfully loaded.
         * TODO: Come up with a way to provide the preload status to the user of an AssetManager. Might require completely changing how preloading is done, such as no longer having this method.
         */
        std::size_t preload(const std::initializer_list<std::pair<const AssetType, const std::filesystem::path>>& assetPairs);

        /**
         * Clears all assets from the assets map. If shared pointers of loaded
         * assets are still held outside the manager, they won't be unloaded
         * until all references to those assets are destructed, otherwise all
         * the assets will be unloaded.
         */
        void unload();

        /**
         * Use to get an asset by type and name. The asset will be loaded when
         * requested, if it's not already loaded. If the asset couldn't be made
         * available for some reason, doesn't load anything, and returns an
         * empty shared_ptr. Asset names are the filename without the extension.
         */
        Asset& operator[](const std::pair<const AssetType, const std::filesystem::path>& assetPair);

    private:
        // I found out that if the key type is const for unordered_map, Visual
        // Studio generates some weird errors, despite it working fine with GCC
        // and Clang. So just keep key types for unordered_map not const.
        // -Brandon McGriff
        std::unordered_map<AssetType, AssetLoader*> loaders;
        struct AssetsKeyHash {
            std::size_t operator()(const std::pair<const AssetType, const std::filesystem::path>& key) const noexcept;
        };
        std::unordered_map<std::pair<const AssetType, const std::filesystem::path>, Asset*, AssetsKeyHash> assets;
    };
}