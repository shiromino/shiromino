#include "video/TextGraphic.h"
#include "SDL_stdinc.h"
#include <cassert>
#include <cstddef>
#include <sstream>
#include <unordered_map>
#include <utility>
#include "video/Render.h"

namespace Shiro {
    struct Screen;

    TextGraphic::TextGraphic(
        const Screen& screen,
        const FontAsset& font,
        const std::string& text,
        const int x,
        const int y,
        const float scale,
        const std::uint32_t color
    ) : x(x), y(y), scale(scale), color(color), screen(screen) {
        if (text == "") {
            return;
        }

        std::istringstream lines(text);
        int printY = 0;
        for (std::string line; std::getline(lines, line);) {
            if (line.size() > 0u) {
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
            }
            printY += font.bmFont.common.lineHeight;
        }
    }

    void TextGraphic::draw() const {
        for (const auto& charData : textData) {
            auto dstRect = std::get<2>(charData);
            dstRect.x += x;
            dstRect.y += y;

            Uint8 r, g, b, a;
            SDL_GetTextureColorMod(std::get<0>(charData), &r, &g, &b);
            SDL_GetTextureAlphaMod(std::get<0>(charData), &a);

            SDL_SetTextureColorMod(std::get<0>(charData), R(color), G(color), B(color));
            SDL_SetTextureAlphaMod(std::get<0>(charData), A(color));
            Shiro::RenderCopy(screen, std::get<0>(charData), &std::get<1>(charData), &dstRect);

            SDL_SetTextureColorMod(std::get<0>(charData), r, g, b);
            SDL_SetTextureAlphaMod(std::get<0>(charData), a);
        }
    }
}
