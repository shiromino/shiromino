/**
 * Copyright (c) 2020 Brandon McGriff and Felicity Violette
 *
 * Licensed under the MIT license; see the LICENSE-src file at the top level
 * directory for the full text of the license.
 */
#include "Video/TextGraphic.h"
#include "gfx_old.h"
#include "stringtools.h"

using namespace Shiro;
using namespace std;

TextGraphic::TextGraphic(
    const std::string& text,
    const size_t pos,
    const size_t len,
    const int x,
    const int y,
    const png_monofont& font,
    const text_formatting& fmt
) :
    text(text),
    pos(pos),
    len(len),
    x(x),
    y(y),
    font(font),
    fmt(fmt) {}

TextGraphic::TextGraphic(
    const std::string& text,
    const int x,
    const int y,
    const png_monofont& font,
    const text_formatting& fmt
) :
    TextGraphic(
        text,
        0,
        text.size(),
        x,
        y,
        font,
        fmt
    ) {}

void TextGraphic::draw(const Screen& screen) const {
    if (text == "") {
        return;
    }

    SDL_SetTextureColorMod(font.sheet, R(fmt.rgba), G(fmt.rgba), B(fmt.rgba));
    SDL_SetTextureAlphaMod(font.sheet, A(fmt.rgba));

    if (font.outline_sheet) {
        SDL_SetTextureColorMod(font.outline_sheet, R(fmt.outline_rgba), G(fmt.outline_rgba), B(fmt.outline_rgba));
        SDL_SetTextureAlphaMod(font.outline_sheet, A(fmt.outline_rgba));
    }

    SDL_Rect src = { 0, 0, (int)font.char_w, (int)font.char_h };
    SDL_Rect dest = {
        x,
        y,
        (int)(fmt.size_multiplier * (float)font.char_w),
        (int)(fmt.size_multiplier * (float)font.char_h)
    };

    vector<string> lines = strtools::split(text, '\n');

    bool usingTargetTex = false;

    if (SDL_GetRenderTarget(screen.renderer) != NULL) {
        usingTargetTex = true;
    }

    switch (fmt.align) {
    case ALIGN_LEFT:
        dest.x = x;
        break;

    case ALIGN_RIGHT:
        dest.x = x - (fmt.size_multiplier * (float)font.char_w) * lines[0].size();
        break;

    case ALIGN_CENTER:
        if (fmt.wrap_length < lines[0].size()) {
            dest.x = x;
        }
        else {
            dest.x = x + (fmt.size_multiplier * (float)font.char_w / 2.0f) * (fmt.wrap_length - lines[0].size());
        }
        break;

    default:
        break;
    }

    const size_t end = pos + len;
    for (size_t i = pos, linefeeds = 0, lastWrapLinePos = 0, lastWrapPos = 0; i < text.size() && i < end; i++) {
        if ((fmt.wrap_length && i > 0 && (i - lastWrapPos) % fmt.wrap_length == 0) || text[i] == '\n') {
            if (text[i] == '\n') {
                linefeeds++;
                lastWrapLinePos = i - lastWrapPos + lastWrapLinePos;
                lastWrapPos = i;
            }
            else if (i != 0 && i % fmt.wrap_length == 0) {
                lastWrapLinePos = i - lastWrapPos + lastWrapLinePos;
                lastWrapPos = i;
            }

            dest.y += fmt.line_spacing * fmt.size_multiplier * (float)font.char_h;

            switch (fmt.align) {
            default:
            case ALIGN_LEFT:
                dest.x = x;
                break;

            case ALIGN_RIGHT:
                dest.x = x - (font.char_w) * lines[linefeeds].size();
                break;

            case ALIGN_CENTER:
                if(fmt.wrap_length < lines[linefeeds].size() - lastWrapLinePos)
                    dest.x = x;
                else
                    dest.x = (int)(x + (font.char_w / 2) * (fmt.wrap_length - (lines[linefeeds].size() - lastWrapLinePos)));

                break;
            }

            if(text[i] == '\n')
                continue;
        }

        // we draw a square behind each character if we have no outlines to use
        if (fmt.outlined && !font.outline_sheet) {
            src.x = 31 * font.char_w;
            src.y = 3 * font.char_h;
            SDL_SetTextureColorMod(font.sheet, R(fmt.outline_rgba), G(fmt.outline_rgba), B(fmt.outline_rgba));
            SDL_SetTextureAlphaMod(font.sheet, A(fmt.outline_rgba));

            if (usingTargetTex) {
                SDL_SetRenderDrawColor(screen.renderer, 0, 0, 0, 0);
                SDL_RenderFillRect(screen.renderer, &dest);
                SDL_RenderCopy(screen.renderer, font.sheet, &src, &dest);
            }
            else {
                SDL_RenderCopy(screen.renderer, font.sheet, &src, &dest);
            }

            SDL_SetTextureColorMod(font.sheet, R(fmt.rgba), G(fmt.rgba), B(fmt.rgba));
            SDL_SetTextureAlphaMod(font.sheet, A(fmt.rgba));
        }

        src.x = font.char_w * (text[i] % 32);
        src.y = font.char_h * ((int)(text[i] / 32) - 1);
        if (src.y < 0) {
            src.x = 31 * font.char_w;
            src.y = 2 * font.char_h;
        }

        if (fmt.shadow && !usingTargetTex) {
            dest.x -= 2.0 * fmt.size_multiplier;
            dest.y += 2.0 * fmt.size_multiplier;

            SDL_SetTextureAlphaMod(font.sheet, A(fmt.rgba) / 4);
            if (font.outline_sheet) {
                SDL_SetTextureAlphaMod(font.outline_sheet, A(fmt.rgba) / 4);
            }

            SDL_RenderCopy(screen.renderer, font.sheet, &src, &dest);

            if (fmt.outlined && font.outline_sheet) {
                SDL_RenderCopy(screen.renderer, font.outline_sheet, &src, &dest);
            }

            dest.x += 2.0 * fmt.size_multiplier;
            dest.y -= 2.0 * fmt.size_multiplier;

            SDL_SetTextureAlphaMod(font.sheet, A(fmt.rgba));
            if (font.outline_sheet) {
                SDL_SetTextureAlphaMod(font.outline_sheet, A(fmt.rgba));
            }
        }

        if (usingTargetTex) {
            SDL_SetRenderDrawColor(screen.renderer, 0, 0, 0, 0);
            SDL_RenderFillRect(screen.renderer, &dest);
            SDL_RenderCopy(screen.renderer, font.sheet, &src, &dest);

            if (fmt.outlined && font.outline_sheet) {
                SDL_RenderCopy(screen.renderer, font.outline_sheet, &src, &dest);
            }
        }
        else {
            SDL_RenderCopy(screen.renderer, font.sheet, &src, &dest);

            if (fmt.outlined && font.outline_sheet) {
                SDL_RenderCopy(screen.renderer, font.outline_sheet, &src, &dest);
            }
        }

        dest.x += fmt.size_multiplier * (float)font.char_w;
    }

    SDL_SetTextureColorMod(font.sheet, 255, 255, 255);
    SDL_SetTextureAlphaMod(font.sheet, 255);

    if (font.outline_sheet) {
        SDL_SetTextureColorMod(font.outline_sheet, 255, 255, 255);
        SDL_SetTextureAlphaMod(font.outline_sheet, 255);
    }
}