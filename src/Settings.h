#pragma once
#include "definitions.h"
#include "Input/ControllerBindings.h"
#include "Input/KeyBindings.h"
#include "PDINI.h"
#include <filesystem>
#include <string>
namespace Shiro {
    struct Settings {
        Settings();
        void setDefaults();
        bool init(const int argc, const char* const argv[]);
        PDINI::INI read(const std::string &filename);
        void resolvePaths(PDINI::INI configuration, const std::filesystem::path &executablePath, const std::filesystem::path &cwd);
        void write() const;
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
        int samplingRate;
        std::filesystem::path configurationPath;
        std::filesystem::path cachePath;
        std::filesystem::path sharePath;
        std::string playerName;
    };
}