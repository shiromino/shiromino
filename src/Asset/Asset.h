/**
 * Copyright (c) 2020 Brandon McGriff
 *
 * Licensed under the MIT license; see the LICENSE-src file at the top level
 * directory for the full text of the license.
 */
#pragma once
#include <utility>
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

        virtual ~AssetLoader();

        virtual bool load(Asset& asset) const = 0;
        virtual void unload(Asset& asset) const = 0;
        virtual AssetType getType() const = 0;
    };

    class Asset {
    public:
        Asset() = delete;

        /**
         * Don't add any other constructors to subclasses, just use this one. If
         * an Asset subclass needs to do some more elaborate loading, like
         * loading multiple files for its associated Asset subclass, then the
         * loader can manage its own Asset instances for the files it loads for
         * the asset.
         */
        Asset(std::shared_ptr<AssetLoader> loader, const std::string& name);
        virtual ~Asset();

        virtual AssetType getType() const = 0;

        const std::string name;

    private:
        const std::shared_ptr<AssetLoader> loader;
    };

    /**
     * AssetFactory subclasses are considered "platform specific", so they can
     * define whatever constructors they want, with platform-specific code
     * handling how they're created. But each platform should keep the same name
     * for the same type of factory, just having platform-specific details
     * internally, so the platform-independent code can keep using assets with
     * the same code for all platforms.
     */
    class AssetFactory {
    public:
        AssetFactory() = delete;

        virtual ~AssetFactory();

        /**
         * Creates assets. This method handles providing an AssetLoader shared
         * pointer to the Asset; it's impossible for an AssetLoader instance to
         * provide its shared pointer to an Asset, so a factory class has to do
         * it. An AssetFactory has freedom with how an AssetLoader is provided
         * to Assets; it can construct a new AssetLoader for each asset
         * instance, or hold a shared pointer to an AssetLoader that's shared
         * with all assets it creates.
         */
        virtual std::shared_ptr<Asset> create(const std::pair<const AssetType, const std::string>& assetPair) const = 0;
    };

    class AssetManager {
    public:
        AssetManager() = delete;

        AssetManager(const std::initializer_list<std::pair<const Shiro::AssetType, std::shared_ptr<Shiro::AssetFactory>>>& factories);

        /**
         * Call with a list of asset types and names to preload them, so the
         * assets are immediately available when requesting them later. Using
         * this isn't required, because operator[] will load assets on-demand if
         * they're not yet loaded. Returns the number of assets successfully
         * loaded; if the number returned is less than the length of the list
         * argument, then some assets weren't successfully loaded.
         * TODO: Come up with a way to provide the preload status to the user of an AssetManager. Might require completely changing how preloading is done, such as no longer having this method.
         */
        std::size_t preload(const std::initializer_list<std::pair<const AssetType, const std::string>>& assetPairs);

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
        std::shared_ptr<Asset> operator[](const std::pair<const AssetType, const std::string>& assetPair);

    private:
        std::unordered_map<const AssetType, std::shared_ptr<AssetFactory>> factories;
        struct AssetsKeyHash {
            std::size_t operator()(const std::pair<const AssetType, const std::string>& key) const noexcept;
        };
        std::unordered_map<std::pair<const AssetType, const std::string>, std::shared_ptr<Asset>, AssetsKeyHash> assets;
    };
}