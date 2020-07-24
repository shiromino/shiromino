#pragma once
#include "PDINI.h"
#include <SDL.h>
#include <string>
namespace Shiro {
    class KeyBindings {
    public:
        KeyBindings();
        KeyBindings(const int playerNumber);
        void read(PDINI::INI& ini);
        SDL_Keycode left;
        SDL_Keycode right;
        SDL_Keycode up;
        SDL_Keycode down;
        SDL_Keycode start;
        SDL_Keycode a;
        SDL_Keycode b;
        SDL_Keycode c;
        SDL_Keycode d;
        SDL_Keycode escape;
    };
}