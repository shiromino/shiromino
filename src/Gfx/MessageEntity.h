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
#include <string>
#include <memory>
#include <functional>

namespace Shiro {
    class MessageEntity : public Entity {
    public:
        DEFINE_ENTITY_PUSH(MessageEntity)

        MessageEntity() = delete;

        MessageEntity(
            const std::string text,
            const size_t layerNum,
            const int x,
            const int y,
            const png_monofont& font,
            const text_formatting& fmt,
            const size_t numFrames = 0u,
            const std::function<bool()> deleteCheck = []() { return false; }
        );

        bool update(Layers& layers);

    private:
        struct Impl;
        std::shared_ptr<Impl> implPtr;
    };
}