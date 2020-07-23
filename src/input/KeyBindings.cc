#include "KeyBindings.h"
#include <iostream>
#include <string>
static constexpr auto keyBindingNames = {
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
void Shiro::KeyBindings::read(PDINI::INI& ini, const std::string sectionName) {
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
        }
        else {
            **keycode = SDL_GetKeyFromName(keyName.c_str());
        }
        ++keycode;
    }
}