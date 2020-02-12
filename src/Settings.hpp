#pragma once

#include "PDINI.hpp"
#include <string>
#include "SDL.h"

namespace Shiro {
    class KeyBinds {
    public:
        KeyBinds();
        KeyBinds(const int playerNum);

        bool read(PDINI::INI& ini, const std::string sectionName);

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

    class JoyBinds {
    public:
        JoyBinds();

        bool read(PDINI::INI& ini, const std::string sectionName);

        std::string name;
        int joyIndex;
        SDL_JoystickID joyID;

        struct Buttons {
            Buttons();

            int left;
            int right;
            int up;
            int down;
            int start;
            int a;
            int b;
            int c;
            int d;
            int escape;
        } buttons;

        struct Axes {
            Axes();

            int x;
            int right;

            int y;
            int down;

            // TODO: Axis bindings for non-directional inputs.
        } axes;

        int hatIndex;
    };

    class Settings {
    public:
        Settings();

        bool read(const std::string filename);

        KeyBinds keyBinds;
        JoyBinds joyBinds;
        // TODO: XInputBinds xInputBinds;

        float videoScale;
        int videoStretch;
        int fullscreen;
        int vsync;

        int masterVolume;
        int sfxVolume;
        int musicVolume;

        std::string basePath;

        std::string playerName;
    };
}
