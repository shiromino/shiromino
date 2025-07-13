#include "video/Screen.h"
#include "SDL_blendmode.h"
#include "SDL_error.h"
#include "Debug.h"
#include "Settings.h"
#include "types.h"

Shiro::Screen::Screen(const std::string& name, const unsigned w, const unsigned h, const unsigned logicalW, const unsigned logicalH, const float render_scale) :
    name(name),
    w(w),
    h(h),
    logicalW(logicalW),
    logicalH(logicalH),
    renderAreaX(0),
    renderAreaY(0),
    renderAreaW(0),
    renderAreaH(0),
    innerRenderAreaX(0),
    innerRenderAreaY(0),
    render_scale(render_scale),
    window(nullptr),
    renderer(nullptr),
    target_tex(nullptr)
{}

Shiro::Screen::~Screen() {
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }

    if (window) {
        SDL_DestroyWindow(window);
    }
}

bool Shiro::Screen::init(const Settings& settings) {
    u32 windowFlags = SDL_WINDOW_RESIZABLE;
    window = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, windowFlags);
    if (window == nullptr) {
        log_err("SDL_CreateWindow: %s", SDL_GetError());
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE | (settings.vsync ? SDL_RENDERER_PRESENTVSYNC : 0));
    if (renderer == nullptr) {
        log_err("SDL_CreateRenderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        return false;
    }

    if (SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND) < 0) {
        log_err("SDL_SetRenderDrawBlendMode: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return false;
    }

    SDL_SetWindowMinimumSize(window, 640, 480);
    if (settings.fullscreen) {
        if (SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP) < 0) {
            log_err("SDL_SetWindowFullscreen: %s", SDL_GetError());
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            return false;
        }
    }

    /*if (SDL_RenderSetLogicalSize(renderer, 640, 480) < 0) {
        log_err("SDL_RenderSetLogicalSize: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return false;
    }*/

    /*
    if (!settings.videoStretch) {
        if (SDL_RenderSetIntegerScale(renderer, SDL_TRUE) < 0) {
            log_err("SDL_RenderSetIntegerScale: %s", SDL_GetError());
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            return false;
        }
    }
    */
    return true;
}

void Shiro::Screen::updateRenderAreaPosition() {
    float aspect = float(w) / float(h);
    float aspectDefault = float(logicalW) / float(logicalH);

    renderAreaX = 0;
    renderAreaY = 0;
    renderAreaW = w;
    renderAreaH = h;

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

    innerRenderAreaX = (w - static_cast<int>(float(logicalW) * render_scale)) / 2;
    innerRenderAreaY = (h - static_cast<int>(float(logicalH) * render_scale)) / 2;
}