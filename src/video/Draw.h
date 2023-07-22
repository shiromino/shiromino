#pragma once
#include "SDL.h"
#include "video/Screen.h"

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

    int DrawTexture(const Screen& screen, SDL_Texture *tex, const SDL_Rect *srcrect, SDL_Rect *dstrect);
    int DrawTextureOuter(const Screen& screen, SDL_Texture *tex, const SDL_Rect *srcrect, SDL_Rect *dstrect, Draw::Alignment alignment);
    int DrawRect(const Screen& screen, SDL_Rect *rect);
    int DrawRectOuter(const Screen& screen, SDL_Rect *rect, Draw::Alignment alignment);
}