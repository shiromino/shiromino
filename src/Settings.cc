#include "Settings.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cinttypes>
#include <filesystem>
#include <iostream>
Shiro::Settings::Settings(const std::filesystem::path &basePath) :
    videoScale(1.0f),
    videoStretch(1),
    fullscreen(0),
    vsync(0),
    frameDelay(1),
    vsyncTimestep(0),
#ifdef ENABLE_OPENGL_INTERPOLATION
    interpolate(0),
#endif
    masterVolume(80),
    sfxVolume(100),
    musicVolume(90),
    basePath(basePath),
    playerName("ARK") {}
void Shiro::Settings::read(std::string filename) {
    PDINI::INI ini;
    auto [wasReadSuccessful, lineNumber] = ini.read(filename);
    if (lineNumber > 0) {
        std::cerr << "Error reading configuration INI \"" << filename << "\" on line " << lineNumber << std::endl;
    }
    if (!wasReadSuccessful) {
        std::cerr << "Failed opening configuration INI \"" << filename << "\"." << std::endl;
    }
    this->keyBindings.read(ini);
    this->controllerBindings.read(ini);
    std::filesystem::path specifiedBasePath;
    if (ini.get("PATHS", "BASE_PATH", specifiedBasePath)) {
        if (specifiedBasePath.is_relative()) {
            std::filesystem::path filenamePath = { filename };
            const auto filenameDirectory = filenamePath.remove_filename();
            const auto resolvedPath = std::filesystem::canonical(
                std::filesystem::path(specifiedBasePath) / filenameDirectory
            );
            this->basePath = resolvedPath;
        }
        else {
            this->basePath = specifiedBasePath;
        }
    }
    unsigned int masterVolume;
    if (ini.get("AUDIO", "MASTER_VOLUME", masterVolume)) {
        this->masterVolume = std::clamp(masterVolume, 0u, 100u);
    }
    unsigned int sfxVolume;
    if (ini.get("AUDIO", "SFX_VOLUME", sfxVolume)) {
        this->sfxVolume = std::clamp(sfxVolume, 0u, 100u);
    }
    unsigned int musicVolume;
    if (ini.get("AUDIO", "MUSIC_VOLUME", musicVolume)) {
        this->musicVolume = std::clamp(musicVolume, 0u, 100u);
    }
    float videoScale;
    if (ini.get("SCREEN", "VIDEO_SCALE", videoScale)) {
        const auto epsilon = std::numeric_limits<decltype(videoScale)>::epsilon();
        this->videoScale = std::min(epsilon, videoScale);
    }
    int videoStretch;
    if (ini.get("SCREEN", "VIDEO_STRETCH", videoStretch)) {
        this->videoStretch = videoStretch;
    }
    int fullscreen;
    if (ini.get("SCREEN", "FULL_SCREEN", fullscreen)) {
        this->fullscreen = !!fullscreen;
    }
    int vsync;
    if (ini.get("SCREEN", "V_SYNC", vsync)) {
        this->vsync = !!vsync;
    }
    int frameDelay;
    if (ini.get("SCREEN", "FRAME_DELAY", frameDelay)) {
        this->frameDelay = std::min(0, frameDelay);
    }
    int vsyncTimestep;
    if (ini.get("SCREEN", "V_SYNC_TIME_STEP", vsyncTimestep)) {
        this->vsyncTimestep = !!vsyncTimestep;
    }
#ifdef ENABLE_OPENGL_INTERPOLATION
    int interpolate;
    if (ini.get("SCREEN", "INTERPOLATE", interpolate)) {
        this->interpolate = !!interpolate;
    }
#endif
    std::string playerName;
    if (ini.get("ACCOUNT", "PLAYER_NAME", playerName)) {
        this->playerName = playerName;
    }
}