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
#include <functional>
#include <string>

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

        virtual std::unique_ptr<Asset> create(const std::filesystem::path& location) const = 0;
        virtual bool load(Asset& asset) const = 0;
        virtual void unload(Asset& asset) const = 0;
        virtual AssetType getType() const = 0;
    };

    class Asset {
        friend AssetLoader;

    public:
        virtual ~Asset();

        virtual bool loaded() const = 0;
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
        ~AssetManager();

        void addLoader(const AssetType type, std::unique_ptr<AssetLoader> loader);

        /**
         * Call with a list of asset types and paths to preload them, so the
         * assets are immediately available when requesting them later. Using
         * this isn't required, because operator[] will load assets on-demand if
         * they're not yet loaded. Returns the number of assets successfully
         * loaded; if the number returned is less than the length of the list
         * argument, then some assets weren't successfully loaded.
         */
        std::size_t preload(
            const std::initializer_list<std::pair<AssetType, std::filesystem::path>>& names,
            std::function<void(const std::pair<AssetType, std::filesystem::path>& name, const bool loaded)> onLoad =
                 [](const std::pair<AssetType, std::filesystem::path>& name, const bool loaded) {}
        );

        /**
         * Clears all assets from the assets map.
         */
        void unload();

        /**
         * Use to get an asset by type and name. The asset will be loaded when
         * requested, if it's not already loaded. If the asset couldn't be
         * loaded, an asset in an unloaded state will be returned. Asset names
         * are the filename without the extension.
         */
        Asset& operator[](const std::pair<AssetType, std::filesystem::path>& name);

    private:
        // Default to non-const for template arguments unless told otherwise. I
        // found different compilers allow it and others don't if you use const.
        // -Brandon McGriff
        std::unordered_map<AssetType, std::unique_ptr<AssetLoader>> loaders;
        struct AssetsKeyHash {
            std::size_t operator()(const std::pair<AssetType, std::filesystem::path>& key) const noexcept;
        };
        std::unordered_map<std::pair<AssetType, std::filesystem::path>, std::unique_ptr<Asset>, AssetsKeyHash> assets;
    };

    template<class AssetSubclass, AssetType type>
    class AssetCommon {
    public:
        static inline AssetSubclass& get(AssetManager& mgr, const std::filesystem::path& location) {
            return static_cast<AssetSubclass&>(mgr[{type, location}]);
        }

        static inline AssetSubclass& get(AssetManager& mgr, const std::filesystem::path& location, std::size_t i) {
            auto locationIndexed = location;
            locationIndexed.replace_filename(location.filename().string().append(std::to_string(i)));
            return static_cast<AssetSubclass&>(mgr[{type, locationIndexed}]);
        }
    };
}