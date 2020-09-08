/**
 * Copyright (c) 2020 Brandon McGriff
 *
 * Licensed under the MIT license; see the LICENSE-src file at the top level
 * directory for the full text of the license.
 */
#pragma once
#include "Gfx/Gfx.h"
#include "gfx_structures.h"
#include "SDL.h"
#include <memory>

namespace Shiro {
    class AnimationEntity : public Entity {
    public:
        DEFINE_ENTITY_PUSH(AnimationEntity)

        AnimationEntity() = delete;

        AnimationEntity(
            SDL_Renderer* const renderer,
            gfx_image* const frames,
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