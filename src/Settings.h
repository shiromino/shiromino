#pragma once

#include <string>

#include "Config.hpp"
#include "Path.hpp"
#include "PDINI.hpp"
#include "SDL.h"

namespace Shiro {
    class KeyBindings {
    public:
        KeyBindings();
        KeyBindings(const int playerNum);

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

    class GamepadBindings {
    public:
        GamepadBindings();

        bool read(PDINI::INI& ini, const std::string sectionName);

        std::string name;
        int gamepadIndex;
        SDL_JoystickID gamepadID;

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

        KeyBindings keyBindings;
        GamepadBindings gamepadBindings;

        float videoScale;
        int videoStretch;
        int fullscreen;
        int vsync;
        int frameDelay;
        int vsyncTimestep;
#ifdef OPENGL_INTERPOLATION
        int interpolate;
#endif

        int masterVolume;
        int sfxVolume;
        int musicVolume;

        std::string basePath;

        std::string playerName;
    };
}
