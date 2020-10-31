#include "Settings.h"
#include "Debug.h"
#include "SDL.h"
#include "SDL_mixer.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cinttypes>
#include <filesystem>
#include <iostream>

static void printHelp(const char* const executableName);

Shiro::Settings::Settings() {
    setDefaults();
}

void Shiro::Settings::setDefaults(const std::filesystem::path& configurationPath, const std::filesystem::path& basePath) {
    keyBindings = KeyBindings();
    controllerBindings = ControllerBindings();
    videoScale = 1.0f;
    videoStretch = 1;
    fullscreen = 0;
    vsync = 0;
    frameDelay = 16;
    vsyncTimestep = 0;
#ifdef ENABLE_OPENGL_INTERPOLATION
    interpolate = 1;
#endif
    masterVolume = 80;
    sfxVolume = 100;
    musicVolume = 90;
    samplingRate = 48000;
    this->configurationPath = configurationPath;
    this->basePath = basePath;
    playerName = "ARK";
}

bool Shiro::Settings::init(const int argc, const char* const argv[]) {
    basePath = std::filesystem::current_path();
    if (argc == 1) {
        configurationPath = basePath / "shiromino.ini";
        if (!std::filesystem::exists(configurationPath)) {
            std::cerr << "Couldn't find configuration file `" << configurationPath.string() << "`, creating a new configuration file" << std::endl;
            setDefaults(configurationPath, basePath);
            write();
        }
        else {
            read(configurationPath.string());
        }
    }
    /* TODO: Use an argument handler library here */
    else if (argc == 3) {
        const auto firstArgument = std::string(argv[1]);
        const auto secondArgument = std::string(argv[2]);
        if (firstArgument == "--configuration-file" || firstArgument == "-c") {
            configurationPath = std::filesystem::path(secondArgument);
            if (!std::filesystem::exists(configurationPath)) {
                std::cerr << "Couldn't find configuration file `" << configurationPath.string() << "`, creating a new configuration file" << std::endl;
                setDefaults(configurationPath, basePath);
                write();
            }
            else {
                basePath = std::filesystem::canonical(configurationPath).remove_filename();
                read(configurationPath.string());
            }
        }
        else {
            printHelp(argv[0]);
            return false;
        }
    }
    else {
        printHelp(argv[0]);
        return false;
    }

    return true;
}


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
    unsigned int samplingRate;
    if (ini.get("AUDIO", "SAMPLING_RATE", samplingRate)) {
        this->samplingRate = std::clamp(samplingRate, 0u, 48000u);
    }
    float videoScale;
    if (ini.get("SCREEN", "VIDEO_SCALE", videoScale)) {
        const auto epsilon = std::numeric_limits<decltype(videoScale)>::epsilon();
        this->videoScale = std::max(epsilon, videoScale);
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
        this->frameDelay = std::max(0, frameDelay);
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

    this->configurationPath = filename;
}

void Shiro::Settings::write() const {
    PDINI::INI ini;

    keyBindings.write(ini);
    controllerBindings.write(ini);

    ini.set("PATHS", "BASE_PATH", basePath);

    ini.set("AUDIO", "MASTER_VOLUME", masterVolume);
    ini.set("AUDIO", "SFX_VOLUME", sfxVolume);
    ini.set("AUDIO", "MUSIC_VOLUME", musicVolume);
    ini.set("AUDIO", "SAMPLING_RATE", samplingRate);

    ini.set("SCREEN", "VIDEO_SCALE", videoScale);
    ini.set("SCREEN", "VIDEO_STRETCH", videoStretch);
    ini.set("SCREEN", "FULL_SCREEN", fullscreen);
    ini.set("SCREEN", "V_SYNC", vsync);
    ini.set("SCREEN", "FRAME_DELAY", frameDelay);
    ini.set("SCREEN", "V_SYNC_TIME_STEP", vsyncTimestep);
#ifdef ENABLE_OPENGL_INTERPOLATION
    ini.set("SCREEN", "INTERPOLATE", interpolate);
#endif

    ini.set("ACCOUNT", "PLAYER_NAME", playerName);

    // Unfortunately, Microsoft left operator std::string() out of
    // std::filesystem::path. We can still get by with
    // std::filesystem::path::string(), though.
    // TODO: If operator std::string() gets added to MSVC, change this to use
    // that operator.
    ini.write(configurationPath.string());
}

static void printHelp(const char* executableName) {
    std::cerr << "Usage: " << executableName << " --configuration-file <configuration file>" << std::endl;
}
