/**
 * Copyright (c) 2020 Brandon McGriff and Felicity Violette
 *
 * Licensed under the MIT license; see the LICENSE-src file at the top level
 * directory for the full text of the license.
 */
#pragma once
#include "Settings.h"
#include "SDL.h"
#ifdef ENABLE_OPENGL_INTERPOLATION
        #include "glad/glad.h"
#endif
#include <string>

namespace Shiro {
    struct Screen {
        Screen() = delete;

        Screen(const std::string& name, const unsigned w, const unsigned h);
        ~Screen();

        bool init(const Settings& settings);

        std::string name;
        unsigned w;
        unsigned h;
        SDL_Window* window;
        SDL_Renderer* renderer;
        SDL_Texture* target_tex;
#ifdef ENABLE_OPENGL_INTERPOLATION
        GLuint interpolate_shading_prog;
#endif
    };
}