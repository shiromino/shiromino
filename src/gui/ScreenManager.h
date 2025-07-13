#pragma once
#include "SDL_events.h"
#include "SDL_rect.h"
#include "SDL_stdinc.h"
#include <stddef.h>
#include <map>
#include <string>
#include <utility>
#include "gui/GUI.h"

struct CoreState;

class ScreenManager {
public:
    ScreenManager()
    {
        currentScreen = NULL;
    }

    ~ScreenManager() {
        for (const auto& screen : screens) {
            delete screen.second;
        }
    }

    void addScreen(std::string name, GUIScreen *screen)
    {
        screens[name] = screen;
    }

    GUIScreen *getCurrentScreen()
    {
        return currentScreen;
    }

    void loadScreen(std::string name)
    {
        auto it = screens.find(name);
        if(it == screens.end())
        {
            /*
            auto itM = screenMakers.find(name);
            if(itM != screenMakers.end())
            {
                GUIScreen *s = (std::get<1>(*itM))(this);
                screens[std::get<0>(*itM)] = s;
                currentScreen = s;
            }
            */
        }
        else
        {
            currentScreen = std::get<1>(*it);
        }
    }

    void drawScreen()
    {
        if(currentScreen)
        {
            currentScreen->draw();
        }
    }

    void handleSDLEvent(SDL_Event& event, GUIPoint logicalMousePos)
    {
        if(currentScreen)
        {
            currentScreen->handleSDLEvent(event, logicalMousePos);
        }
    }

protected:
    std::map<std::string, GUIScreen *> screens;
    //std::map<std::string, std::function<GUIScreen *(ScreenManager *)>> screenMakers;
    GUIScreen *currentScreen;
};

class GUIScreenSwitchButton : public Button {
public:
    GUIScreenSwitchButton(int ID, std::string screenName, ScreenManager *mngr, std::string displayString, BitFont& font, SDL_Rect relativeDestRect)
        : Button(ID, relativeDestRect, displayString, font)
    {
        this->screenName = screenName;
        this->mngr = mngr;
    }

    virtual void mouseClicked(int, int, Uint8) override
    {
        if(mngr)
        {
            mngr->loadScreen(screenName);
        }
    }

protected:
    std::string screenName;
    ScreenManager *mngr;
};

GUIScreen *mainMenu_create(CoreState *cs, BitFont& font);
void mainMenuInteractionCallback(GUIInteractable&, GUIEvent&);