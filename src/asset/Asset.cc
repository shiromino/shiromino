#include "asset/Asset.h"
#include <cassert>
#include <cstdint>

Shiro::AssetLoader::~AssetLoader() {}

Shiro::Asset::Asset(const std::filesystem::path& location) : location(location) {}

Shiro::Asset::~Asset() {}

Shiro::AssetManager::~AssetManager() {
    unload();
}

void Shiro::AssetManager::addLoader(const Shiro::AssetType type, std::unique_ptr<Shiro::AssetLoader> loader) {
    assert(type == loader->getType());
    loaders[type] = std::move(loader);
}

std::size_t Shiro::AssetManager::preload(
    const std::initializer_list<std::pair<Shiro::AssetType, std::filesystem::path>>& names,
    std::function<void(const std::pair<AssetType, std::filesystem::path>& name, const bool loaded)> onLoad)
{
    std::size_t numPreloaded = 0u;

    for (const auto& name : names) {
        assert(loaders.count(name.first) != 0u);
        if (assets.count(name)) {
            if (assets[name]->loaded()) {
                numPreloaded++;
            }
        }
        else {
            std::unique_ptr<Shiro::Asset> asset(loaders[name.first]->create(name.second));
            if (asset) {
                if (loaders[name.first]->load(*asset)) {
                    numPreloaded++;
                }
                assets[name] = std::move(asset);
            }
        }

        onLoad(name, assets.count(name) && assets[name]->loaded());
    }

    return numPreloaded;
}

void Shiro::AssetManager::unload() {
    for (const auto& asset : assets) {
        assert(loaders.count(asset.first.first) != 0u);
        loaders[asset.first.first]->unload(*asset.second);
    }
    assets.clear();
}

namespace std {
    constexpr bool operator==(
        const std::pair<Shiro::AssetType, std::filesystem::path>& lhs,
        const std::pair<Shiro::AssetType, std::filesystem::path>& rhs
    ) {
        return lhs.first == rhs.first && lhs.second == rhs.second;
    }
}

Shiro::Asset& Shiro::AssetManager::operator[](const std::pair<Shiro::AssetType, std::filesystem::path>& name) {
    if (!assets.count(name)) {
        assert(loaders.count(name.first) != 0u);
        auto asset = loaders[name.first]->create(name.second);
        if (asset) {
            loaders[name.first]->load(*asset);
            assets[name] = std::move(asset);
        }
    }
    assert(assets.count(name) != 0u);
    return *assets[name];
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

std::size_t Shiro::AssetManager::AssetsKeyHash::operator()(const std::pair<Shiro::AssetType, std::filesystem::path>& key) const noexcept {
    return (
        (static_cast<std::size_t>(key.first) + 1) *
        MAKE_SIZE_T_CONSTANT(0x13C3F896, 0xC70D0493) +
        MAKE_SIZE_T_CONSTANT(0x2D1BCEA4, 0xCB50BC27)
    ) ^ std::hash<std::string>{}(key.second.string());
}