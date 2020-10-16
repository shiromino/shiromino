/**
 * Copyright (c) 2020 Brandon McGriff
 *
 * Licensed under the MIT license; see the LICENSE-src file at the top level
 * directory for the full text of the license.
 */
#pragma once
#include "Video/Gfx.h"
#include "Asset/Font.h"
#include "SDL.h"
#include <string>
#include <memory>
#include <functional>
#include <utility>
#include <cstddef>
#include <cstdint>

namespace Shiro {
    class MessageEntity : public Entity {
    public:
        DEFINE_ENTITY_PUSH(MessageEntity)

        MessageEntity() = delete;

        MessageEntity(
            const FontAsset& font,
            const std::string& text,
            //const std::shared_ptr<std::pair<int, int>> pos,
            const int& x,
            const int& y,
            const int offsetX = 0u,
            const int offsetY = 0u,
            const float scale = 1.0f,
            const std::uint32_t color = 0xFFFFFFFFu,
            const std::size_t numFrames = 1u,
            const std::size_t layerNum = GfxLayer::messages,
            const std::function<bool()> deleteCheck = []() { return false; }
        );

        bool update(Layers& layers);

    private:
        struct Impl;
        std::shared_ptr<Impl> implPtr;
    };
}