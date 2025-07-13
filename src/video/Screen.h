#pragma once
#include "SDL_render.h"
#include "SDL_video.h"
#include <string>

namespace Shiro {
    struct Settings;
    
    struct Screen {
        Screen() = delete;

        Screen(const std::string& name, const unsigned w, const unsigned h, const unsigned logicalW, const unsigned logicalH, const float render_scale);
        ~Screen();

        bool init(const Settings& settings);
        void updateRenderAreaPosition();

        std::string name;

        // actual size of window
        unsigned w;
        unsigned h;

        // size of logical coordinate system
        unsigned logicalW;
        unsigned logicalH;

        // window-relative position and size of the render area
        int renderAreaX;
        int renderAreaY;
        int renderAreaW;
        int renderAreaH;

        // window-relative position of the render area used for GUI and sprites
        int innerRenderAreaX;
        int innerRenderAreaY;

        // scaling factor used by the SDL_Renderer
        float render_scale;

        SDL_Window* window;
        SDL_Renderer* renderer;
        SDL_Texture* target_tex;
    };
}