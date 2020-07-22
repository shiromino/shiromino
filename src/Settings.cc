#include "Settings.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cinttypes>
#include <filesystem>
#include <iostream>
static std::array<std::string, 10> keyBindingNames = {
    "LEFT",
    "RIGHT",
    "UP",
    "DOWN",
    "START",
    "A",
    "B",
    "C",
    "D",
    "ESCAPE"
};
Shiro::KeyBindings::KeyBindings() : KeyBindings(0) {}
/**
 * We have to guarantee some default control option for fresh installations, so
 * keyboard is the best option. Controllers don't currently have any defaults set.
 */
Shiro::KeyBindings::KeyBindings(int playerNumber) {
    switch (playerNumber) {
        default:
        case 0:
            left = SDLK_LEFT;
            right = SDLK_RIGHT;
            up = SDLK_UP;
            down = SDLK_DOWN;
            start = SDLK_RETURN;
            a = SDLK_f;
            b = SDLK_d;
            c = SDLK_s;
            d = SDLK_a;
            escape = SDLK_ESCAPE;
            break;
        case 1:
            left = SDLK_j;
            right = SDLK_l;
            up = SDLK_i;
            down = SDLK_k;
            start = SDLK_TAB;
            a = SDLK_r;
            b = SDLK_e;
            c = SDLK_w;
            d = SDLK_q;
            escape = SDLK_F11;
            break;
    }
}
bool Shiro::KeyBindings::read(PDINI::INI& ini, const std::string sectionName) {
    bool defaultUsed = false;
    SDL_Keycode* const keycodes[] = {
        &left,
        &right,
        &up,
        &down,
        &start,
        &a,
        &b,
        &c,
        &d,
        &escape
    };
    SDL_Keycode* const* keycode = keycodes;
    for (const auto& keyBindingName : keyBindingNames) {
        std::string keyName;
        if (!ini.get(sectionName, keyBindingName, keyName) || SDL_GetKeyFromName(keyName.c_str()) == SDLK_UNKNOWN) {
            std::cerr << "Binding for " << keyBindingName << " is invalid." << std::endl;
            defaultUsed = true;
        }
        else {
            **keycode = SDL_GetKeyFromName(keyName.c_str());
        }
        ++keycode;
    }
    return defaultUsed;
}
Shiro::GamepadBindings::GamepadBindings() :
    name(""),
    gamepadIndex(-1),
    gamepadID(-1),
    hatIndex(-1) {}
void Shiro::GamepadBindings::read(PDINI::INI& ini, const std::string sectionName) {
    if (!ini.get(sectionName, "CONTROLLER_NAME", name) && !ini.get(sectionName, "CONTROLLER_INDEX", gamepadIndex)) {
        // When no controller name nor index is set in the INI, just disable
        // controller input completely.
        name = "";
        gamepadIndex = -1;
    }
    // A controller was selected; controller axes and hat settings have defaults,
    // but buttons don't.
    unsigned buttonGets = 0u;
    buttonGets += ini.get(sectionName, "BUTTON_LEFT", buttons.left) && buttons.left >= 0;
    buttonGets += ini.get(sectionName, "BUTTON_RIGHT", buttons.right) && buttons.right >= 0;
    buttonGets += ini.get(sectionName, "BUTTON_UP", buttons.up) && buttons.up >= 0;
    buttonGets += ini.get(sectionName, "BUTTON_DOWN", buttons.down) && buttons.down >= 0;
    buttonGets += ini.get(sectionName, "BUTTON_START", buttons.start) ? buttons.start >= 0 : 0u;
    buttonGets += ini.get(sectionName, "BUTTON_A", buttons.a) ? buttons.a >= 0 : 0u;
    buttonGets += ini.get(sectionName, "BUTTON_B", buttons.b) ? buttons.b >= 0 : 0u;
    buttonGets += ini.get(sectionName, "BUTTON_C", buttons.c) ? buttons.c >= 0 : 0u;
    buttonGets += ini.get(sectionName, "BUTTON_D", buttons.d) ? buttons.d >= 0 : 0u;
    buttonGets += ini.get(sectionName, "BUTTON_ESCAPE", buttons.escape) ? buttons.escape >= 0 : 0u;
    unsigned axisGets = 0u;
    axisGets += ini.get(sectionName, "AXIS_X", axes.x) && axes.x >= 0;
    axisGets += ini.get(sectionName, "AXIS_Y", axes.y) && axes.y >= 0;
    if (axisGets == 0u) {
        axes.x = 0;
        axes.right = 1;
        axes.y = 1;
        axes.down = 1;
    }
    else {
        if (axes.x >= 0) {
            std::string axisDirection = "";
            if (ini.get(sectionName, "AXIS_RIGHT", axisDirection)) {
                if (axisDirection == "+") {
                    axes.right = 1;
                }
                else if (axisDirection == "-") {
                    axes.right = -1;
                }
            }
            if (axes.right == 0) {
                axes.right = 1;
            }
        }
        else {
            axes.x = 0;
            axes.right = 1;
        }
        if (axes.y >= 0) {
            std::string axisDirection = "";
            if (ini.get(sectionName, "AXIS_DOWN", axisDirection)) {
                if (axisDirection == "+") {
                    axes.down = 1;
                }
                else if (axisDirection == "-") {
                    axes.down = -1;
                }
            }
            if (axes.down == 0) {
                axes.down = 1;
            }
        }
        else {
            axes.y = 1;
            axes.down = 1;
        }
        // Use default axis numbers if the user accidentally makes them
        // identical.
        if (axes.x == axes.y) {
            axes.x = 0;
            axes.y = 1;
        }
    }
    if (!ini.get(sectionName, "HAT_INDEX", hatIndex) || hatIndex < 0) {
        hatIndex = 0;
    }
}
Shiro::GamepadBindings::Buttons::Buttons() :
    left(-1),
    right(-1),
    up(-1),
    down(-1),
    start(-1),
    a(-1),
    b(-1),
    c(-1),
    d(-1),
    escape(-1) {}
Shiro::GamepadBindings::Axes::Axes() :
    x(-1),
    right(0),
    y(-1),
    down(0) {}
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
    this->keyBindings.read(ini, "PLAYER_1_KEY_BINDINGS");
    this->gamepadBindings.read(ini, "PLAYER_1_CONTROLLER_BINDINGS");
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