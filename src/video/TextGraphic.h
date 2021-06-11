#pragma once
#include "asset/Font.h"
#include "video/Screen.h"
#include "video/Gfx.h"
#include "SDL.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace Shiro {
    // TODO: Investigate whether more features should be added, after considering whether previous output features should be changed to more premade fonts, but certainly alignment should be added.
    struct TextGraphic : public Graphic {
    public:
        TextGraphic() = delete;

        TextGraphic(
            const Screen& screen,
            const FontAsset& font,
            const std::string& text,
            const int x,
            const int y,
            const int offsetX = 0,
            const int offsetY = 0,
            const float scale = 1.0f,
            const std::uint32_t color = 0xFFFFFFFFu
        );

        void draw() const;

        int x, y;
        int offsetX, offsetY;
        float scale;
        std::uint32_t color;

    private:
        const Screen& screen;
        std::vector<std::tuple<SDL_Texture*, SDL_Rect, SDL_Rect>> textData;
    };
}
