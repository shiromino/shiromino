#include "Settings.hpp"
#include <array>
#include <cstdio>
#include <cstdint>
#include <cinttypes>

using namespace Shiro;
using namespace std;
using namespace PDINI;

static array<string, 10> KeyBindNames = {
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

KeyBinds::KeyBinds() : KeyBinds(0) {}

/**
 * We have to guarantee some default control option for fresh installs, so
 * keyboard is the best option. Other inputs, like joysticks, don't have any
 * defaults set.
 */
KeyBinds::KeyBinds(int playerNum) {
    switch (playerNum) {
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

bool KeyBinds::read(INI& ini, const string sectionName) {
    bool defaultUsed = false;
    SDL_Keycode* const keycodes[] = {&left, &right, &up, &down, &start, &a, &b, &c, &d, &escape};
    SDL_Keycode* const* keycode = keycodes;
    for (const auto keyBindName : KeyBindNames) {
        string keyName;
        if (!ini.get(sectionName, keyBindName, keyName) || SDL_GetKeyFromName(keyName.c_str()) == SDLK_UNKNOWN) {
            fprintf(stderr, "Binding for %s is invalid\n", keyBindName.c_str());
            defaultUsed = true;
        }
        else {
            **keycode = SDL_GetKeyFromName(keyName.c_str());
        }
        keycode++;
    }
    return defaultUsed;
}

JoyBinds::JoyBinds() : name(""), joyIndex(-1), joyID(-1), hatIndex(-1) {}

bool JoyBinds::read(INI& ini, const string sectionName) {
    bool defaultUsed = false;

    if (!ini.get(sectionName, "JOYNAME", name) && !ini.get(sectionName, "JOYINDEX", joyIndex)) {
        // When no joystick name nor index is set in the INI, just disable
        // joystick input completely.
        name = "";
        joyIndex = -1;
        return defaultUsed;
    }

    // A joystick was selected; joystick axes and hat settings have defaults,
    // but buttons don't.

    unsigned buttonGets = 0u;
    buttonGets += ini.get(sectionName, "BUTTONLEFT", buttons.left) && buttons.left >= 0;
    buttonGets += ini.get(sectionName, "BUTTONRIGHT", buttons.right) && buttons.right >= 0;
    buttonGets += ini.get(sectionName, "BUTTONUP", buttons.up) && buttons.up >= 0;
    buttonGets += ini.get(sectionName, "BUTTONDOWN", buttons.down) && buttons.down >= 0;
    buttonGets += ini.get(sectionName, "BUTTONSTART", buttons.start) ? buttons.start >= 0 : 0u;
    buttonGets += ini.get(sectionName, "BUTTONA", buttons.a) ? buttons.a >= 0 : 0u;
    buttonGets += ini.get(sectionName, "BUTTONB", buttons.b) ? buttons.b >= 0 : 0u;
    buttonGets += ini.get(sectionName, "BUTTONC", buttons.c) ? buttons.c >= 0 : 0u;
    buttonGets += ini.get(sectionName, "BUTTOND", buttons.d) ? buttons.d >= 0 : 0u;
    buttonGets += ini.get(sectionName, "BUTTONESCAPE", buttons.escape) ? buttons.escape >= 0 : 0u;
    if (buttonGets != 10u) {
        defaultUsed = true;
    }

    unsigned axisGets = 0u;
    axisGets += ini.get(sectionName, "AXISX", axes.x) && axes.x >= 0;
    axisGets += ini.get(sectionName, "AXISY", axes.y) && axes.y >= 0;
    if (axisGets == 0u) {
        axes.x = 0;
        axes.right = 1;
        axes.y = 1;
        axes.down = 1;
        defaultUsed = true;
    }
    else {
        if (axes.x >= 0) {
            string axisDirection = "";
            if (ini.get(sectionName, "AXISRIGHT", axisDirection)) {
                if (axisDirection == "+") {
                    axes.right = 1;
                }
                else if (axisDirection == "-") {
                    axes.right = -1;
                }
            }
            if (axes.right == 0) {
                axes.right = 1;
                defaultUsed = true;
            }
        }
        else {
            axes.x = 0;
            axes.right = 1;
            defaultUsed = true;
        }

        if (axes.y >= 0) {
            string axisDirection = "";
            if (ini.get(sectionName, "AXISDOWN", axisDirection)) {
                if (axisDirection == "+") {
                    axes.down = 1;
                }
                else if (axisDirection == "-") {
                    axes.down = -1;
                }
            }
            if (axes.down == 0) {
                axes.down = 1;
                defaultUsed = true;
            }
        }
        else {
            axes.y = 1;
            axes.down = 1;
            defaultUsed = true;
        }

        // Use default axis numbers if the user accidentally makes them
        // identical.
        if (axes.x == axes.y) {
            axes.x = 0;
            axes.y = 1;
        }
    }

    if (!ini.get(sectionName, "HATINDEX", hatIndex) || hatIndex < 0) {
        hatIndex = 0;
        defaultUsed = true;
    }

    return defaultUsed;
}

JoyBinds::Buttons::Buttons() : left(-1), right(-1), up(-1), down(-1), start(-1), a(-1), b(-1), c(-1), d(-1), escape(-1) {}
JoyBinds::Axes::Axes() : x(-1), right(0), y(-1), down(0) {}

Settings::Settings() :
    videoScale(1.0f),
    videoStretch(1),
    fullscreen(0),
    vsync(0),
    frameDelay(1),
    vsyncTimestep(0),
#ifdef OPENGL_INTERPOLATION
    interpolate(0),
#endif
    masterVolume(80),
    sfxVolume(100),
    musicVolume(90),
    basePath("."),
    playerName("ARK") {}

bool Settings::read(string filename) {
    INI ini;
    auto readStatus = ini.read(filename);
    if (readStatus.second > 0) {
        fprintf(stderr, "Error reading configuation INI \"%s\" on line %zu\n", filename.c_str(), readStatus.second);
    }
    if (!readStatus.first) {
        fprintf(stderr, "Failed opening configuration INI \"%s\"\n", filename.c_str());
        return true;
    }

    // [P1KEYBINDS]
    bool defaultUsed = this->keyBinds.read(ini, "P1KEYBINDS");

    // [P1JOYBINDS]
    if (!this->joyBinds.read(ini, "P1JOYBINDS")) {
        defaultUsed = true;
    }

    // [PATHS]
    string basePath;
    if (!ini.get("PATHS", "BASE_PATH", basePath)) {
        char *basePath = SDL_GetBasePath();
        size_t i = strlen(basePath);
        do {
            if (basePath[i] == '\\' || basePath[i] == '/') {
                basePath[i] = '\0';
            }
            if (i == 0) {
                break;
            }
            i--;
        } while (true);
        this->basePath = basePath;
        SDL_free(basePath);
        defaultUsed = true;
    }
    else {
        this->basePath = basePath;
    }

    // [AUDIO]
    int volume;
    if (!ini.get("AUDIO", "MASTERVOLUME", volume) || (volume < 0 && volume > 100)) {
        defaultUsed = true;
    }
    else {
        this->masterVolume = volume;
    }
    if (!ini.get("AUDIO", "SFXVOLUME", volume) || (volume < 0 && volume > 100)) {
        defaultUsed = true;
    }
    else {
        this->sfxVolume = volume;
    }
    if (!ini.get("AUDIO", "MUSICVOLUME", volume) || (volume < 0 && volume > 100)) {
        defaultUsed = true;
    }
    else {
        this->musicVolume = volume;
    }

    // [SCREEN]
    float videoScale;
    if (!ini.get("SCREEN", "VIDEOSCALE", videoScale) || videoScale <= 0.0f) {
        defaultUsed = true;
    }
    else {
        this->videoScale = videoScale;
    }
    
    int videoStretch;
    if (!ini.get("SCREEN", "VIDEOSTRETCH", videoStretch)) {
        defaultUsed = true;
    }
    else {
        this->videoStretch = videoStretch;
    }
    
    int fullscreen;
    if (!ini.get("SCREEN", "FULLSCREEN", fullscreen)) {
        defaultUsed = true;
    }
    else {
        this->fullscreen = !!fullscreen;
    }

    int vsync;
    if (!ini.get("SCREEN", "VSYNC", vsync)) {
        defaultUsed = true;
    }
    else {
        this->vsync = !!vsync;
    }

    int frameDelay;
    if (!ini.get("SCREEN", "FRAMEDELAY", frameDelay) || frameDelay < 0) {
        defaultUsed = true;
    }
    else {
        this->frameDelay = frameDelay;
    }

    int vsyncTimestep;
    if (!ini.get("SCREEN", "VSYNCTIMESTEP", vsyncTimestep)) {
        defaultUsed = true;
    }
    else {
        this->vsyncTimestep = !!vsyncTimestep;
    }

#ifdef OPENGL_INTERPOLATION
    int interpolate;
    if (!ini.get("SCREEN", "INTERPOLATE", interpolate)) {
        defaultUsed = true;
    }
    else {
        this->interpolate = interpolate;
    }
#endif

    // [ACCOUNT]
    string playerName;
    if (ini.get("ACCOUNT", "PLAYERNAME", playerName)) {
        this->playerName = playerName;
    }
    else {
        defaultUsed = true;
    }

    return defaultUsed;
}
