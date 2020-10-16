/**
 * Copyright (c) 2020 Brandon McGriff and Felicity Violette
 *
 * Licensed under the MIT license; see the LICENSE-src file at the top level
 * directory for the full text of the license.
 */
#include "Video/TextGraphic.h"
#include <sstream>
#include <cassert>

namespace Shiro {
    TextGraphic::TextGraphic(
        const FontAsset& font,
        const std::string& text,
        const int x,
        const int y,
        const int offsetX,
        const int offsetY,
        const float scale,
        const std::uint32_t color
    ) : x(x), y(y), offsetX(offsetX), offsetY(offsetY), scale(scale), color(color) {
        if (text == "") {
            return;
        }

        std::istringstream lines(text);
        int printY = 0;
        for (std::string line; std::getline(lines, line);) {
            assert(font.bmFont.chars.count(line[0]));
            int printX = -font.bmFont.chars.at(line[0]).xoffset;
            for (std::size_t i = 0u; i < line.size(); i++) {
                assert(font.bmFont.chars.count(line[i]));
                const PDBMFont::BMFont::Char& ch = font.bmFont.chars.at(line[i]);

                textData.push_back({
                    font.pages[ch.page],
                    {
                        int(ch.x), int(ch.y),
                        int(ch.width), int(ch.height)
                    },
                    {
                        int(scale * (printX + ch.xoffset)), int(scale * (printY + ch.yoffset)),
                        int(scale * ch.width), int(scale * ch.height)
                    }
                });

                printX += int(ch.xadvance);
                if (line[i + 1] != '\0' && font.bmFont.kernings.count({line[i], line[i + 1]})) {
                    printX += font.bmFont.kernings.at({line[i], line[i + 1]});
                }
            }
            printY += font.bmFont.common.lineHeight;
        }
    }

    void TextGraphic::draw(const Screen& screen) const {
        const int dstOffsetX = x + offsetX;
        const int dstOffsetY = y + offsetY;
        for (const auto& charData : textData) {
            auto dstRect = std::get<2>(charData);
            dstRect.x += dstOffsetX;
            dstRect.y += dstOffsetY;

            Uint8 r, g, b, a;
            SDL_GetTextureColorMod(std::get<0>(charData), &r, &g, &b);
            SDL_GetTextureAlphaMod(std::get<0>(charData), &a);

            SDL_SetTextureColorMod(std::get<0>(charData), R(color), G(color), B(color));
            SDL_SetTextureAlphaMod(std::get<0>(charData), A(color));
            SDL_RenderCopy(screen.renderer, std::get<0>(charData), &std::get<1>(charData), &dstRect);

            SDL_SetTextureColorMod(std::get<0>(charData), r, g, b);
            SDL_SetTextureAlphaMod(std::get<0>(charData), a);
        }
    }
}