/**
 * Copyright (c) 2020 Brandon McGriff
 *
 * Licensed under the MIT license; see the LICENSE-src file at the top level
 * directory for the full text of the license.
 */
#pragma once
#include "Asset/Asset.h"
#include "Video/Gfx.h"
#include "SDL.h"
#include <filesystem>
#include <memory>

namespace Shiro {
    class AnimationEntity : public Entity {
    public:
        DEFINE_ENTITY_PUSH(AnimationEntity)

        AnimationEntity() = delete;

        AnimationEntity(
            AssetManager& mgr,
            const std::filesystem::path& frames,
            const size_t layerNum,
            const int x,
            const int y,
            const size_t numFrames,
            const size_t frameMultiplier,
            const Uint32 rgbaMod
        );

        bool update(Layers& layers);

    private:
        struct Impl;
        std::shared_ptr<Impl> implPtr;
    };
}