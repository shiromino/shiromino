#pragma once
#include "SDL.h"
#include "video/Screen.h"
#include <cstddef>

using namespace Shiro;

namespace Shiro {
    namespace Draw {
        enum class Alignment {
            topLeft,
            bottomLeft,
            topRight,
            bottomRight
        };
    }

    // wrapper for SDL_RenderCopy -- passes through rendering logic which handles resolution scaling to current window size
    int DrawTexture(const Screen& screen, SDL_Texture *tex, const SDL_Rect *srcrect, SDL_Rect *dstrect);
    //int DrawTexture(const Screen& screen, SDL_Texture *tex, const SDL_Rect *srcrect, SDL_FRect *dstrect);
    //int DrawTexture(const Screen& screen, SDL_Texture *tex, const SDL_Rect *srcrect, std::nullptr_t);

    int DrawTextureOuter(const Screen& screen, SDL_Texture *tex, const SDL_Rect *srcrect, SDL_Rect *dstrect, Draw::Alignment alignment);
    int DrawRect(const Screen& screen, SDL_Rect *rect);
    int DrawRectOuter(const Screen& screen, SDL_Rect *rect, Draw::Alignment alignment);
}