#pragma once
#include "SDL_rect.h"
#include "SDL_render.h"
#include <cstdint>
#include <string>
#include <tuple>
#include <vector>
#include "asset/Font.h"
#include "video/Gfx.h"

namespace Shiro {
    struct Screen;
    
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
            const float scale = 1.0f,
            const std::uint32_t color = 0xFFFFFFFFu
        );

        void draw() const;

        int x, y;
        float scale;
        std::uint32_t color;

    private:
        const Screen& screen;
        std::vector<std::tuple<SDL_Texture*, SDL_Rect, SDL_Rect>> textData;
    };
}
