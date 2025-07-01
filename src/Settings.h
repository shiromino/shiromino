#pragma once
#include <filesystem>
#include <string>
#include "PDINI.h"
#include "input/ControllerBindings.h"
#include "input/KeyBindings.h"

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
        int vsyncTimestep;
        int masterVolume;
        int sfxVolume;
        int musicVolume;
        int samplingRate;
        int sampleSize;
        std::filesystem::path configurationPath;
        std::filesystem::path cachePath;
        std::filesystem::path sharePath;
        std::string playerName;
    };
}