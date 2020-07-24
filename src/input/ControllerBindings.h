#pragma once
#include "PDINI.h"
#include <SDL.h>
#include <string>
namespace Shiro {
    class ControllerBindings {
    public:
        ControllerBindings();
        void read(PDINI::INI& ini);
        std::string name;
        int controllerIndex;
        SDL_JoystickID controllerID;
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
};