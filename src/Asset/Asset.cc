/**
 * Copyright (c) 2020 Brandon McGriff
 *
 * Licensed under the MIT license; see the LICENSE-src file at the top level
 * directory for the full text of the license.
 */
#include "Asset/Asset.h"

Shiro::Asset::Asset(std::shared_ptr<AssetLoader> loader, const std::string& name) : name(name), loader(loader) {
    // TODO: Do something when loading fails.
    this->loader->load(*this);
}

Shiro::Asset::~Asset() {
    loader->unload(*this);
}

Shiro::AssetManager::AssetManager(const std::initializer_list<std::pair<const Shiro::AssetType, std::shared_ptr<Shiro::AssetFactory>>>& factories) : factories(factories) {}

std::size_t Shiro::AssetManager::preload(const std::initializer_list<std::pair<const Shiro::AssetType, const std::string>>& assetPairs) {
    std::size_t numPreloaded = 0u;

    for (const auto& assetPair : assetPairs) {
        std::shared_ptr<Asset> asset(factories[assetPair.first]->create(assetPair));
        if (asset) {
            assets[assetPair] = asset;
            numPreloaded++;
        }
    }

    return numPreloaded;
}

void Shiro::AssetManager::unload() {
    assets.clear();
}

std::shared_ptr<Shiro::Asset> Shiro::AssetManager::operator[](const std::pair<const Shiro::AssetType, const std::string>& assetPair) {
    if (assets.count(assetPair)) {
        return assets[assetPair];
    }
    else {
        return factories[assetPair.first]->create(assetPair);
    }
}

std::size_t Shiro::AssetManager::AssetsKeyHash::operator()(const std::pair<const Shiro::AssetType, const std::string>& key) const noexcept {
    // TODO: Consider changing how the asset type integer xor'd into the asset name's hash is calculated, such as making it fill all bits of size_t, rather than only 32 bits as it currently is here.
    return (static_cast<const std::size_t>(key.first) * 0x41c64e6du + 12345u) ^ std::hash<std::string>{}(key.second);
}