/**
 * Copyright (c) 2020 Brandon McGriff
 *
 * Licensed under the MIT license; see the LICENSE-src file at the top level
 * directory for the full text of the license.
 */
#pragma once
#include "Video/Screen.h"
#include "Asset/Asset.h"
#include "Video/Gfx.h"
#include "SDL.h"
#include <filesystem>
#include <memory>

namespace Shiro {
    struct AnimationGraphic : public Graphic {
        AnimationGraphic() = delete;

        AnimationGraphic(
            const Screen& screen,
            SDL_Texture* const frame,
            const int x,
            const int y,
            const Uint32 rgbaMod
        );

        void draw() const;

        const Screen& screen;
        SDL_Texture* frame;
        const int x;
        const int y;
        const Uint32 rgbaMod;
    };

    class AnimationEntity : public Entity {
    public:
        DEFINE_ENTITY_PUSH(AnimationEntity)

        AnimationEntity() = delete;

        AnimationEntity(
            const Screen& screen,
            AssetManager& assetMgr,
            const std::filesystem::path& frames,
            const std::size_t layerNum,
            const int x,
            const int y,
            const std::size_t numFrames,
            const std::size_t frameMultiplier,
            const Uint32 rgbaMod
        );

        bool update(Layers& layers);

    private:
        AssetManager& assetMgr;
        const std::filesystem::path frames;
        const std::size_t layerNum;
        std::size_t counter;
        const std::size_t numFrames;
        const std::size_t frameMultiplier;
        const std::shared_ptr<AnimationGraphic> graphic;
    };
}
