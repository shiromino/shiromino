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
        int renderAreaX = 0;
        int renderAreaY = 0;

        float aspect = float(w) / float(h);
        float aspectDefault = 640.0 / 480.0;

        if(aspect > aspectDefault) // extra width
        {
            renderAreaW = aspectDefault * float(h);
            renderAreaX = (w - renderAreaW) / 2;
        }
        else if(aspect < aspectDefault) // extra height
        {
            renderAreaH = float(w) / aspectDefault;
            renderAreaY = (h - renderAreaH) / 2;
        }

        if(dstrect == nullptr)
        {
            SDL_Rect rect = { renderAreaX, renderAreaY, renderAreaW, renderAreaH};

            SDL_RenderSetScale(screen.renderer, 1.0, 1.0);
            int rc = SDL_RenderCopy(screen.renderer, tex, srcrect, &rect);
            SDL_RenderSetScale(screen.renderer, scaleX, scaleY);

            return rc;
        }

        int scaledRenderAreaX = static_cast<int>(float((w - static_cast<int>(640.0 * scaleX)) / 2) / scaleX);
        int scaledRenderAreaY = static_cast<int>(float((h - static_cast<int>(480.0 * scaleY)) / 2) / scaleY);

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
        int renderAreaX = 0;
        int renderAreaY = 0;

        float scaleX;
        float scaleY;
        SDL_RenderGetScale(screen.renderer, &scaleX, &scaleY);

        float aspect = float(w) / float(h);
        float aspectDefault = 640.0 / 480.0;

        if(aspect > aspectDefault) // extra width
        {
            renderAreaW = aspectDefault * float(h);
            renderAreaX = (w - renderAreaW) / 2;
        }
        else if(aspect < aspectDefault) // extra height
        {
            renderAreaH = float(w) / aspectDefault;
            renderAreaY = (h - renderAreaH) / 2;
        }

        if(rect == nullptr)
        {
            SDL_Rect rect_ = { renderAreaX, renderAreaY, renderAreaW, renderAreaH};

            SDL_RenderSetScale(screen.renderer, 1.0, 1.0);
            int rc = SDL_RenderFillRect(screen.renderer, &rect_);
            SDL_RenderSetScale(screen.renderer, scaleX, scaleY);

            return rc;
        }

        int scaledRenderAreaX = static_cast<int>(float((w - static_cast<int>(640.0 * scaleX)) / 2) / scaleX);
        int scaledRenderAreaY = static_cast<int>(float((h - static_cast<int>(480.0 * scaleY)) / 2) / scaleY);

        SDL_Rect dst = { rect->x, rect->y, rect->w, rect->h };

        dst.x += scaledRenderAreaX;
        dst.y += scaledRenderAreaY;

        return SDL_RenderFillRect(screen.renderer, &dst);
    }
}