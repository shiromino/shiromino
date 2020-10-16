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
#include <vector>
#include <utility>
#include <string>
#include <cstddef>
#include <cstdint>

namespace Shiro {
    // TODO: Investigate whether more features should be added, after considering whether previous output features should be changed to more premade fonts, but certainly alignment should be added.
    struct TextGraphic : public Graphic {
    public:
        TextGraphic() = delete;

        TextGraphic(
            const FontAsset& font,
            const std::string& text,
            const int x,
            const int y,
            const int offsetX = 0,
            const int offsetY = 0,
            const float scale = 1.0f,
            const std::uint32_t color = 0xFFFFFFFFu
        );

        void draw(const Screen& screen) const;

        int x, y;
        int offsetX, offsetY;
        float scale;
        std::uint32_t color;

    private:
        std::vector<std::tuple<SDL_Texture*, SDL_Rect, SDL_Rect>> textData;
    };
}