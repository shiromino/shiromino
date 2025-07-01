#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include "asset/Font.h"
#include "video/Gfx.h"

namespace Shiro {
    struct Screen;
    struct TextGraphic;

    class MessageEntity : public Entity {
    public:
        DEFINE_ENTITY_PUSH(MessageEntity)

        MessageEntity() = delete;

        MessageEntity(
            const Screen& screen,
            const FontAsset& font,
            const std::string& text,
            const int x,
            const int y,
            const float scale = 1.0f,
            const std::uint32_t color = 0xFFFFFFFFu,
            const std::size_t numFrames = 1u,
            const std::size_t layerNum = GfxLayer::messages,
            const std::function<bool()> deleteCheck = []() { return true; }
        );

        bool update(Layers& layers);

    private:
        const int x;
        const int y;
        std::size_t counter;
        const std::size_t layerNum;
        const std::function<bool()> deleteCheck;
        std::shared_ptr<TextGraphic> graphic;
    };
}
