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

namespace Shiro {
    struct TextGraphic : public Graphic {
        TextGraphic() = delete;

        TextGraphic(
            const std::string& text,
            const size_t pos,
            const size_t len,
            const int x,
            const int y,
            const png_monofont& font,
            const text_formatting& fmt
        );

        TextGraphic(
            const std::string& text,
            const int x,
            const int y,
            const png_monofont& font,
            const text_formatting& fmt
        );

        void draw(const Screen& screen) const;

        const std::string text;
        const size_t pos;
        const size_t len;
        const int x;
        const int y;
        const png_monofont& font;
        const text_formatting fmt;
    };
}