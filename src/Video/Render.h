#pragma once
#include "SDL.h"
#include "Video/Screen.h"

using namespace Shiro;

namespace Shiro {
    int RenderCopy(const Screen& screen, SDL_Texture *tex, const SDL_Rect *srcrect, SDL_Rect *dstrect);
    int RenderFillRect(const Screen& screen, SDL_Rect *rect);
}