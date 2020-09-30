/**
 * Copyright (c) 2020 Brandon McGriff
 *
 * Licensed under the MIT license; see the LICENSE-src file at the top level
 * directory for the full text of the license.
 */
#include "Asset/Asset.h"
#include <cassert>
#include <cstdint>

Shiro::AssetLoader::~AssetLoader() {}

Shiro::Asset::Asset(const std::filesystem::path& location) : location(location) {}

Shiro::Asset::~Asset() {}

Shiro::AssetManager::AssetManager(const std::initializer_list<std::pair<const Shiro::AssetType, Shiro::AssetLoader*>>& loaders) : loaders(loaders) {}

Shiro::AssetManager::~AssetManager() {
    unload();
    for (const auto loader : loaders) {
        delete loader.second;
    }
}

std::size_t Shiro::AssetManager::preload(const std::initializer_list<std::pair<const Shiro::AssetType, const std::filesystem::path>>& assetPairs) {
    std::size_t numPreloaded = 0u;

    for (const auto& assetPair : assetPairs) {
        Asset* asset(loaders[assetPair.first]->create(assetPair.second));
        if (asset) {
            loaders[assetPair.first]->load(*asset);
            assets[assetPair] = asset;
            numPreloaded++;
        }
    }

    return numPreloaded;
}

void Shiro::AssetManager::unload() {
    for (const auto& assetPair : assets) {
        loaders[assetPair.first.first]->unload(*assets[assetPair.first]);
        delete assets[assetPair.first];
    }
    assets.clear();
}

namespace std {
    constexpr bool operator==(
        const std::pair<const Shiro::AssetType, const std::filesystem::path>& lhs,
        const std::pair<const Shiro::AssetType, const std::filesystem::path>& rhs
    ) {
        return lhs.first == rhs.first && lhs.second == rhs.second;
    }
}

Shiro::Asset& Shiro::AssetManager::operator[](const std::pair<const Shiro::AssetType, const std::filesystem::path>& assetPair) {
    if (!assets.count(assetPair)) {
        assets[assetPair] = loaders[assetPair.first]->create(assetPair.second);
        loaders[assetPair.first]->load(*assets[assetPair]);
    }
    assert(assets[assetPair] != nullptr);
    return *assets[assetPair];
}

#ifdef UINT64_C
#define MAKE_SIZE_T_CONSTANT(upper, lower) \
    ((sizeof(std::size_t) >= 8) ? \
        static_cast<std::size_t>((UINT64_C(upper) << 32) | UINT64_C(lower)) : \
        static_cast<std::size_t>(lower) \
    )
#else
#define MAKE_SIZE_T_CONSTANT(upper, lower) \
    (static_cast<std::size_t>(lower ## u))
#endif

std::size_t Shiro::AssetManager::AssetsKeyHash::operator()(const std::pair<const Shiro::AssetType, const std::filesystem::path>& key) const noexcept {
    return (
        (static_cast<const std::size_t>(key.first) + 1) *
        MAKE_SIZE_T_CONSTANT(0x13C3F896, 0xC70D0493) +
        MAKE_SIZE_T_CONSTANT(0x2D1BCEA4, 0xCB50BC27)
    ) ^ std::hash<std::string>{}(key.second.string());
}