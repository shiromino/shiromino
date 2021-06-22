#include "Settings.h"
#include "Debug.h"
#include "OS.h"
#include "PDINI.h"
#include "Records.h"
#include "SDL.h"
#include "SDL_mixer.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <cinttypes>
#include <filesystem>
#include <iostream>
#include <optional>
#include <sstream>
namespace fs = std::filesystem;
const auto applicationName = "shiromino";
std::string env(const char *string) {
    const auto value = std::getenv(string);
    return value
        ? value
        : std::string();
}
const auto xdgConfigHome = env("XDG_CONFIG_HOME").empty()
        ? std::nullopt
        : std::optional(env("XDG_CONFIG_HOME"));
const auto xdgConfigHomeDefault = env("HOME").empty()
        ? std::nullopt
        : std::optional(fs::path(env("HOME")) / ".config");
const auto xdgDataHome = env("XDG_DATA_HOME").empty()
        ? std::nullopt
        : std::optional(fs::path(env("XDG_DATA_HOME")) / applicationName);
const auto xdgDataHomeDefault = env("HOME").empty()
        ? std::nullopt
        : std::optional(fs::path(env("HOME")) / ".local" / "share" / applicationName);
const auto xdgCacheHome = env("XDG_CACHE_HOME").empty()
        ? std::nullopt
        : std::optional(fs::path(env("XDG_CACHE_HOME")) / applicationName);
const auto xdgCacheHomeDefault = env("HOME").empty()
        ? std::nullopt
        : std::optional(fs::path(env("HOME")) / ".cache" / applicationName);
static void printHelp(const char* const executableName);
Shiro::Settings::Settings() {
    setDefaults();
}
void Shiro::Settings::setDefaults() {
    keyBindings = KeyBindings();
    controllerBindings = ControllerBindings();
    videoScale = 1.0f;
    videoStretch = 1;
    fullscreen = 0;
    vsync = 0;
    vsyncTimestep = 0;
    masterVolume = 80;
    sfxVolume = 100;
    musicVolume = 90;
    samplingRate = 48000;
    sampleSize = 1024;
    playerName = "ARK";
}
void Shiro::Settings::resolvePaths(PDINI::INI configuration, const fs::path &executableDirectory, const fs::path &cwd) {
    std::string configurationCachePath;
    if (!configuration.get("PATHS", "CACHE_PATH", configurationCachePath)) {
        const std::vector<std::optional<fs::path>> cachePrefixes = {
            executableDirectory,
            fs::weakly_canonical(executableDirectory / ".." / "var" / "cache" / applicationName),
            cwd,
            xdgCacheHome,
            xdgCacheHomeDefault,
            fs::path("/usr/local/var/cache") / applicationName,
            fs::path("/var/cache") / applicationName
        };
        for (const auto &prefix : cachePrefixes) {
            if (!prefix) {
                continue;
            }
            const auto candidate = prefix.value() / Shiro::Records::filename;
            if (fs::exists(candidate)) {
                cachePath = prefix.value();
                break;
            }
        }
        if (cachePath.empty()) {
            std::cerr << "Couldn't determine cache directory. Using default value." << std::endl;
            const std::vector<std::optional<fs::path>> paths = {
                xdgCacheHome,
                xdgCacheHomeDefault,
                executableDirectory
            };
            for (const auto &path : paths) {
                if (!path) {
                    continue;
                }
                fs::create_directories(path.value());
                cachePath = path.value();
                break;
            }
        }
    }
    std::string configurationSharePath;
    if (!configuration.get("PATHS", "SHARE_PATH", configurationSharePath)) {
        const std::vector<std::optional<fs::path>> sharePrefixes = {
            executableDirectory,
            fs::weakly_canonical(executableDirectory / ".." / "share" / applicationName),
            cwd,
            xdgDataHome,
            xdgDataHomeDefault,
            fs::path("/usr/local/share") / applicationName,
            fs::path("/usr/share") / applicationName
        };
        for (const auto &prefix : sharePrefixes) {
            if (!prefix) {
                continue;
            }
            const auto candidate = prefix.value() / "data";
            if (fs::exists(candidate)) {
                sharePath = candidate;
                break;
            }
        }
        if (sharePath.empty()) {
            throw std::logic_error("Unable to determine share path.");
        }
    }
}
bool Shiro::Settings::init(const int argc, const char *const argv[]) {
    const auto cwd = fs::current_path();
    const auto executableDirectory = OS::getExecutablePath().remove_filename();
    PDINI::INI configuration;
    if (argc == 1) {
        PDINI::INI configuration;
        const std::vector<std::optional<fs::path>> configurationPrefixes = {
            executableDirectory,
            fs::weakly_canonical(executableDirectory / ".." / "etc"),
            cwd,
            xdgConfigHome,
            xdgConfigHomeDefault,
            "/usr/local/etc/",
            "/etc"
        };
        for (const auto &prefix : configurationPrefixes) {
            if (!prefix) {
                continue;
            }
            const auto candidate = prefix.value() / "shiromino.ini";
            if (fs::exists(candidate)) {
                configuration = read(candidate.string());
                break;
            }
        }
        if (configurationPath.empty()) {
            std::cerr << "Couldn't find configuration file. Creating new configuration file." << std::endl;
            const std::vector<std::optional<fs::path>> paths = {
                xdgConfigHome,
                xdgConfigHomeDefault,
                executableDirectory
            };
            bool success = false;
            for (const auto &path : paths) {
                if (!path) {
                    continue;
                }
                if (fs::exists(path.value())) {
                    configurationPath = path.value();
                    setDefaults();
                    write();
                    success = true;
                    break;
                }
            }
            if (!success) {
                throw std::logic_error("Unable to create new configuration file.");
            }
        }
        resolvePaths(configuration, executableDirectory, cwd);
    }
    /* TODO: Use an argument handler library here */
    else if (argc == 3) {
        const auto firstArgument = std::string(argv[1]);
        const auto secondArgument = std::string(argv[2]);
        if (firstArgument == "--configuration-file" || firstArgument == "-c") {
            configurationPath = fs::path(secondArgument);
            resolvePaths(read(configurationPath.string()), executableDirectory, cwd);
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
PDINI::INI Shiro::Settings::read(const std::string &filename) {
    if (!fs::exists(filename)) {
        std::stringstream ss;
        ss << "Couldn't find configuration file: `" << filename << "`";
        throw std::logic_error(ss.str());
    }
    PDINI::INI configuration;
    auto [wasReadSuccessful, lineNumber] = configuration.read(filename);
    if (lineNumber > 0) {
        std::cerr << "Error reading configuration `" << filename << "` on line " << lineNumber << std::endl;
    }
    if (!wasReadSuccessful) {
        std::cerr << "Failed opening configuration `" << filename << "`." << std::endl;
    }
    this->keyBindings.read(configuration);
    this->controllerBindings.read(configuration);
    fs::path specifiedCachePath;
    auto relativeTo = configurationPath;
    relativeTo.remove_filename();
    if (configuration.get("PATHS", "CACHE_PATH", specifiedCachePath)) {
        if (specifiedCachePath.is_relative()) {
            this->cachePath = fs::canonical(relativeTo / specifiedCachePath);
        }
        else {
            this->cachePath = specifiedCachePath;
        }
    }
    fs::path specifiedSharePath;
    if (configuration.get("PATHS", "SHARE_PATH", specifiedSharePath)) {
        if (specifiedSharePath.is_relative()) {
            this->sharePath = fs::canonical(relativeTo / specifiedSharePath);
        }
        else {
            this->sharePath = specifiedSharePath;
        }
    }
    unsigned int masterVolume;
    if (configuration.get("AUDIO", "MASTER_VOLUME", masterVolume)) {
        this->masterVolume = std::clamp(masterVolume, 0u, 100u);
    }
    unsigned int sfxVolume;
    if (configuration.get("AUDIO", "SFX_VOLUME", sfxVolume)) {
        this->sfxVolume = std::clamp(sfxVolume, 0u, 100u);
    }
    unsigned int musicVolume;
    if (configuration.get("AUDIO", "MUSIC_VOLUME", musicVolume)) {
        this->musicVolume = std::clamp(musicVolume, 0u, 100u);
    }
    unsigned int samplingRate;
    if (configuration.get("AUDIO", "SAMPLING_RATE", samplingRate)) {
        this->samplingRate = std::clamp(samplingRate, 0u, 48000u);
    }
    unsigned int sampleSize;
    if (configuration.get("AUDIO", "SAMPLE_SIZE", sampleSize)) {
        this->sampleSize = std::clamp(sampleSize, 0u, 4096u);
    }
    float videoScale;
    if (configuration.get("SCREEN", "VIDEO_SCALE", videoScale)) {
        const auto epsilon = std::numeric_limits<decltype(videoScale)>::epsilon();
        this->videoScale = std::max(epsilon, videoScale);
    }
    int videoStretch;
    if (configuration.get("SCREEN", "VIDEO_STRETCH", videoStretch)) {
        this->videoStretch = videoStretch;
    }
    int fullscreen;
    if (configuration.get("SCREEN", "FULL_SCREEN", fullscreen)) {
        this->fullscreen = !!fullscreen;
    }
    int vsync;
    if (configuration.get("SCREEN", "V_SYNC", vsync)) {
        this->vsync = !!vsync;
    }
    int vsyncTimestep;
    if (configuration.get("SCREEN", "V_SYNC_TIME_STEP", vsyncTimestep)) {
        this->vsyncTimestep = !!vsyncTimestep;
    }
    std::string playerName;
    if (configuration.get("ACCOUNT", "PLAYER_NAME", playerName)) {
        this->playerName = playerName;
    }
    this->configurationPath = filename;
    return configuration;
}
void Shiro::Settings::write() const {
    PDINI::INI configuration;
    keyBindings.write(configuration);
    controllerBindings.write(configuration);
    configuration.set("ACCOUNT", "PLAYER_NAME", playerName);
    configuration.set("AUDIO", "MASTER_VOLUME", masterVolume);
    configuration.set("AUDIO", "MUSIC_VOLUME", musicVolume);
    configuration.set("AUDIO", "SAMPLING_RATE", samplingRate);
    configuration.set("AUDIO", "SAMPLE_SIZE", sampleSize);
    configuration.set("AUDIO", "SFX_VOLUME", sfxVolume);
    configuration.set("SCREEN", "FULL_SCREEN", fullscreen);
    configuration.set("SCREEN", "VIDEO_SCALE", videoScale);
    configuration.set("SCREEN", "VIDEO_STRETCH", videoStretch);
    configuration.set("SCREEN", "V_SYNC", vsync);
    configuration.set("SCREEN", "V_SYNC_TIME_STEP", vsyncTimestep);
    // Unfortunately, Microsoft left operator std::string() out of
    // fs::path. We can still get by with
    // fs::path::string(), though.
    // TODO: If operator std::string() gets added to MSVC, change this to use
    // that operator.
    configuration.write(configurationPath.string());
}
static void printHelp(const char* executableName) {
    std::cerr << "Usage: " << executableName << " --configuration-file <configuration file>" << std::endl;
}