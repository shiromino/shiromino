#pragma once
#include "SDL_keycode.h"
#include "SDL_scancode.h"

namespace PDINI { class INI; }

namespace Shiro {
    struct KeyBinding {
        union {
            SDL_Keycode keycode;
            SDL_Scancode scancode;
        };
        bool isKeycode;
    };

    class KeyBindings {
    public:
        KeyBindings();
        KeyBindings(const int playerNumber);

        void read(PDINI::INI& ini);
        void write(PDINI::INI& ini) const;

        KeyBinding left;
        KeyBinding right;
        KeyBinding up;
        KeyBinding down;
        KeyBinding start;
        KeyBinding a;
        KeyBinding b;
        KeyBinding c;
        KeyBinding d;
        KeyBinding escape;
    };
}