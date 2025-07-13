#include "KeyBindings.h"
#include "PDINI.h"
#include "SDL_keyboard.h"
#include <initializer_list>
#include <iostream>
#include <regex>
#include <string>
#include <utility>

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
 * TODO: Discuss and decide on the default key configuration here while also closing issue #18.
 */
Shiro::KeyBindings::KeyBindings(int playerNumber) {
    switch (playerNumber) {
        default:
        case 0:
            left.scancode = SDL_SCANCODE_LEFT;
            right.scancode = SDL_SCANCODE_RIGHT;
            up.scancode = SDL_SCANCODE_UP;
            down.scancode = SDL_SCANCODE_DOWN;
            start.scancode = SDL_SCANCODE_RETURN;
            a.scancode = SDL_SCANCODE_F;
            b.scancode = SDL_SCANCODE_D;
            c.scancode = SDL_SCANCODE_S;
            d.scancode = SDL_SCANCODE_A;
            escape.scancode = SDL_SCANCODE_ESCAPE;
            break;

        case 1:
            left.scancode = SDL_SCANCODE_J;
            right.scancode = SDL_SCANCODE_L;
            up.scancode = SDL_SCANCODE_I;
            down.scancode = SDL_SCANCODE_K;
            start.scancode = SDL_SCANCODE_TAB;
            a.scancode = SDL_SCANCODE_R;
            b.scancode = SDL_SCANCODE_E;
            c.scancode = SDL_SCANCODE_W;
            d.scancode = SDL_SCANCODE_Q;
            escape.scancode = SDL_SCANCODE_F11;
            break;
    }

    left.isKeycode = false;
    right.isKeycode = false;
    up.isKeycode = false;
    down.isKeycode = false;
    start.isKeycode = false;
    a.isKeycode = false;
    b.isKeycode = false;
    c.isKeycode = false;
    d.isKeycode = false;
    escape.isKeycode = false;
}

void Shiro::KeyBindings::read(PDINI::INI& ini) {
    const auto sectionName = "PLAYER_1_KEY_BINDINGS";
    KeyBinding* const keyBindings[] = {
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
    KeyBinding* const * keyBinding = &keyBindings[0];
    for(const auto &keyBindingName : keyBindingNames)
    {
        std::string keyName;
        if(ini.get(sectionName, keyBindingName, keyName)) {
            SDL_Keycode keycode = SDL_GetKeyFromName(keyName.c_str());
            SDL_Scancode scancode;
            if(keycode == SDLK_UNKNOWN)
            {
                std::smatch match;
                if(
                    std::regex_search(keyName, match, std::regex("^(United States\\s+)", std::regex::icase)) &&
                    (scancode = SDL_GetScancodeFromName(keyName.c_str() + match[1].str().length())) != SDL_SCANCODE_UNKNOWN
                    )
                {
                    (*keyBinding)->scancode = scancode;
                    (*keyBinding)->isKeycode = false;
                }
                else
                {
                    std::cerr << "Binding for " << keyBindingName << " is invalid." << std::endl;
                }
            }
            else
            {
                (*keyBinding)->keycode = keycode;
                (*keyBinding)->isKeycode = true;
            }
        }
        ++keyBinding;
    }
}

void Shiro::KeyBindings::write(PDINI::INI& ini) const {
    const auto sectionName = "PLAYER_1_KEY_BINDINGS";
    const KeyBinding* const keyBindings[] = {
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
    const KeyBinding* const* keyBinding = keyBindings;
    for (const auto& keyBindingName : keyBindingNames) {
        if ((*keyBinding)->isKeycode) {
            ini.set(sectionName, keyBindingName, std::string(SDL_GetKeyName((*keyBinding)->keycode)));
        }
        else {
            ini.set(sectionName, keyBindingName, "United States " + std::string(SDL_GetScancodeName((*keyBinding)->scancode)));
        }
        ++keyBinding;
    }
}
