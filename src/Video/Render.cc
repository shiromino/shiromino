#include "Video/Render.h"
#include "Video/Screen.h"

using namespace Shiro;

namespace Shiro {
    int RenderCopy(const Screen& screen, SDL_Texture *tex, const SDL_Rect *srcrect, SDL_Rect *dstrect)
    {
        if(screen.window == nullptr || screen.renderer == nullptr || tex == nullptr)
        {
            return -1;
        }

        float scaleX;
        float scaleY;
        SDL_RenderGetScale(screen.renderer, &scaleX, &scaleY);

        if(SDL_GetRenderTarget(screen.renderer) != nullptr)
        {
            if(dstrect != nullptr)
            {
                SDL_Rect scaledRect = {
                    static_cast<int>(float(dstrect->x) * scaleX),
                    static_cast<int>(float(dstrect->y) * scaleY),
                    static_cast<int>(float(dstrect->w) * scaleX),
                    static_cast<int>(float(dstrect->h) * scaleY),
                };

                return SDL_RenderCopy(screen.renderer, tex, srcrect, &scaledRect);
            }
            else
            {
                return SDL_RenderCopy(screen.renderer, tex, srcrect, dstrect);
            }
        }

        int w;
        int h;
        SDL_GetWindowSize(screen.window, &w, &h);

        int renderAreaW = w;
        int renderAreaH = h;

        float aspect = float(w) / float(h);
        float aspectDefault = float(screen.logicalW) / float(screen.logicalH);

        if(aspect > aspectDefault) // extra width
        {
            renderAreaW = aspectDefault * float(h);
        }
        else if(aspect < aspectDefault) // extra height
        {
            renderAreaH = float(w) / aspectDefault;
        }

        if(dstrect == nullptr)
        {
            SDL_Rect rect = { screen.renderAreaX, screen.renderAreaY, renderAreaW, renderAreaH };

            SDL_RenderSetScale(screen.renderer, 1.0, 1.0);
            int rc = SDL_RenderCopy(screen.renderer, tex, srcrect, &rect);
            SDL_RenderSetScale(screen.renderer, scaleX, scaleY);

            return rc;
        }

        int scaledRenderAreaX = static_cast<int>(float(screen.innerRenderAreaX) / scaleX);
        int scaledRenderAreaY = static_cast<int>(float(screen.innerRenderAreaY) / scaleY);

        SDL_Rect dst = { dstrect->x, dstrect->y, dstrect->w, dstrect->h };

        dst.x += scaledRenderAreaX;
        dst.y += scaledRenderAreaY;

        return SDL_RenderCopy(screen.renderer, tex, srcrect, &dst);
    }

    int RenderFillRect(const Screen& screen, SDL_Rect *rect)
    {
        if(screen.window == nullptr || screen.renderer == nullptr)
        {
            return -1;
        }

        if(SDL_GetRenderTarget(screen.renderer) != nullptr)
        {
            return SDL_RenderFillRect(screen.renderer, rect);
        }

        int w;
        int h;
        SDL_GetWindowSize(screen.window, &w, &h);

        int renderAreaW = w;
        int renderAreaH = h;

        float scaleX;
        float scaleY;
        SDL_RenderGetScale(screen.renderer, &scaleX, &scaleY);

        float aspect = float(w) / float(h);
        float aspectDefault = float(screen.logicalW) / float(screen.logicalH);

        if(aspect > aspectDefault) // extra width
        {
            renderAreaW = aspectDefault * float(h);
        }
        else if(aspect < aspectDefault) // extra height
        {
            renderAreaH = float(w) / aspectDefault;
        }

        if(rect == nullptr)
        {
            SDL_Rect rect_ = { screen.renderAreaX, screen.renderAreaY, renderAreaW, renderAreaH };

            SDL_RenderSetScale(screen.renderer, 1.0, 1.0);
            int rc = SDL_RenderFillRect(screen.renderer, &rect_);
            SDL_RenderSetScale(screen.renderer, scaleX, scaleY);

            return rc;
        }

        int scaledRenderAreaX = static_cast<int>(float(screen.innerRenderAreaX) / scaleX);
        int scaledRenderAreaY = static_cast<int>(float(screen.innerRenderAreaY) / scaleY);

        SDL_Rect dst = { rect->x, rect->y, rect->w, rect->h };

        dst.x += scaledRenderAreaX;
        dst.y += scaledRenderAreaY;

        return SDL_RenderFillRect(screen.renderer, &dst);
    }
}