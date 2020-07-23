#pragma once
#include "definitions.h"
#include "input/ControllerBindings.h"
#include "input/KeyBindings.h"
#include <filesystem>
#include <string>
namespace Shiro {
    class Settings {
    public:
        Settings(const std::filesystem::path &basePath);
        void read(const std::string filename);
        KeyBindings keyBindings;
        ControllerBindings controllerBindings;
        float videoScale;
        int videoStretch;
        int fullscreen;
        int vsync;
        int frameDelay;
        int vsyncTimestep;
#ifdef ENABLE_OPENGL_INTERPOLATION
        int interpolate;
#endif
        int masterVolume;
        int sfxVolume;
        int musicVolume;
        std::filesystem::path basePath;
        std::string playerName;
    };
}